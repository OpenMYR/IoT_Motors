#include "include/stepper_driver.h"
#include "Arduino.h"
#include "include/task_prio.h"
#include "core_esp8266_waveform.h"

#define GPIO_STEP 4
#define GPIO_STEP_ENABLE 5
#define GPIO_STEP_DIR 13
#define GPIO_USTEP_A 0

//end stop
#define GPIO_IO_A 14
#define GPIO_IO_B 12

uint32_t direction = 0;
uint32_t paused = 0;
uint32_t limit = 0;
int32_t location = 0;
uint32_t command_done = 1;

extern "C"
{

    uint32_t ICACHE_RAM_ATTR stepper_isr()
    {
        limit--;
        if(paused == 0)
        {
            if(direction == 1)
                location++;
            else
                location--;
        }

        if(limit == 0)
        {
            stopWaveform(GPIO_STEP);
            command_done = 1;
            if(paused == 1)
            {
                paused = 0;
                digitalWrite(GPIO_STEP_ENABLE, 0);
            }
            system_os_post(ACK_TASK_PRIO, 0, 0);
        }
        return UINT32_MAX;
    }
}

stepper_driver::stepper_driver() : motor_driver()
{
    setTimer1Callback(&stepper_isr);
    init_motor_gpio();
}

void stepper_driver::init_motor_gpio()
{
    
    pinMode(GPIO_STEP, OUTPUT);
    pinMode(GPIO_STEP_ENABLE, OUTPUT);
    pinMode(GPIO_STEP_DIR, OUTPUT);
    pinMode(GPIO_USTEP_A, OUTPUT);

    digitalWrite(GPIO_USTEP_A, 0);
    digitalWrite(GPIO_STEP_ENABLE, 0);
}

void stepper_driver::opcode_move(signed int step_num, unsigned short step_rate, uint8_t motor_id)
{
    //Serial.printf("move %d steps at %d steps per sec", step_num, step_rate);
    stopWaveform(GPIO_STEP);
    limit = abs(step_num) * 2;
    direction = step_num > 0 ? 1 : 0;
    digitalWrite(GPIO_STEP_DIR, direction);
    if(limit > 0)
    {
        command_done = 0;
        startWaveform(GPIO_STEP, 10, (1 / ((float)step_rate) * 1000000) - 10, 0);
    }
    else
    {
        command_done = 1;
        system_os_post(ACK_TASK_PRIO, 0, 0);
    }
}

void stepper_driver::opcode_goto(signed int step_num, unsigned short step_rate, uint8_t motor_id)
{
    stopWaveform(GPIO_STEP);
    //Serial.printf("goto %d %d\n", step_num, step_rate);
    if(location > step_num)
    {
        limit = (location - step_num) * 2;
        direction = 0;
    }
    else if(location < step_num)
    {
        limit = (step_num - location) * 2;
        direction = 1;
    }
    digitalWrite(GPIO_STEP_DIR, direction);
    if(limit > 0)
    {
        command_done = 0;
        startWaveform(GPIO_STEP, 10, (1 / ((float)step_rate) * 1000000) - 10, 0);
    }
    else
    {
        command_done = 1;
        system_os_post(ACK_TASK_PRIO, 0, 0);
    }
}

void stepper_driver::opcode_stop(signed int wait_time, unsigned short precision, uint8_t motor_id)
{
    stopWaveform(GPIO_STEP);
    //Serial.printf("stop %d %d\n", wait_time, precision);
    limit = abs(wait_time) * 2;
    if(limit == 0)
    {
        command_done = 1;
        system_os_post(ACK_TASK_PRIO, 0, 0);
    }
    else
    {
        command_done = 0;
        paused = 1;
        digitalWrite(GPIO_STEP_ENABLE, 1);
        startWaveform(GPIO_STEP, 10, (1 / ((float)precision) * 1000000) - 10, 0);
    }
}

bool stepper_driver::is_motor_running(uint8_t motor_id)
{
    return command_done == 0;
}

void stepper_driver::driver_logic_task(os_event_t *events)
{

}

void stepper_driver::driver()
{

}

float stepper_driver::calculate_step_incrementor(unsigned short input_step_rate)
{
    return 0;
}

