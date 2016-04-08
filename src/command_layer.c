#include "command_layer.h"
#include "hw_timer.h"
#include "jsmn.h"
#include "udp.h"
#include "mem.h"
#include <math.h>
#include "motor_driver.h"
#include "op_queue.h"
#include "osapi.h"
#include "tcp.h"
#include "user_config.h"
#include "user_interface.h"
#include "wifi.h"

#define JSON_TOKEN_AMOUNT 83

os_event_t task_queue[TASK_QUEUE_LENGTH];

struct stepper_command_packet command;
uint8 command_address[4];

//static volatile jsmntok_t tokens[JSON_TOKEN_AMOUNT];
static volatile jsmn_parser json_parser; 

void initialize_command_layer()
{
	register_motor_packet_callback(*motor_process_command);
	register_wifi_packet_callback(*wifi_process_command);
	register_tcp_json_callback(*json_process_command);
	
	system_os_task(acknowledge_command, ACK_TASK_PRIO, task_queue, TASK_QUEUE_LENGTH);
	system_os_task(driver_logic_task, MOTOR_DRIVER_TASK_PRIO, task_queue, TASK_QUEUE_LENGTH);
	
	hw_timer_init(FRC1_SOURCE, 1);
	hw_timer_set_func(step_driver);
	hw_timer_arm(RESOLUTION_US);
	system_init_done_cb(wifi_init);
}

void motor_process_command(struct stepper_command_packet *packet, uint8 *ip_addr)
{
	
	if (packet->queue && ( !is_queue_empty() ||  is_motor_running(0) ) )
	{
		store_command(packet, ip_addr);
	}
	else
	{
		command = *packet;
		command_address[0] = ip_addr[0];
		command_address[1] = ip_addr[1];
		command_address[2] = ip_addr[2];
		command_address[3] = ip_addr[3];
		issue_command();
		clear_queue();
	}
	
}

void wifi_process_command(struct wifi_command_packet *packet, uint8 *ip_addr)
{
	if(packet->opcode == 'C')
	{
		char *ssid = packet->ssid;
		char *pass =  packet->password;
		change_opmode(STATION_CONNECT, ssid, pass);
	}
	else if(packet->opcode == 'D')
	{
		os_printf("Disconnect from this network and resume broadcasting\n");
		change_opmode(BROADCAST, "", "");
	}
	else if(packet->opcode == 'N')
	{
		os_printf("Mesh Node\n");
	}
	else
	{
		os_printf("Opcode not found!\n");
	}
	
}

void json_process_command(char *json_input)
{
	jsmn_init(&json_parser);
	jsmntok_t tokens[JSON_TOKEN_AMOUNT] = {0};
	int len = jsmn_parse(&json_parser, json_input, os_strlen(json_input), tokens, JSON_TOKEN_AMOUNT);
	os_printf("\nJson parsed, length %d\n", len);
	os_printf(json_input);
	if(len < 0)
	{
		os_printf("JSON Parsing error code %d\n", len);
		return;
	}
	int place = 0;
	while(place < len)
	{
		switch(tokens[place].type)
		{
			case JSMN_STRING:
				if(os_memcmp("code", json_input + tokens[place].start, os_strlen("code")) == 0)
				{
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
							wifi_process_command(&parsed_wifi_command, dummy_ip);
							place += 6;
							break;
						}
						case 'M':
						case 'S':
						case 'G':
						{
							struct stepper_command_packet parsed_motor_command;
							parsed_motor_command.port = 0;
							parsed_motor_command.opcode = json_opcode;
							parsed_motor_command.queue = (*(json_input + tokens[place+4].start) == '1') ? 1 : 0;
							signed int steps = 0;
							int place_tracker = 0;
							for(place_tracker; place_tracker < (tokens[place+5].end - tokens[place+5].start); place_tracker++)
							{
								steps *= 10;
								steps += *(json_input + tokens[place+5].start + place_tracker) - 48;
							}
							parsed_motor_command.step_num = steps;
							unsigned short rate = 0;
							place_tracker = 0;
							for(place_tracker; place_tracker < (tokens[place+6].end - tokens[place+6].start); place_tracker++)
							{
								rate *= 10;
								rate += *(json_input + tokens[place+6].start + place_tracker) - 48;
							}
							parsed_motor_command.step_rate = rate;
							os_printf("Parsed command: Opcode: %c,\n Queue: %d,\n Steps: %d,\n Rate: %d\n", json_opcode, parsed_motor_command.queue, steps, rate);
							motor_process_command(&parsed_motor_command, dummy_ip);
							place +=7;
							break;
						}
						default:
							os_printf("Opcode %c not found!\n", json_opcode);
							return;
							break;
					}
					break;
				}
			case JSMN_OBJECT:
			case JSMN_ARRAY:
				place++;
				break;
			case JSMN_PRIMITIVE:
			case JSMN_UNDEFINED:
				os_printf("Malformed JSON query!\n");
				return;
				break;
		}
	}
}

void issue_command()
{
	
	if(command.opcode == 'S')
    {
        //os_printf("Stop Command, %d Delay Counts at %u counts per second\n",
            //ntohl(command->step_num), ntohs(command->step_rate));
	    opcode_stop(ntohl(command.step_num), ntohs(command.step_rate), 0);
    }
    else if(command.opcode == 'M')
    {
        //os_printf("Move Command, %d Relative Steps at %u steps per second\n",
            //ntohl(command->step_num), ntohs(command->step_rate));
	    opcode_move(ntohl(command.step_num), ntohs(command.step_rate), 0);
    }
    else if(command.opcode == 'G')
    {
        //os_printf("Goto Command, %d Absolute Steps at %u steps per second\n",
            //ntohl(command->step_num), ntohs(command->step_rate));
	    opcode_goto(ntohl(command.step_num), ntohs(command.step_rate), 0);
    }
    else
    {
        os_printf("Opcode not found!\n");
        return;
    }
	
}

void fetch_command()
{
	if(!is_queue_empty())
	{
		command = get_command()->packet;
		os_memcpy(command_address, get_command()->ip_addr, 4);
		remove_first_command();
		issue_command();
	}
}

void acknowledge_command(os_event_t *events)
{
	if(command.port != 0) udp_send_ack(command.opcode, 0, command_address, ntohs(command.port));
	fetch_command();
}