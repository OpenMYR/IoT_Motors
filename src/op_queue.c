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

int store_command(struct stepper_command_packet *packet, uint8* ip_addr, char id)
{
	if((id < 0) || (id >= MOTOR_COUNT)) return -1;
	if (queue_length[id] < BUFFER_SIZE)
	{	
		command_queue[id][(last_element[id] + 1) % BUFFER_SIZE].packet = *packet;
		os_memcpy(command_queue[id][(last_element[id] + 1) % BUFFER_SIZE].ip_addr, ip_addr, 4);
		last_element[id] = (last_element[id] + 1) % BUFFER_SIZE;
		queue_length[id]++;
		return queue_length[id];
	}
	else return -2;
}

struct stepper_command_data* get_command(char id)
{
	if((id < 0) || (id >= MOTOR_COUNT))
	{
		return NULL;
	}
	if (queue_length[id] > 0) 
	{
		return  &command_queue[id][first_element[id]];
	} else {
		return NULL;
	}
}

int remove_first_command(char id)
{
	if((id < 0) || (id >= MOTOR_COUNT))
	{
		return -1;
	}
	else if(is_queue_empty(id))
	{
		return -2;
	}
	first_element[id] = (first_element[id]  + 1) % BUFFER_SIZE;
	queue_length[id]--;
	return queue_length[id];
}

int clear_queue(char id)
{
	if((id < 0) || (id >= MOTOR_COUNT)) return -1;
	first_element[id] = (last_element[id] + 1) % BUFFER_SIZE;
	queue_length[id] = 0;
	return queue_length[id];
}

int is_queue_empty(char id)
{
	if((id < 0) || (id >= MOTOR_COUNT)) return -1;
	return (queue_length[id] == 0) ? 1: 0;
}

#ifdef TEST_MODE

static void tearDown()
{
	int x = 0; 
	for(x; x < MOTOR_COUNT; x++)
	{
		assert_int_equal(clear_queue(x), 0);
		assert_int_equal(is_queue_empty(x), 1);
	}
}

static void test_queue_empty_on_init(void **state)
{
	(void) state;
	int x = 0;
	for(x; x < MOTOR_COUNT; x++)
	{
		assert_int_equal(is_queue_empty(x), 1);
	}

	tearDown();
}

static void test_queue_not_empty_after_element_added(void **state)
{
	(void) state;

	struct stepper_command_packet test_data;
	test_data.port = 1100;
	test_data.opcode = 'G';
	test_data.queue = 0;
	test_data.step_num = 3000;
	test_data.step_rate = 50;
	uint8 ip_address[4] = {192, 168, 1, 150};

	int x = 0;
	for(x; x < MOTOR_COUNT; x++)
	{
		test_data.motor_id = x;
		store_command(&test_data, ip_address, x);
		assert_int_equal(is_queue_empty(x), 0);
	}

	tearDown();
}

static void test_queue_not_empty_bad_motor_id(void **state)
{
	(void) state;

	assert_int_equal(is_queue_empty(-1), -1);
	assert_int_equal(is_queue_empty(5), -1);
	tearDown();
}

static void test_clear_queue(void **state)
{
	(void) state;

	struct stepper_command_packet test_data;
	test_data.port = 1100;
	test_data.opcode = 'G';
	test_data.queue = 0;
	test_data.step_num = 3000;
	test_data.step_rate = 50;
	uint8 ip_address[4] = {192, 168, 1, 150};

	int x = 0;
	for(x; x < MOTOR_COUNT; x++)
	{
		test_data.motor_id = x;
		int y = 0;
		for(y; y < BUFFER_SIZE; y++)
		{
			assert_int_equal(store_command(&test_data, ip_address, x), y+1);
			assert_int_equal(is_queue_empty(x), 0);
		}
	}

	x = 0;

	for(x; x < MOTOR_COUNT; x++)
	{
		assert_int_equal(clear_queue(x), 0);
		assert_int_equal(is_queue_empty(x), 1);
	}

	tearDown();

}

static void test_clear_queue_bad_motor_id( void **state)
{
	(void) state;

	struct stepper_command_packet test_data;
	test_data.port = 1100;
	test_data.opcode = 'G';
	test_data.queue = 0;
	test_data.step_num = 3000;
	test_data.step_rate = 50;
	uint8 ip_address[4] = {192, 168, 1, 150};

	int x = 0;
	for(x; x < MOTOR_COUNT; x++)
	{
		test_data.motor_id = x;
		store_command(&test_data, ip_address, x);
		assert_int_equal(is_queue_empty(x), 0);
	}

	assert_int_equal(clear_queue(-1), -1);
	assert_int_equal(clear_queue(5), -1);

	tearDown();
} 

static void test_remove_first_command(void **state)
{
	(void) state;

	struct stepper_command_packet test_data;
	test_data.port = 1100;
	test_data.opcode = 'G';
	test_data.queue = 0;
	test_data.step_num = 3000;
	test_data.step_rate = 50;
	uint8 ip_address[4] = {192, 168, 1, 150};

	int x = 0;
	for(x; x < MOTOR_COUNT; x++)
	{
		test_data.motor_id = x;
		int y = 0;
		for(y; y < BUFFER_SIZE; y++)
		{
			assert_int_equal(store_command(&test_data, ip_address, x), y+1);
			assert_int_equal(is_queue_empty(x), 0);
		}
	}

	x = 0;

	for(x; x < MOTOR_COUNT; x++)
	{
		int y = 0;
		for(y; y < BUFFER_SIZE; y++)
		{
			assert_int_equal(remove_first_command(x), BUFFER_SIZE - 1 - y);
		}
		assert_int_equal(is_queue_empty(x), 1);
	}

	tearDown();
}

static void test_remove_first_command_empty_queue(void **state)
{
	(void) state;

	int x = 0;
	for(x; x < MOTOR_COUNT; x++)
	{
		assert_int_equal(is_queue_empty(x), 1);
		assert_int_equal(remove_first_command(x), -2);
	}

	tearDown();
}

static void test_remove_first_command_bad_motor_id(void **state)
{
	(void) state;

	assert_int_equal(remove_first_command(-1), -1);
	assert_int_equal(remove_first_command(5), -1);

	tearDown();
}

static void test_store_and_get_command(void **state)
{
	(void) state;

	struct stepper_command_packet test_data[BUFFER_SIZE];
	uint8 ip_address[BUFFER_SIZE][4];
	int x = 0;
	for(x; x < BUFFER_SIZE; x++)
	{
		test_data[x].port = x;
		test_data[x].opcode = 'G';
		test_data[x].queue = 1;
		test_data[x].step_num = x;
		test_data[x].step_rate = 50;
		ip_address[x][0] = x / 1000;
		ip_address[x][1] = (x % 1000) / 100;
		ip_address[x][2] = (x % 100) / 10;
		ip_address[x][3] = x % 10;
	}

	x = 0;

	for(x; x < MOTOR_COUNT; x++)
	{
		int y = 0;
		for(y; y < BUFFER_SIZE; y++)
		{
			assert_int_equal(store_command(&(test_data[y]), ip_address[y], x), y + 1);
		}

		y = 0;
		for(y; y < BUFFER_SIZE; y++)
		{
			stepper_command_data *stored_data = get_command(x);
			assert_int_equal(stored_data->packet.port, test_data[y].port);
			assert_int_equal(stored_data->packet.opcode, test_data[y].opcode);
			assert_int_equal(stored_data->packet.queue, test_data[y].queue);
			assert_int_equal(stored_data->packet.step_num, test_data[y].step_num);
			assert_int_equal(stored_data->packet.step_rate, test_data[y].step_rate);
			assert_int_equal(stored_data->ip_addr[0], ip_address[y][0]);
			assert_int_equal(stored_data->ip_addr[1], ip_address[y][1]);
			assert_int_equal(stored_data->ip_addr[2], ip_address[y][2]);
			assert_int_equal(stored_data->ip_addr[3], ip_address[y][3]);

			assert_int_equal(remove_first_command(x), BUFFER_SIZE - 1 - y);
		}
	}

	tearDown();
}

static void test_get_command_empty_queue(void **state)
{
	(void) state;

	int x = 0;
	for(x; x < MOTOR_COUNT; x++)
	{
		assert_null(get_command(x));
	}

	tearDown();
}

static void test_get_command_bad_motor_id(void **state)
{
	(void) state;
	assert_null(get_command(-1));
	assert_null(get_command(5));

	tearDown();
}

static void test_store_command_add_to_full_queue(void **state)
{
	(void) state;

	struct stepper_command_packet test_data[BUFFER_SIZE];
	uint8 ip_address[BUFFER_SIZE][4];
	int x = 0;
	for(x; x < BUFFER_SIZE; x++)
	{
		test_data[x].port = x;
		test_data[x].opcode = 'G';
		test_data[x].queue = 1;
		test_data[x].step_num = x;
		test_data[x].step_rate = 50;
		ip_address[x][0] = x / 1000;
		ip_address[x][1] = (x % 1000) / 100;
		ip_address[x][2] = (x % 100) / 10;
		ip_address[x][3] = x % 10;
	}

	x = 0;

	for(x; x < MOTOR_COUNT; x++)
	{
		int y = 0;
		for(y; y < BUFFER_SIZE; y++)
		{
			assert_int_equal(store_command(&(test_data[y]), ip_address[y], x), y + 1);
		}

		assert_int_equal(store_command(&(test_data[0]), ip_address[0], x), -2);
	}

	tearDown();
}

static void test_store_command_bad_motor_id(void **state)
{
	(void) state;

	struct stepper_command_packet test_data;
	test_data.port = 1100;
	test_data.opcode = 'G';
	test_data.queue = 0;
	test_data.step_num = 3000;
	test_data.step_rate = 50;
	uint8 ip_address[4] = {192, 168, 1, 150};

	assert_int_equal(store_command(&test_data, ip_address, -1), -1);
	assert_int_equal(store_command(&test_data, ip_address, 5), -1);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_queue_empty_on_init),
		cmocka_unit_test(test_queue_not_empty_after_element_added),
		cmocka_unit_test(test_queue_not_empty_bad_motor_id),
		cmocka_unit_test(test_clear_queue),
		cmocka_unit_test(test_clear_queue_bad_motor_id),
		cmocka_unit_test(test_remove_first_command),
		cmocka_unit_test(test_remove_first_command_empty_queue),
		cmocka_unit_test(test_remove_first_command_bad_motor_id),
		cmocka_unit_test(test_store_and_get_command),
		cmocka_unit_test(test_get_command_empty_queue),
		cmocka_unit_test(test_get_command_bad_motor_id),
		cmocka_unit_test(test_store_command_add_to_full_queue),
		cmocka_unit_test(test_store_command_bad_motor_id)
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

#endif
