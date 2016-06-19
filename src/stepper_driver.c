#define ICACHE_FLASH

#include "c_types.h"
#include "eagle_soc.h"
#include "motor_driver.h"
#include "user_config.h"
#include "osapi.h"
#include <stdlib.h>

#define GPIO_MASK_WRITE(mask)	{												\
									GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, (mask));	\
									GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, ~(mask));	\
								}

#define PULSE_FREQUENCY (1 / (RESOLUTION_US * 0.000001))

#define GPIO_STEP 4
#define GPIO_STEP_ENABLE 5
#define GPIO_STEP_DIR 13
#define GPIO_USTEP_A 0
#define GPIO_IO_A 14
#define GPIO_IO_B 12

#define GPIO_USTEP_A_MASK 0x0001
#define GPIO_STEP_MASK 0x0010
#define GPIO_STEP_ENABLE_MASK 0x0020
#define GPIO_IO_B_MASK 0x1000
#define GPIO_STEP_DIR_MASK 0x2000
#define GPIO_IO_A_MASK 0x4000
#define GPIO_NOSTEP_MASK 0x7021
#define GPIO_NODIR_MASK 0x5031
#define GPIO_ALL_MASK 0x7031

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
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);

	GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, GPIO_ALL_MASK);
	GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, ~GPIO_ALL_MASK);
	GPIO_MASK_WRITE(GPIO_USTEP_A_MASK);
}

void step_driver ( void )
{
	switch(motor_state)
	{
		case STEPPING:
			if(step_pool <= 0)
			{
				motor_state = IDLE;
				GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_MASK));
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
				if(GPIO_REG_READ(GPIO_OUT_ADDRESS) & GPIO_STEP_MASK)
				{
					GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_MASK));
				}
				else if(rate_counter >= step_threshold)
				{
					rate_counter -= step_threshold;
					GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) | GPIO_STEP_MASK);
					step_pool--;
					stepper_position += motor_dir;
				}
			}
			return;
		case DIRECTION_ASSERT:
			if(motor_dir == FORWARDS)
			{
				GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) | GPIO_STEP_DIR_MASK);
			}
			else
			{
				GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_DIR_MASK));
			}
			motor_state = STEPPING;
		case IDLE:
		default:
			return;
	}
}

void opcode_move(signed int step_num, unsigned short step_rate, char motor_id)
{
	motor_dir = (step_num >= 0) ? FORWARDS : BACKWARDS;
	GPIO_MASK_WRITE((unsigned int)GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_MASK));
	step_pool = motor_dir * step_num;
	rate_counter = 0.0;
	rate_incrementor = calculate_step_incrementor(step_rate);
	opcode = 'M';
	motor_state = DIRECTION_ASSERT;
}

void opcode_goto(signed int step_num, unsigned short step_rate, char motor_id)
{
	motor_dir = (stepper_position <= step_num) ? FORWARDS : BACKWARDS;
	GPIO_MASK_WRITE((unsigned int)GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_MASK));
	step_pool = motor_dir * (step_num - stepper_position);
	rate_counter = 0.0;
	rate_incrementor = calculate_step_incrementor(step_rate);
	opcode = 'G';
	motor_state = DIRECTION_ASSERT;
}

void opcode_stop(signed int wait_time, unsigned short precision, char motor_id)
{
	motor_dir = PAUSED;
	GPIO_MASK_WRITE((unsigned int)GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_DIR_MASK));
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
			if(data > 0)
			{
				GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) | GPIO_USTEP_A_MASK);
			}
			else
			{
				GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_USTEP_A_MASK));
			}
			break;
	}
}

int is_motor_running(char motor_id)
{
	return (motor_state == IDLE) ? 0 : 1;
}