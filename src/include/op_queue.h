#ifndef OP_QUEUE_H
#define OP_QUEUE_H

#include "udp.h"

typedef struct stepper_command_data
{
	struct stepper_command_packet packet;
	uint8 ip_addr[4];
} stepper_command_data;

int store_command(struct stepper_command_packet *, uint8 *, char);
stepper_command_data* get_command(char);
int remove_first_command(char);
int clear_queue(char);
int is_queue_empty(char);



#endif