#include "include/op_queue.h"
#include "osapi.h"

void op_queue::store_command(struct stepper_command_packet *packet, IPAddress* ip_addr, uint8_t id)
{
	if (queue_length[id] < BUFFER_SIZE)
	{	
		command_queue[id][(last_element[id] + 1) % BUFFER_SIZE].packet = *packet;
		os_memcpy(&(command_queue[id][(last_element[id] + 1) % BUFFER_SIZE].ip_addr), ip_addr, sizeof(IPAddress));
		last_element[id] = (last_element[id] + 1) % BUFFER_SIZE;
		queue_length[id]++;
	}
}

struct stepper_command_data* op_queue::get_command(uint8_t id)
{
	if (queue_length[id] > 0) 
	{
		return  &command_queue[id][first_element[id]];
	} else {
		return NULL;
	}
}

int op_queue::remove_first_command(uint8_t id)
{
	//command_queue[first_element] = *NULL;
	first_element[id] = (first_element[id]  + 1) % BUFFER_SIZE;
	queue_length[id]--;
}

void op_queue::clear_queue(uint8_t id)
{
	//command_queue = (stepper_command_data)os_zalloc(sizeof(command_queue));
	first_element[id] = (last_element[id] + 1) % BUFFER_SIZE;
	queue_length[id] = 0;
}

bool op_queue::is_queue_empty(uint8_t id)
{
	return queue_length[id] == 0;
}