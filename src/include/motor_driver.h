#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H
#define ICACHE_FLASH

#include "c_types.h"
#include "user_interface.h"

#define RESOLUTION_US 10

#define CLOCKWISE_PINOUT 1
#define COUNTERCLOCKWISE_PINOUT -1

typedef enum
{
	MIN_SERVO_BOUND,
	MAX_SERVO_BOUND,
	MICROSTEPPING
} config_setting;

void init_motor_gpio();

void step_driver ( void );
void opcode_move(signed int step_num, unsigned short step_rate, char motor_id);
void opcode_goto(signed int step_num, unsigned short step_rate, char motor_id);
void opcode_stop(signed int wait_time, unsigned short precision, char motor_id);
float calculate_step_incrementor(unsigned short input_step_rate);
int is_motor_running(char motor_id);
void ICACHE_FLASH_ATTR change_motor_setting(config_setting input, int data);

void driver_logic_task(os_event_t *events);

enum stepper_state
{
    IDLE,
    DIRECTION_ASSERT,
    STEPPING,
	DWELL
};

enum motor_direction
{
    FORWARDS = 1,
    PAUSED = 0,
    BACKWARDS = -1
};


#endif