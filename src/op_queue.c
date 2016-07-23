#include "op_queue.h"
#include "osapi.h"
#ifdef TEST_MODE
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmocka.h"
#endif


#define BUFFER_SIZE (1000 / MOTOR_COUNT)

static struct stepper_command_data command_queue[MOTOR_COUNT][BUFFER_SIZE];
static unsigned short queue_length[MOTOR_COUNT] = {[0 ... MOTOR_COUNT - 1] = 0};
static unsigned short first_element[MOTOR_COUNT] = {[0 ... MOTOR_COUNT - 1] = 1};
static unsigned short last_element[MOTOR_COUNT] = {[0 ... MOTOR_COUNT - 1] = 0};

void store_command(struct stepper_command_packet *packet, uint8* ip_addr, char id)
{
	if (queue_length[id] < BUFFER_SIZE)
	{	
		command_queue[id][(last_element[id] + 1) % BUFFER_SIZE].packet = *packet;
		os_memcpy(command_queue[id][(last_element[id] + 1) % BUFFER_SIZE].ip_addr, ip_addr, 4);
		last_element[id] = (last_element[id] + 1) % BUFFER_SIZE;
		queue_length[id]++;
	}
}

struct stepper_command_data* get_command(char id)
{
	if (queue_length[id] > 0) 
	{
		return  &command_queue[id][first_element[id]];
	} else {
		return NULL;
	}
}

int remove_first_command(char id)
{
	//command_queue[first_element] = *NULL;
	first_element[id] = (first_element[id]  + 1) % BUFFER_SIZE;
	queue_length[id]--;
}

void clear_queue(char id)
{
	//command_queue = (stepper_command_data)os_zalloc(sizeof(command_queue));
	first_element[id] = (last_element[id] + 1) % BUFFER_SIZE;
	queue_length[id] = 0;
}

int is_queue_empty(char id)
{
	return (queue_length[id] == 0) ? 1: 0;
}

#ifdef TEST_MODE

static void test_queue_empty_on_init(void **state)
{
	(void) state;
	int x = 0;
	for(x; x < MOTOR_COUNT; x++)
	{
		assert_int_equal(is_queue_empty(x), 1);
	}
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_queue_empty_on_init)
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#endif
