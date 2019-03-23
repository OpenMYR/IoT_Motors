#pragma once

#include "motor_driver.h"
#include <Servo.h>

class quad_servo_driver : public motor_driver
{
    public:
        quad_servo_driver();

        void opcode_move(signed int step_num, unsigned short step_rate, uint8_t motor_id);
        void opcode_move_cont(signed int step_num, unsigned short step_rate, uint8_t motor_id);
        void opcode_goto(signed int step_num, unsigned short step_rate, uint8_t motor_id);
        void opcode_stop(signed int wait_time, unsigned short precision, uint8_t motor_id);
        bool is_motor_running(uint8_t motor_id);
        void driver_logic_task(os_event_t *events);
        void driver();
        void change_motor_setting(config_setting setting, uint32_t data1, uint32_t data2);
    
    private:
        void init_motor_gpio();
        float calculate_step_incrementor(unsigned short input_step_rate);

        Servo srv[4];

        struct servo_conf
        {
            uint32_t min = 0;
            uint32_t max = 180;
        };

        servo_conf confs[4];

        bool command_done[4] = {1, 1, 1, 1};

        uint16_t current_location[4] = {90, 90, 90, 90};
        uint16_t goal_location[4] = {90, 90, 90, 90};
        uint16_t next_location[4] =  {90, 90, 90, 90};
        uint16_t degree_pool[4] = {0, 0, 0, 0};

        float rate_counter[4] = {0, 0, 0, 0};
        float rate_incrementor[4] = {2, 2, 2, 2};

        motor_direction direction[4] = {PAUSED, PAUSED, PAUSED, PAUSED};
};
