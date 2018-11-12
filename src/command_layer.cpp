#include "include/command_layer.h"
#include "Arduino.h"
#include "include/quad_servo_driver.h"
#include "include/stepper_driver.h"
#include "include/wifi.h"

#include <string>

#define JSON_TOKEN_AMOUNT 83

jsmn_parser command_layer::json_parser;
stepper_command_packet command_layer::current_command[4];
IPAddress command_layer::current_addr[4];
motor_driver* command_layer::motor = nullptr;
Ticker command_layer::motor_drv_timer;
op_queue command_layer::command_queue;
bool command_layer::ota_active = false;

command_layer::command_layer()
{

}

void command_layer::init_motor_driver()
{
	if(MOTOR_TYPE == 0)
	{
    	motor = new quad_servo_driver();
    	motor_drv_timer.attach_ms(20,command_layer::motor_drv_isr);
	}
	else
	{
		motor = new stepper_driver();
	}
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
}

void command_layer::json_process_command(const char *json_input)
{
	if(ota_active)
		return; 
		
    jsmn_init(&json_parser);
	jsmntok_t tokens[JSON_TOKEN_AMOUNT];
	int len = jsmn_parse(&json_parser, json_input, os_strlen(json_input), tokens, JSON_TOKEN_AMOUNT);
	if(len < 0)
	{
		Serial.printf("JSON Parsing error code %d\n", len);
		return;
	}
	int place = 0;
	while(place < len)
	{
		//Serial.printf("JSON token %d, %d\n", place, tokens[place].type);
		switch(tokens[place].type)
		{
			case JSMN_STRING:
				if(strncmp("code", json_input + tokens[place].start, tokens[place].end - tokens[place].start) == 0)
				{
					//Serial.printf("opcode\n");
					char json_opcode = *(json_input + tokens[place+1].start);
					uint8 dummy_ip[4];
					switch(json_opcode)
					{
						case 'C':
						case 'D':
						{
							struct wifi_command_packet parsed_wifi_command;
							parsed_wifi_command.opcode = json_opcode;
							int token_len = tokens[place+4].end - tokens[place+4].start;
							os_strncpy(parsed_wifi_command.ssid, json_input + tokens[place+4].start, token_len);
							parsed_wifi_command.ssid[token_len] = 0;
							token_len = tokens[place+5].end - tokens[place+5].start;
							os_strncpy(parsed_wifi_command.password, json_input + tokens[place+5].start, token_len);
							parsed_wifi_command.password[token_len] = 0;
							wifi_process_command(parsed_wifi_command, dummy_ip);
							place += 6;
							break;
						}
						case 'M':
						case 'S':
						case 'G':
						{
							//Serial.printf("M, S or G\n");
							struct stepper_command_packet parsed_motor_command;
							parsed_motor_command.port = 0;
							parsed_motor_command.opcode = json_opcode;
							parsed_motor_command.motor_id = 0;
							parsed_motor_command.queue = (*(json_input + tokens[place+4].start) == '1') ? 0x01 : 0x00;
							signed int steps = 0;
							int place_tracker = 0;
							int negative = 0;
							if(*(json_input + tokens[place+5].start) == '-')
							{
								negative = 1;
								place_tracker++;
							}
							for(place_tracker; place_tracker < (tokens[place+5].end - tokens[place+5].start); place_tracker++)
							{
								steps *= 10;
								steps += *(json_input + tokens[place+5].start + place_tracker) - 48;
							}
							steps = (negative == 1) ? ((-1) * steps) : steps;
							parsed_motor_command.step_num = steps;
							unsigned short rate = 0;
							place_tracker = 0;
							for(place_tracker; place_tracker < (tokens[place+6].end - tokens[place+6].start); place_tracker++)
							{
								rate *= 10;
								rate += *(json_input + tokens[place+6].start + place_tracker) - 48;
							}
							parsed_motor_command.step_rate = rate;
							//Serial.printf("%c %d %d\n", parsed_motor_command.opcode, parsed_motor_command.step_num, parsed_motor_command.step_rate);
							motor_process_command(parsed_motor_command, dummy_ip);
							place +=7;
							break;
						}
						case 'R':
							//steps per degree
						{
							break;
						}
						case 'U':
						{
							Serial.printf("U\n");
							struct stepper_command_packet parsed_motor_command;
							parsed_motor_command.port = 0;
							parsed_motor_command.opcode = json_opcode;
							parsed_motor_command.motor_id = 0;
							parsed_motor_command.queue = (*(json_input + tokens[place+4].start) == '1') ? 0x01 : 0x00;
							parsed_motor_command.step_num = 0;
							unsigned short ustep_setting = *(json_input + tokens[place+5].start) - 48;
							parsed_motor_command.step_rate = ustep_setting;
							motor_process_command(parsed_motor_command, dummy_ip);
							place += 6;
							break;
						}
						case 'B':
							//servo bounds
						{
							//Serial.printf("B\n");
							//struct stepper_command_packet stop_packet;
							//stop_packet.queue = 0;
							//stop_packet.opcode = 'S';
							//stop_packet.port = 0;
							//stop_packet.step_num = 0;
							//stop_packet.step_rate = 0;
							//stop_packet.motor_id = 0;
							//current_command[0] = stop_packet;
							//current_addr[0][0] = IPAddress();
							//issue_command(0);
							//command_queue.clear_queue(0);
							//int place_tracker = 0;
							//int bound = 0;
							//for(place_tracker; place_tracker < (tokens[place+4].end - tokens[place+4].start); place_tracker++)
							//{
							//	bound *= 10;
							//	bound += *(json_input + tokens[place+4].start + place_tracker) - 48;
							//}
							//motor->change_motor_setting(MIN_SERVO_BOUND, bound);
							//Serial.printf("Min: %d\n", bound);
							//bound = 0;
							//place_tracker = 0;
							//for(place_tracker; place_tracker < (tokens[place+5].end - tokens[place+5].start); place_tracker++)
							//{
							//	bound *= 10;
							//	bound += *(json_input + tokens[place+5].start + place_tracker) - 48;
							//}
							//motor->change_motor_setting(MAX_SERVO_BOUND, bound);
							//Serial.printf("Max: %d\n", bound);
							//place += 6;
							break;
						}
						default:
							Serial.printf("Opcode %c not found!\n", json_opcode);
							return;
							break;
					}
					break;
				}
                place++;
                break;
			case JSMN_OBJECT:
			case JSMN_ARRAY:
				//Serial.printf("JSON Object or Array %d\n", place);
				place++;
				break;
			case JSMN_PRIMITIVE:
			case JSMN_UNDEFINED:
				Serial.printf("Malformed JSON query!\n");
				return;
				break;
		}
	}
}

void command_layer::acknowledge_command(os_event_t *events)
{
	if(!motor->is_motor_running(events->sig))
	{
    	fetch_command(events->sig);
	}
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
    if(current_command[motor_id].opcode == 'S')
    {
        motor->opcode_stop(current_command[motor_id].step_num, current_command[motor_id].step_rate, motor_id);
    }
    else if(current_command[motor_id].opcode == 'M')
    {
        motor->opcode_move(current_command[motor_id].step_num, current_command[motor_id].step_rate, motor_id);
    }
    else if(current_command[motor_id].opcode == 'G')
    {
        motor->opcode_goto(current_command[motor_id].step_num, current_command[motor_id].step_rate, motor_id);
    }
    else if(current_command[motor_id].opcode == 'U')
    {
		if(!current_command[motor_id].queue)
		{
			issue_stop_packet(motor_id);
		}
        motor->change_motor_setting(motor_driver::config_setting::MICROSTEPPING, current_command[motor_id].step_rate , 0);
    }
	else if(current_command[motor_id].opcode == 'H')
	{
		issue_stop_packet(motor_id);
		motor->change_motor_setting(motor_driver::config_setting::MAX_SERVO_BOUND, current_command[motor_id].step_rate, motor_id);
	}
	else if(current_command[motor_id].opcode == 'L')
	{
		issue_stop_packet(motor_id);
		motor->change_motor_setting(motor_driver::config_setting::MIN_SERVO_BOUND, current_command[motor_id].step_rate, motor_id);
	}
}

void command_layer::motor_drv_isr()
{
    motor->driver();
}

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