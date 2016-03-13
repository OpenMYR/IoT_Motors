#include "gpio_driver.h"
#include "motor_driver.h"
#include "osapi.h"
#include "udp.h"
#include "user_config.h"

#define PAUSED_HIGH_TICKS 501
#define SERVO_TICKS_FLOOR 150
#define SERVO_TICKS_CEILING 500

#define MINIMUM_DUTY_CYCLE_US 1000
#define MAXIMUM_DUTY_CYCLE_US 2000
#define PULSE_LENGTH_US 20000
#define PULSE_FREQUENCY (1 / (PULSE_LENGTH_US * 0.000001))

#define MAX_HIGH_TICKS MAXIMUM_DUTY_CYCLE_US / RESOLUTION_US
#define MIN_HIGH_TICKS MINIMUM_DUTY_CYCLE_US / RESOLUTION_US
#define PULSE_LENGTH_TICKS (PULSE_LENGTH_US / RESOLUTION_US)
#define SECOND_LENGTH_TICKS 1000000 / RESOLUTION_US
#define SECONDS_PER_US 10

static volatile int ticks = 0;
static volatile int high_ticks = MAX_HIGH_TICKS;
static volatile enum motor_direction motor_state = PAUSED;
static volatile int next_high_ticks = MAX_HIGH_TICKS;
static volatile int goal_high_ticks = MAX_HIGH_TICKS;
static volatile int step_rate = 100;
static volatile float rate_counter = 0.0; 
static volatile float rate_incrementor = 2;
static const float step_threshold = 1;
static volatile long int step_pool;
static volatile char opcode = ' ';
static volatile int command_done = 1;


void step_driver ( void )
{
    ticks++;

    if(ticks == high_ticks)
    {
        eio_low ( GPIO_STEP );
    }
    else if(ticks == PULSE_LENGTH_TICKS)
    {
        ticks = 0;
        eio_high ( GPIO_STEP );
		high_ticks = next_high_ticks;
		if(high_ticks != goal_high_ticks)
		{
			system_os_post(MOTOR_DRIVER_TASK_PRIO, 0, 0);
		}
		else if(!command_done)
		{
			system_os_post(ACK_TASK_PRIO, 0, 0);
			command_done = 1;
		}
    }

}

void opcode_move(signed int step_num, unsigned short step_rate)
{
	int tick_total = high_ticks+step_num;
	if(tick_total > SERVO_TICKS_CEILING)
	{
		goal_high_ticks = SERVO_TICKS_CEILING;
	}
	else if(tick_total < SERVO_TICKS_FLOOR)
	{
		goal_high_ticks = SERVO_TICKS_FLOOR;
	}
	else
	{
		goal_high_ticks = tick_total;
	}
	rate_counter = 0.0;
	rate_incrementor = calculate_step_incrementor(step_rate);
	motor_state = (goal_high_ticks >= high_ticks) ? FORWARDS : BACKWARDS;
	step_pool = motor_state * (goal_high_ticks - high_ticks);
	opcode = 'M';
	if(goal_high_ticks == high_ticks){
		system_os_post(ACK_TASK_PRIO, 0, 0);
	} else {
		command_done = 0;
	}
}

void opcode_goto(signed int step_num, unsigned short step_rate)
{
	goal_high_ticks = ((step_num <= SERVO_TICKS_CEILING) && (step_num >= SERVO_TICKS_FLOOR))
		? step_num : goal_high_ticks;
	rate_counter = 0.0;
	rate_incrementor = calculate_step_incrementor(step_rate);
	motor_state = (goal_high_ticks >= high_ticks) ? FORWARDS : BACKWARDS;
	step_pool = motor_state * (goal_high_ticks - high_ticks);
	opcode = 'G';
	if(goal_high_ticks == high_ticks){
		system_os_post(ACK_TASK_PRIO, 0, 0);
	} else {
		command_done = 0;
	}
}

void opcode_stop(signed int wait_time, unsigned short precision)
{
	motor_state = PAUSED;
	goal_high_ticks = PAUSED_HIGH_TICKS;
	step_pool = precision * wait_time;
	rate_incrementor = 1;
	rate_counter = 0.0;
	opcode = 'S';
	if(wait_time <= 0){
		system_os_post(ACK_TASK_PRIO, 0, 0);
	} else {
		command_done = 0;
	}
}

float calculate_step_incrementor(unsigned short input_step_rate)
{
	return input_step_rate / PULSE_FREQUENCY;
}

void driver_logic_task(os_event_t *events)
{
	rate_counter += rate_incrementor;
	if(rate_counter >= step_threshold)
	{
		int steps_to_take = rate_counter / step_threshold;
		rate_counter -= step_threshold * (float)steps_to_take;
		if(step_pool <= steps_to_take)
		{
			if(motor_state == PAUSED)
			{
				goal_high_ticks = high_ticks;
			}
			next_high_ticks = goal_high_ticks;
		}
		else
		{
			next_high_ticks += steps_to_take * motor_state;
			step_pool -= steps_to_take;
		}
	}
}

int is_motor_running()
{
	return !command_done;
}