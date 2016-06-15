#define ICACHE_FLASH

#include "c_types.h"
#include "motor_driver.h"
#include "gpio_driver.h"
#include "user_config.h"
#include "osapi.h"
#include <stdlib.h>

#define PULSE_FREQUENCY (1 / (RESOLUTION_US * 0.000001))

#define GPIO_STEP 4
#define GPIO_STEP_ENABLE 5
#define GPIO_STEP_DIR 13
#define GPIO_USTEP_A 0
#define GPIO_USTEP_B 14
#define GPIO_USTEP_C 12

static volatile unsigned long step_pool = 0;
static volatile signed long stepper_position = 0;

static volatile enum stepper_state motor_state = IDLE;
static volatile enum motor_direction motor_dir = FORWARDS;

static volatile char opcode = ' ';

static volatile float rate_counter = 0.0; 
static volatile float rate_incrementor = 0.2;
static const float step_threshold = 1;

void init_motor_gpio()
{
	eio_setup ( GPIO_STEP );
	eio_setup ( GPIO_USTEP_A );
	eio_setup ( GPIO_USTEP_B );
	eio_setup ( GPIO_USTEP_C );
	eio_setup ( GPIO_STEP_DIR);
	eio_setup(GPIO_STEP_ENABLE);

	eio_low ( GPIO_STEP );
	eio_high( GPIO_USTEP_A );
	eio_high( GPIO_USTEP_B );
	eio_high( GPIO_USTEP_C );
	eio_low ( GPIO_STEP_DIR);
	eio_low(GPIO_STEP_ENABLE);
}

void step_driver ( void )
{
	switch(motor_state)
	{
		case STEPPING:
			if(step_pool <= 0)
			{
				motor_state = IDLE;
				eio_low(GPIO_STEP);
				system_os_post(ACK_TASK_PRIO, 0, 0);
			}
			else if(motor_dir == PAUSED)
			{
				step_pool--;
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
						eio_high(GPIO_STEP);
						step_pool--;
						stepper_position += motor_dir;
					}
			}
			return;
		case DIRECTION_ASSERT:
			motor_dir == FORWARDS ? eio_high(GPIO_STEP_DIR) : eio_low(GPIO_STEP_DIR);
			motor_state = STEPPING;
		case IDLE:
		default:
			return;
	}
}

void opcode_move(signed int step_num, unsigned short step_rate, char motor_id)
{
	motor_dir = (step_num >= 0) ? FORWARDS : BACKWARDS;
	eio_low(GPIO_STEP);
	step_pool = motor_dir * step_num;
	rate_counter = 0.0;
	rate_incrementor = calculate_step_incrementor(step_rate);
	opcode = 'M';
	motor_state = DIRECTION_ASSERT;
}

void opcode_goto(signed int step_num, unsigned short step_rate, char motor_id)
{
	motor_dir = (stepper_position <= step_num) ? FORWARDS : BACKWARDS;
	eio_low(GPIO_STEP);
	step_pool = motor_dir * (step_num - stepper_position);
	rate_counter = 0.0;
	rate_incrementor = calculate_step_incrementor(step_rate);
	opcode = 'G';
	motor_state = DIRECTION_ASSERT;
}

void opcode_stop(signed int wait_time, unsigned short precision, char motor_id)
{
	motor_dir = PAUSED;
	eio_low(GPIO_STEP_DIR);
	rate_counter = 0.0;
	rate_incrementor = 1;
	step_pool = abs(wait_time) / calculate_step_incrementor(precision);
	opcode = 'S';
	motor_state = STEPPING;
}

float calculate_step_incrementor(unsigned short input_step_rate)
{
	return input_step_rate / PULSE_FREQUENCY;
}

void driver_logic_task(os_event_t *events)
{

}

void ICACHE_FLASH_ATTR change_motor_setting(config_setting input, int data)
{
	switch(input)
	{
		case MIN_SERVO_BOUND:
		case MAX_SERVO_BOUND:
			break;
		case MICROSTEPPING:
			break;
	}
}

int is_motor_running(char motor_id)
{
	return (motor_state == IDLE) ? 0 : 1;
}