#pragma once

#include <os_type.h>
#include <user_interface.h>

class motor_driver
{
    public:
        motor_driver()
        {
        };

        enum config_setting
        {
            MIN_SERVO_BOUND,
            MAX_SERVO_BOUND,
            MICROSTEPPING,
            HOMECONFIG
        };

        virtual void opcode_move(signed int step_num, unsigned short step_rate, uint8_t motor_id);
        virtual void opcode_goto(signed int step_num, unsigned short step_rate, uint8_t motor_id);
        virtual void opcode_stop(signed int wait_time, unsigned short precision, uint8_t motor_id);
        virtual void opcode_home(signed int step_num, unsigned short step_rate, uint8_t motor_id);

        virtual bool is_motor_running(uint8_t motor_id);
        
        virtual void driver_logic_task(os_event_t *events);
        
        virtual void driver();

        virtual void change_motor_setting(config_setting setting, uint32_t data1, uint32_t data2);
    
    protected:
        enum motor_direction
        {
            FORWARDS = 1,
            PAUSED = 0,
            BACKWARDS = -1
        };

    private:
        virtual void init_motor_gpio();
        virtual float calculate_step_incrementor(unsigned short input_step_rate);
};