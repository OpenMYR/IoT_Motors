#define ICACHE_FLASH

/*
 * This whole file is depreciated due to OpenMYR
 * only supporting quad_servo_driver and
 * stepper_driver currently.
 */

#include "c_types.h"
#include "eagle_soc.h"
#include "motor_driver.h"
#include "osapi.h"
#include "udp.h"
#include "user_config.h"

#define GPIO_MASK_WRITE(mask)	{												\
									GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, (mask));	\
									GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, ~(mask));	\
								}


#define GPIO_STEP 4
#define GPIO_STEP_ENABLE 5
#define GPIO_STEP_DIR 13
#define GPIO_USTEP_A 0
#define GPIO_USTEP_B 14
#define GPIO_USTEP_C 12

#define GPIO_USTEP_A_MASK 0x0001
#define GPIO_STEP_MASK 0x0010
#define GPIO_STEP_ENABLE_MASK 0x0020
#define GPIO_USTEP_C_MASK 0x1000
#define GPIO_STEP_DIR_MASK 0x2000
#define GPIO_USTEP_B_MASK 0x4000
#define GPIO_USTEP_ALL_MASK 0x5001 
#define GPIO_NOSTEP_MASK 0x7021
#define GPIO_NODIR_MASK 0x5031
#define GPIO_ALL_MASK 0x7031
#define GPIO_STOP_MASK 0xDFEF

#define STEP_RATE_MAX 1000
#define PULSE_LENGTH_US 2000
#define PULSE_FREQUENCY (1 / (PULSE_LENGTH_US * 0.000001))

#define PULSE_LENGTH_TICKS (PULSE_LENGTH_US / RESOLUTION_US)
#define SECOND_LENGTH_TICKS 1000000 / RESOLUTION_US

void ICACHE_FLASH_ATTR set_duty_cycle (unsigned short step_rate);

static volatile enum motor_direction motor_state = PAUSED;
static volatile float duty_ratio = 1;
static volatile int tick_counter = 0; 
static volatile int tick_incrementor = 1;
static volatile int gpio_change_needed = 0;
static volatile int high_threshold = 0;
static volatile long int step_pool;
static volatile long int brushed_dc_position;

static volatile float cycle_inaccuracy = 0;
static volatile float inaccuracy_compensation_counter = 0;

static volatile char opcode = ' ';
static volatile int command_done = 1;

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
	GPIO_MASK_WRITE(GPIO_USTEP_ALL_MASK);
}

void step_driver ( void )
{
	if (motor_state != PAUSED) 
	{
		if (tick_counter >= PULSE_LENGTH_TICKS)
		{
			tick_counter -= PULSE_LENGTH_TICKS;
			gpio_change_needed = 1;
		}
		if (tick_counter < high_threshold)
		{
			if (gpio_change_needed)
			{
				gpio_change_needed = 0;
				if(motor_state == FORWARDS)
				{
					GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) | GPIO_STEP_DIR_MASK);
				}
				else
				{
					GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) | GPIO_STEP_MASK);
				}
			}
			step_pool--;
			brushed_dc_position += motor_state;
			if(step_pool <= 1)//0?
			{
				if(motor_state == FORWARDS)
				{
					GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_DIR_MASK));
				}
				else
				{
					GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_MASK));
				}
				motor_state = PAUSED;
				command_done = 1;
				system_os_post(ACK_TASK_PRIO, 0, 0);
			}
		} 
		else if(tick_counter > high_threshold)
		{
			if (gpio_change_needed)
			{
				if(motor_state == FORWARDS)
				{
					GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_DIR_MASK));
				}
				else
				{
					GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_MASK));
				}
			}		
		} 
		//only one segment of the cycle changes between cycles, this is that segment
		else 
		{
			inaccuracy_compensation_counter += cycle_inaccuracy;
			if (inaccuracy_compensation_counter >= tick_incrementor)
			{
				gpio_change_needed = 1;
				step_pool--;
				brushed_dc_position += motor_state;
				inaccuracy_compensation_counter -= tick_incrementor;
				if(motor_state == FORWARDS)
				{
					GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) | GPIO_STEP_DIR_MASK);
				}
				else
				{
					GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) | GPIO_STEP_MASK);
				}
				if(step_pool <= 1)//0?
				{
					if(motor_state == FORWARDS)
					{
						GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_DIR_MASK));
					}
					else
					{
						GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_MASK));
					}
					motor_state = PAUSED;
					command_done = 1;
					system_os_post(ACK_TASK_PRIO, 0, 0);
				}
			}
			else {
				gpio_change_needed = 0;
				if(motor_state == FORWARDS)
				{
					GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_DIR_MASK));
				}
				else
				{
					GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_MASK));
				}
			}

		}
		tick_counter += tick_incrementor;
	}
	else if(!command_done && step_pool > 0)
	{
		step_pool--;
		if(step_pool == 0) system_os_post(ACK_TASK_PRIO, 0, 0);
	}
}

void opcode_move(signed int step_num, unsigned short step_rate, char motor_id)
{
	set_duty_cycle(step_rate);
	
	motor_state = (step_num >= 0) ? FORWARDS : BACKWARDS;
	step_pool = motor_state * step_num;
	opcode = 'M';
	if(step_num == 0){
		system_os_post(ACK_TASK_PRIO, 0, 0);
	} else {
		command_done = 0;
	}
	if(motor_state == FORWARDS)
	{
		GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_MASK));
	}
	else
	{
		GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_DIR_MASK));
	}
}

void opcode_goto(signed int step_num, unsigned short step_rate, char motor_id)
{
	set_duty_cycle(step_rate);

	motor_state = (step_num >= brushed_dc_position) ? FORWARDS : BACKWARDS;
	step_pool = motor_state * (step_num - brushed_dc_position);
	opcode = 'G';
	if(step_pool == 0){
		system_os_post(ACK_TASK_PRIO, 0, 0);
	} else {
		command_done = 0;
	}
	if(motor_state == FORWARDS)
	{
		GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_MASK));
	}
	else
	{
		GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & (~GPIO_STEP_DIR_MASK));
	}
}

void opcode_stop(signed int wait_time, unsigned short precision, char motor_id)
{
	motor_state = PAUSED;
	//goal_high_ticks = PAUSED_HIGH_TICKS;
	step_pool = precision * wait_time;
	opcode = 'S';
	if(wait_time <= 0){
		system_os_post(ACK_TASK_PRIO, 0, 0);
	} else {
		command_done = 0;
	}
	GPIO_MASK_WRITE(GPIO_REG_READ(GPIO_OUT_ADDRESS) & GPIO_STOP_MASK);
}

void ICACHE_FLASH_ATTR set_duty_cycle (unsigned short step_rate)
{
	float bob = step_rate;
	if(bob > STEP_RATE_MAX)
	{
		bob = STEP_RATE_MAX;
	}
	else if(step_rate < 0)
	{
		bob = 0;
	}
	duty_ratio = bob / 1000.0;
	high_threshold = PULSE_LENGTH_TICKS * duty_ratio;
	cycle_inaccuracy = ((float)PULSE_LENGTH_TICKS * duty_ratio) - (float)high_threshold;
	os_printf("Inaccuracy: %d\n", (int)(cycle_inaccuracy*1000.0));
	os_printf("duty ratio: %d\n", (int)(duty_ratio * 1000.0));
	os_printf( "high thresh: %d\n",high_threshold);
	inaccuracy_compensation_counter = 0;
}

float calculate_step_incrementor(unsigned short input_step_rate)
{
}

void driver_logic_task(os_event_t *events)
{
}

void ICACHE_FLASH_ATTR change_motor_setting(config_setting input, int data)
{
}

int is_motor_running(char motor_id)
{
	return !command_done;
}