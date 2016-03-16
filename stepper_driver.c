#include "motor_driver.h"
#include "gpio_driver.h"
#include "user_config.h"
#include "osapi.h"

#define PULSE_FREQUENCY (1 / (RESOLUTION_US * 0.000001))

static volatile unsigned long step_pool = 0;
static volatile signed long stepper_position = 0;

static volatile enum stepper_state motor_state = IDLE;
static volatile enum motor_direction motor_dir = FORWARDS;

static volatile char opcode = ' ';

static volatile int step_rate = 100;
static volatile float rate_counter = 0.0; 
static volatile float rate_incrementor = 0.2;
static const float step_threshold = 1;

void step_driver ( void )
{
	if(motor_state == IDLE) return;
	if(motor_state == DIRECTION_ASSERT)
	{
		//assert the direction for one cycle
		motor_dir == FORWARDS ? eio_high(GPIO_STEP_DIR) : eio_low(GPIO_STEP_DIR);
		motor_state = STEPPING;
	}
	else if(step_pool <= 0)
	{
		motor_state = IDLE;
		eio_low(GPIO_STEP);
		system_os_post(ACK_TASK_PRIO, 0, 0);
	}
	else
	{		
		rate_counter += rate_incrementor;
		//stepping logic
		if(eio_read(GPIO_STEP))
		{
			eio_low(GPIO_STEP);
		}
		else if(rate_counter >= step_threshold)
		{
			rate_counter -= step_threshold;
			if(motor_dir != PAUSED) eio_high(GPIO_STEP) ;
			step_pool--;
			stepper_position += motor_dir;
		}
	}
}

void opcode_move(signed int step_num, unsigned short step_rate)
{
	motor_dir = (step_num >= 0) ? FORWARDS : BACKWARDS;
	eio_low(GPIO_STEP);
	step_pool = motor_dir * step_num;
	rate_counter = 0.0;
	rate_incrementor = calculate_step_incrementor(step_rate);
	motor_state = DIRECTION_ASSERT;
	opcode = 'M';
}

void opcode_goto(signed int step_num, unsigned short step_rate)
{
	motor_dir = (stepper_position <= step_num) ? FORWARDS : BACKWARDS;
	motor_state = DIRECTION_ASSERT;
	eio_low(GPIO_STEP);
	step_pool = motor_dir * (step_num - stepper_position);
	rate_counter = 0.0;
	rate_incrementor = calculate_step_incrementor(step_rate);
	opcode = 'G';
}

void opcode_stop(signed int wait_time, unsigned short precision)
{
	motor_state = STEPPING;
	motor_dir = PAUSED;
	eio_low(GPIO_STEP_DIR);
	rate_counter = 0.0;
	rate_incrementor = 1;
	step_pool = wait_time * precision;
	opcode = 'S';
}

float calculate_step_incrementor(unsigned short input_step_rate)
{
	return input_step_rate / PULSE_FREQUENCY;
}

void driver_logic_task(os_event_t *events)
{

}

int is_motor_running()
{
	return (motor_state == IDLE) ? 0 : 1;
}