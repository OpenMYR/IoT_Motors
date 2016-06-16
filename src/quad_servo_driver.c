#define ICACHE_FLASH

#include "c_types.h"
#include "gpio_driver.h"
#include "motor_driver.h"
#include "osapi.h"
#include "udp.h"
#include "user_config.h"

#define GPIO_STEP_A 4
#define GPIO_STEP_B 14
#define GPIO_STEP_C 12
#define GPIO_STEP_D 13

#define PAUSED_HIGH_TICKS 501
#define SERVO_TICKS_FLOOR 70
#define SERVO_TICKS_CEILING 250

#define MINIMUM_DUTY_CYCLE_US 1000
#define MAXIMUM_DUTY_CYCLE_US 2000
#define PULSE_LENGTH_US 20000
#define PULSE_FREQUENCY (1 / (PULSE_LENGTH_US * 0.000001))

#define MAX_HIGH_TICKS MAXIMUM_DUTY_CYCLE_US / RESOLUTION_US
#define MIN_HIGH_TICKS MINIMUM_DUTY_CYCLE_US / RESOLUTION_US
#define PULSE_LENGTH_TICKS (PULSE_LENGTH_US / RESOLUTION_US)
#define SECOND_LENGTH_TICKS 1000000 / RESOLUTION_US

//static const int quad_gpio[4] = {GPIO_STEP_A, GPIO_STEP_B, GPIO_STEP_C, GPIO_STEP_D};

static volatile int ticks = 0;
static volatile int high_ticks[4] = {[0 ... 3] = 170};
static volatile enum motor_direction motor_state[4] = {[0 ... 3] = PAUSED};
static volatile int next_high_ticks[4] = {[0 ... 3] = 170};
static volatile int goal_high_ticks[4] = {[0 ... 3] = 170};
static volatile float rate_counter[4] = {[0 ... 3] = 0.0}; 
static volatile float rate_incrementor[4] = {[0 ... 3] = 2};
static const float step_threshold = 1;
static volatile long int step_pool[4];
static volatile char opcode[4] = {[0 ... 3] = ' '};
static volatile int command_done[4] = {[0 ... 3] = 1};

static volatile int minimum_ticks = SERVO_TICKS_FLOOR;
static volatile int maximum_ticks = SERVO_TICKS_CEILING;
//static volatile int bitStates[4] = {[0 ... 3] = 1};
const unsigned int gpio_output_mask[4] = {0x0010, 0x4000, 0x1000, 0x2000};

void init_motor_gpio()
{
	eio_setup ( GPIO_STEP_A );
	eio_setup ( GPIO_STEP_B );
	eio_setup ( GPIO_STEP_C );
	eio_setup ( GPIO_STEP_D );

	//eio_quad_lo ( 1,1,1,1 );
	//eio_quad_shift(0x0000);
	eio_low(GPIO_STEP_A);
	eio_low(GPIO_STEP_B);
	eio_low(GPIO_STEP_C);
	eio_low(GPIO_STEP_D);

	eio_quad_enable();
}

void step_driver ( void )
{
	ticks++;
	if(ticks <= SERVO_TICKS_CEILING)
	{
		int n = 0;
		unsigned int gpio_output = 0;
		for(n; n < 4; n++)
		{
			//bitStates[n] = (ticks != high_ticks[n]);
			gpio_output += ((ticks <= high_ticks[n]) ? gpio_output_mask[n] : 0);
		}
		eio_quad_shift(gpio_output);
	}
	else if(ticks == PULSE_LENGTH_TICKS)
	{
		ticks = 0;
		int current_motor = 0;
		//os_printf("high: ");
		eio_quad_shift(0x7010);
		//bitStates = {0,0,0,0};
		for(current_motor; current_motor < 4; current_motor++)
		{
			//eio_high ( quad_gpio[current_motor] );
			high_ticks[current_motor] = next_high_ticks[current_motor];
			if(high_ticks[current_motor] != goal_high_ticks[current_motor])
			{
				system_os_post(MOTOR_DRIVER_TASK_PRIO, current_motor, 0);
			}
			else if(!command_done[current_motor])
			{
				system_os_post(ACK_TASK_PRIO, current_motor, 0);
				command_done[current_motor] = 1;
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