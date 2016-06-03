#define ICACHE_FLASH

#include "c_types.h"
#include "motor_driver.h"
#include "osapi.h"


#define SERVO_TICKS_FLOOR 150
#define SERVO_TICKS_CEILING 500

static volatile int minimum_ticks = SERVO_TICKS_FLOOR;
static volatile int maximum_ticks = SERVO_TICKS_CEILING;

void step_driver ( void )
{
}

void opcode_move(signed int step_num, unsigned short step_rate, char motor_id)
{
}

void opcode_goto(signed int step_num, unsigned short step_rate,  char motor_id)
{
}

void opcode_stop(signed int wait_time, unsigned short precision,  char motor_id)
{
}

float calculate_step_incrementor(unsigned short input_step_rate)
{
	return 0.0;
}

int is_motor_running( char motor_id)
{
	return 0;
}

void ICACHE_FLASH_ATTR change_motor_setting(config_setting input, int data)
{
	switch(input)
	{
		case MIN_SERVO_BOUND:
			minimum_ticks = data;
			break;
		case MAX_SERVO_BOUND:
			maximum_ticks = data;
			break;
		case MICROSTEPPING:
			break;
	}
}

void driver_logic_task(os_event_t *events)
{
}