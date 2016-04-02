#ifndef OP_QUEUE_H
#define OP_QUEUE_H

#include "udp.h"

typedef struct stepper_command_data
{
	struct stepper_command_packet packet;
	uint8 ip_addr[4];
} stepper_command_data;

void store_command(struct stepper_command_packet *, uint8 *);
stepper_command_data* get_command();
int remove_first_command();
void clear_queue();
int is_queue_empty();



#endif