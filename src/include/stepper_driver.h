#pragma once

#include "motor_driver.h"

class stepper_driver : public motor_driver
{
    public:
        stepper_driver();

        void opcode_move(signed int step_num, unsigned short step_rate, uint8_t motor_id);
        void opcode_goto(signed int step_num, unsigned short step_rate, uint8_t motor_id);
        void opcode_stop(signed int wait_time, unsigned short precision, uint8_t motor_id);
        bool is_motor_running(uint8_t motor_id);
        void driver_logic_task(os_event_t *events);
        void driver();
        void change_motor_setting(config_setting setting, uint32_t data1, uint32_t data2);
    
    private:
        void init_motor_gpio();
        float calculate_step_incrementor(unsigned short input_step_rate);
};