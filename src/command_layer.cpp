#include "include/command_layer.h"
#include "Arduino.h"
#if MOTOR_TYPE == 0
#include "include/quad_servo_driver.h"
#else
#include "include/stepper_driver.h"
#endif
#include "include/wifi.h"

#include <string>

#define JSON_TOKEN_AMOUNT 83

stepper_command_packet command_layer::current_command[4];
IPAddress command_layer::current_addr[4];
motor_driver* command_layer::motor = nullptr;
Ticker command_layer::motor_drv_timer;
op_queue command_layer::command_queue;
bool command_layer::ota_active = false;
StaticJsonBuffer<JSON_OBJECT_SIZE(100)> command_layer::jsonBuf;
std::function<void(command_response_packet&)> command_layer::ack_func;

command_layer::command_layer()
{

}

void command_layer::init_motor_driver()
{
	#if MOTOR_TYPE == 0
    	motor = new quad_servo_driver();
    	motor_drv_timer.attach_ms(20,command_layer::motor_drv_isr);
	#else
		motor = new stepper_driver();
	#endif
}

void command_layer::motor_driver_task_passthrough(os_event_t *events)
{
    motor->driver_logic_task(events);
}

void command_layer::motor_process_command(struct stepper_command_packet packet, IPAddress addr)
{
	if(ota_active)
		return;

    if(packet.motor_id < (MOTOR_TYPE == 0 ? 4 : 1))
    {
        if(packet.queue && (!command_queue.is_queue_empty(packet.motor_id) || motor->is_motor_running(packet.motor_id)))
        {
            command_queue.store_command(&packet, &addr, packet.motor_id);
        }
        else
        {
            current_command[packet.motor_id] = packet;
            current_addr[packet.motor_id] = addr;
            issue_command(packet.motor_id);
            command_queue.clear_queue(packet.motor_id);
        }
    }
}

void command_layer::wifi_process_command(struct wifi_command_packet packet, IPAddress addr)
{
	if(ota_active)
		return;

	if(packet.opcode == 'C')
	{
		change_opmode(true, packet.ssid, packet.password);
	}
	else if(packet.opcode == 'D')
	{
		change_opmode(false, "", "");
	}
	if(packet.opcode == 'O')
	{
		change_ota_pass(packet.ssid, packet.password);
	}
}

void command_layer::json_process_command(const char *json_input)
{
	if(ota_active)
		return; 
		
	Serial.println(json_input);

	JsonObject& commandObj = jsonBuf.parse(json_input);
	if(commandObj.success())
	{
		Serial.println("Object parsed!");
		
		JsonArray& commandArray = commandObj["commands"];

		if(commandArray != JsonArray::invalid())
		{
			Serial.println("Command array found!");
			Serial.printf("There are %d commands!\n", commandArray.size());
			for(int i = 0; i < commandArray.size(); i++)
			{
				JsonObject& cmd = commandArray[i];
				if(cmd.success() && cmd.containsKey("code") && cmd.containsKey("data"))
				{
					const char* code = cmd["code"];

					//Serial.printf("%s %d %d %d\n", code, queue, dataOne, dataTwo);
					switch(*code)
					{
						case 'C':
						case 'D':
						case 'O':
						{
							JsonArray& dataArray = cmd["data"];
							if(dataArray.size() != 2)
							{
								Serial.println("Malformed data array!");
								continue;
							}

							const char* ssid = dataArray[0];
							const char* pass = dataArray[1];

							wifi_command_packet parsed_wifi_command;
							parsed_wifi_command.opcode = *code;
							strlcpy(parsed_wifi_command.ssid, ssid, 32);
							strlcpy(parsed_wifi_command.password, pass, 63);

							uint8 dummy_ip[4];
							wifi_process_command(parsed_wifi_command, dummy_ip);
							}break;
						case 'M':
						case 'm':
						case 'S':
						case 'G':
						{
							JsonArray& dataArray = cmd["data"];
							if(dataArray.size() != 4)
							{
								Serial.println("Malformed data array!");
								continue;
							}
		
							uint32_t motor_id = dataArray[0];
							uint32_t queue = dataArray[1];
							uint32_t dataOne = dataArray[2];
							uint32_t dataTwo = dataArray[3];

							stepper_command_packet parsed_motor_command;
							parsed_motor_command.port = 0;
							parsed_motor_command.motor_id = motor_id;
							parsed_motor_command.opcode = *code;
							parsed_motor_command.queue = queue;
							parsed_motor_command.step_num = dataOne;
							parsed_motor_command.step_rate = dataTwo;
							uint8 dummy_ip[4];
							motor_process_command(parsed_motor_command, dummy_ip);
						}break;
						case 'U':
						case 'H':
						case 'L':
						{
							JsonArray& dataArray = cmd["data"];
							if(dataArray.size() != 4)
							{
								Serial.println("Malformed data array!");
								continue;
							}
		
							uint32_t motor_id = dataArray[0];
							uint32_t queue = dataArray[1];
							uint32_t dataOne = dataArray[2];

							stepper_command_packet parsed_motor_command;
							parsed_motor_command.port = 0;
							parsed_motor_command.motor_id = motor_id;
							parsed_motor_command.opcode = *code;
							parsed_motor_command.queue = queue;
							parsed_motor_command.step_num = 0;
							parsed_motor_command.step_rate = dataOne;
							uint8 dummy_ip[4];
							motor_process_command(parsed_motor_command, dummy_ip);
						}break;
						default: 
							Serial.println("Unknown packet");
							break;
					}
				}
			}
		}
		else
		{
			Serial.println("Command array not found!");
		}
	}
	else
	{
		Serial.println("Object not parsed!");
	}
	jsonBuf.clear();
}

void command_layer::acknowledge_command(os_event_t *events)
{
	command_response_packet ack;
	ack.opcode = current_command[events->sig].opcode;
	ack.motor_id = events->sig;
	ack.position = events->par;
	if(!motor->is_motor_running(events->sig))
	{
    	fetch_command(events->sig);
	}
	if(ack_func)
		ack_func(ack);
}

void command_layer::endstop_ack(os_event_t *events)
{
	command_queue.clear_queue(events->sig);
	command_response_packet ack;
	ack.opcode = 'E';
	ack.motor_id = events->sig;
	ack.position = events->par;
	if(ack_func)
		ack_func(ack);
}

void command_layer::fetch_command(uint8_t motor_id)
{
    if(!command_queue.is_queue_empty(motor_id))
    {
        stepper_command_data* cmd = command_queue.get_command(motor_id);
        current_command[motor_id] = cmd->packet;
        current_addr[motor_id] = cmd->ip_addr;
        command_queue.remove_first_command(motor_id);
        issue_command(motor_id);
    }
}

void command_layer::issue_command(uint8_t motor_id)
{
	stepper_command_packet backup;
	switch(current_command[motor_id].opcode){
		case 'S':
        motor->opcode_stop(current_command[motor_id].step_num, current_command[motor_id].step_rate, motor_id);
		break;
		case 'M':
        motor->opcode_move(current_command[motor_id].step_num, current_command[motor_id].step_rate, motor_id);
		break;
		case 'm':
        motor->opcode_move_cont(current_command[motor_id].step_num, current_command[motor_id].step_rate, motor_id);	
		break;
		case 'G':
        motor->opcode_goto(current_command[motor_id].step_num, current_command[motor_id].step_rate, motor_id);
		break;
		case 'U':
		backup = current_command[motor_id];
		if(!current_command[motor_id].queue)
		{
			issue_stop_packet(motor_id);
		}
		current_command[motor_id] = backup;
        motor->change_motor_setting(motor_driver::config_setting::MICROSTEPPING, current_command[motor_id].step_rate , 0);
    
		break;
		case 'H':
		backup = current_command[motor_id];
		issue_stop_packet(motor_id);
		current_command[motor_id] = backup;
		motor->change_motor_setting(motor_driver::config_setting::MAX_SERVO_BOUND, current_command[motor_id].step_rate, motor_id);
	
		break;
		case 'L':
		backup = current_command[motor_id];
		issue_stop_packet(motor_id);
		current_command[motor_id] = backup;
		motor->change_motor_setting(motor_driver::config_setting::MIN_SERVO_BOUND, current_command[motor_id].step_rate, motor_id);
	
		break;
		default:
		// N/A
		break;
	}
}

void command_layer::motor_drv_isr()
{
    motor->driver();
}

// Clear queue and stop motor_id
void command_layer::issue_stop_packet(uint8_t motor_id)
{
	command_queue.clear_queue(motor_id);
	struct stepper_command_packet stop_packet;
	stop_packet.queue = 0;
	stop_packet.opcode = 'S';
	stop_packet.port = 0;
	stop_packet.step_num = 0;
	stop_packet.step_rate = 0;
	stop_packet.motor_id = motor_id;
	current_command[motor_id] = stop_packet;
	current_addr[motor_id] = IPAddress();
	issue_command(motor_id);
}

void command_layer::stop_motor()
{
	if(MOTOR_TYPE == 0)
	{
		for(uint8_t i = 0; i < 4; i++)
			issue_stop_packet(i);
	}
	else
	{
		issue_stop_packet(0);
	}
	
	ota_active = true;
}

void command_layer::register_udp_ack_func(std::function<void(command_response_packet&)>f)
{
	ack_func = f;
}