#include "op_queue.h"
#include "osapi.h"

#define BUFFER_SIZE 1000

static struct stepper_command_data command_queue[BUFFER_SIZE];
static unsigned short queue_length = 0;
static unsigned short first_element = 1;
static unsigned short last_element = 0;

void store_command(struct stepper_command_packet *packet, uint8* ip_addr)
{
	if (queue_length < BUFFER_SIZE)
	{	
		command_queue[(last_element + 1) % BUFFER_SIZE].packet = *packet;
		os_memcpy(command_queue[(last_element + 1) % BUFFER_SIZE].ip_addr, ip_addr, 4);
		last_element = (last_element + 1) % BUFFER_SIZE;
		queue_length++;
	}
}

struct stepper_command_data* get_command()
{
	if (queue_length > 0) 
	{
		return  &command_queue[first_element];
	} else {
		return NULL;
	}
}

int remove_first_command()
{
	//command_queue[first_element] = *NULL;
	first_element = (first_element  + 1) % BUFFER_SIZE;
	queue_length--;
}

void clear_queue()
{
	//command_queue = (stepper_command_data)os_zalloc(sizeof(command_queue));
	first_element = (last_element + 1) % BUFFER_SIZE;
	queue_length = 0;
}

int is_queue_empty()
{
	return (queue_length == 0) ? 1: 0;
}