#define ICACHE_FLASH

#include "c_types.h"
#include "eagle_soc.h"
#include "motor_driver.h"
#include "osapi.h"
#include "udp.h"
#include "user_config.h"

#define GPIO_MASK_WRITE(mask)	{												\
									GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, mask);	\
									GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, ~mask);	\
								}

#define GPIO_STEP_A 4
#define GPIO_STEP_B 14
#define GPIO_STEP_C 12
#define GPIO_STEP_D 13

#define GPIO_STEP_A_MASK 0x0010
#define GPIO_STEP_B_MASK 0x4000
#define GPIO_STEP_C_MASK 0x1000
#define GPIO_STEP_D_MASK 0x2000
#define GPIO_ALL_MASK 0x7010

#define PAUSED_HIGH_TICKS 501
#define SERVO_TICKS_FLOOR 150
#define SERVO_TICKS_CEILING 500
#define SERVO_TICKS_DEFAULT 340

#define MINIMUM_DUTY_CYCLE_US 1000
#define MAXIMUM_DUTY_CYCLE_US 2000
#define PULSE_LENGTH_US 20000
#define PULSE_FREQUENCY (1 / (PULSE_LENGTH_US * 0.000001))

#define MAX_HIGH_TICKS MAXIMUM_DUTY_CYCLE_US / RESOLUTION_US
#define MIN_HIGH_TICKS MINIMUM_DUTY_CYCLE_US / RESOLUTION_US
#define PULSE_LENGTH_TICKS (PULSE_LENGTH_US / RESOLUTION_US)
#define SECOND_LENGTH_TICKS 1000000 / RESOLUTION_US

static volatile int ticks = 0;
static volatile int high_ticks[4] = {[0 ... 3] = SERVO_TICKS_DEFAULT};
static volatile enum motor_direction motor_state[4] = {[0 ... 3] = PAUSED};
static volatile int next_high_ticks[4] = {[0 ... 3] = SERVO_TICKS_DEFAULT};
static volatile int goal_high_ticks[4] = {[0 ... 3] = SERVO_TICKS_DEFAULT};
static volatile float rate_counter[4] = {[0 ... 3] = 0.0}; 
static volatile float rate_incrementor[4] = {[0 ... 3] = 2};
static const float step_threshold = 1;
static volatile long int step_pool[4];
static volatile char opcode[4] = {[0 ... 3] = ' '};
static volatile int command_done[4] = {[0 ... 3] = 1};

static volatile int minimum_ticks = SERVO_TICKS_FLOOR;
static volatile int maximum_ticks = SERVO_TICKS_CEILING;
const unsigned int gpio_output_mask[4] = {GPIO_STEP_A_MASK, GPIO_STEP_B_MASK, GPIO_STEP_C_MASK, GPIO_STEP_D_MASK};

void init_motor_gpio()
{
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
	GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, GPIO_ALL_MASK);
	GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, ~GPIO_ALL_MASK);
}

void step_driver ( void )
{
	ticks++;
	if(ticks <= (maximum_ticks + 1))
	{
		unsigned int gpio_output = ((ticks > high_ticks[0]) * gpio_output_mask[0]) +
								((ticks > high_ticks[1]) * gpio_output_mask[1]) +
								((ticks > high_ticks[2]) * gpio_output_mask[2]) +
								((ticks > high_ticks[3]) * gpio_output_mask[3]);
		GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, gpio_output);
	}
	else if(ticks == PULSE_LENGTH_TICKS)
	{
		ticks = 0;
		GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, GPIO_ALL_MASK);
		os_memcpy(high_ticks, next_high_ticks, 16);

		if(!command_done[0])
		{
			if(high_ticks[0] != goal_high_ticks[0])
			{
				system_os_post(MOTOR_DRIVER_TASK_PRIO, 0, 0);
			}
			else
			{
				system_os_post(ACK_TASK_PRIO, 0, 0);
				command_done[0] = 1;
			}
		}

		if(!command_done[1])
		{
			if(high_ticks[1] != goal_high_ticks[1])
			{
				system_os_post(MOTOR_DRIVER_TASK_PRIO, 1, 0);
			}
			else
			{
				system_os_post(ACK_TASK_PRIO, 1, 0);
				command_done[1] = 1;
			}
		}

		if(!command_done[2])
		{
			if(high_ticks[2] != goal_high_ticks[2])
			{
				system_os_post(MOTOR_DRIVER_TASK_PRIO, 2, 0);
			}
			else
			{
				system_os_post(ACK_TASK_PRIO, 2, 0);
				command_done[2] = 1;
			}
		}

		if(!command_done[3])
		{
			if(high_ticks[3] != goal_high_ticks[3])
			{
				system_os_post(MOTOR_DRIVER_TASK_PRIO, 3, 0);
			}
			else
			{
				system_os_post(ACK_TASK_PRIO, 3, 0);
				command_done[3] = 1;
			}
		}
	}
}

void opcode_move(signed int step_num, unsigned short step_rate, char motor_id)
{
	int tick_total = high_ticks[motor_id]+step_num;
	if(tick_total > maximum_ticks)
	{
		goal_high_ticks[motor_id] = maximum_ticks;
	}
	else if(tick_total < minimum_ticks)
	{
		goal_high_ticks[motor_id] = minimum_ticks;
	}
	else
	{
		goal_high_ticks[motor_id] = tick_total;
	}
	rate_counter[motor_id] = 0.0;
	rate_incrementor[motor_id] = calculate_step_incrementor(step_rate);
	motor_state[motor_id] = (goal_high_ticks[motor_id] >= high_ticks[motor_id]) ? FORWARDS : BACKWARDS;
	step_pool[motor_id] = motor_state[motor_id] * (goal_high_ticks[motor_id] - high_ticks[motor_id]);
	opcode[motor_id] = 'M';
	if(goal_high_ticks[motor_id] == high_ticks[motor_id]){
		system_os_post(ACK_TASK_PRIO, motor_id, 0);
	} else {
		command_done[motor_id] = 0;
	}
}

void opcode_goto(signed int step_num, unsigned short step_rate,  char motor_id)
{
	goal_high_ticks[motor_id] = ((step_num <= maximum_ticks) && (step_num >= minimum_ticks))
		? step_num : goal_high_ticks[motor_id];
	rate_counter[motor_id] = 0.0;
	rate_incrementor[motor_id] = calculate_step_incrementor(step_rate);
	motor_state[motor_id] = (goal_high_ticks[motor_id] >= high_ticks[motor_id]) ? FORWARDS : BACKWARDS;
	step_pool[motor_id] = motor_state[motor_id] * (goal_high_ticks[motor_id] - high_ticks[motor_id]);
	opcode[motor_id] = 'G';
	if(goal_high_ticks[motor_id] == high_ticks[motor_id]){
		system_os_post(ACK_TASK_PRIO, motor_id, 0);
	} else {
		command_done[motor_id] = 0;
	}
}

void opcode_stop(signed int wait_time, unsigned short precision,  char motor_id)
{
	motor_state[motor_id] = PAUSED;
	goal_high_ticks[motor_id] = PAUSED_HIGH_TICKS;
	step_pool[motor_id] = abs(wait_time) / calculate_step_incrementor (precision);
	rate_incrementor[motor_id] = 1;
	rate_counter[motor_id] = 0.0;
	opcode[motor_id] = 'S';
	if(wait_time <= 0){
		system_os_post(ACK_TASK_PRIO, motor_id, 0);
	} else {
		command_done[motor_id] = 0;
	}
}

float calculate_step_incrementor(unsigned short input_step_rate)
{
	return input_step_rate / PULSE_FREQUENCY;
}

int is_motor_running( char motor_id)
{
	return !command_done[motor_id];
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
	rate_counter[events->sig] += rate_incrementor[events->sig];
	if(rate_counter[events->sig] >= step_threshold)
	{
		int steps_to_take = rate_counter[events->sig] / step_threshold;
		rate_counter[events->sig] -= step_threshold * (float)steps_to_take;
		if(step_pool[events->sig] <= steps_to_take)
		{
			if(motor_state[events->sig] == PAUSED)
			{
				goal_high_ticks[events->sig] = high_ticks[events->sig];
			}
			next_high_ticks[events->sig] = goal_high_ticks[events->sig];
		}
		else
		{
			next_high_ticks[events->sig] += steps_to_take * motor_state[events->sig];
			step_pool[events->sig] -= steps_to_take;
		}
	}
}