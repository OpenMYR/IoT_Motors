#include "command_layer.h"
#include "hw_timer.h"
#include "udp.h"
#include "motor_driver.h"
#include "op_queue.h"
#include "osapi.h"
#include "user_config.h"
#include "user_interface.h"
#include "wifi.h"

os_event_t task_queue[TASK_QUEUE_LENGTH];

struct stepper_command_packet command;
uint8 command_address[4];

void initialize_command_layer()
{
	wifi_init();
	register_motor_packet_callback(*motor_process_command);
	register_wifi_packet_callback(*wifi_process_command);
	
	system_os_task(acknowledge_command, ACK_TASK_PRIO, task_queue, TASK_QUEUE_LENGTH);
	system_os_task(driver_logic_task, MOTOR_DRIVER_TASK_PRIO, task_queue, TASK_QUEUE_LENGTH);
	
	hw_timer_init(FRC1_SOURCE, 1);
	hw_timer_set_func(step_driver);
	hw_timer_arm(RESOLUTION_US);
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
		os_printf("Connect to a network\n");
	}
	else if(packet->opcode == 'D')
	{
		os_printf("Disconnect from this network and resume broadcasting\n");
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
	udp_send_ack(command.opcode, 0, command_address, ntohs(command.port));
	fetch_command();
}