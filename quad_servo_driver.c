#include "motor_driver.h"
#include "osapi.h"

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

void driver_logic_task(os_event_t *events)
{
}