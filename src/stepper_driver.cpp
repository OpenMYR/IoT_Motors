#include "include/stepper_driver.h"
#include "include/waveformPulseCounter.h"
#include "Arduino.h"
#include "include/task_prio.h"

#define GPIO_STEP 4
#define GPIO_STEP_ENABLE 5
#define GPIO_STEP_DIR 13
#define GPIO_USTEP_A 0

//end stop
#define GPIO_IO_A 14
#define GPIO_IO_B 12

uint32_t direction = 0;
uint32_t paused = 0;
int32_t location = 0;
uint32_t command_done = 1;
uint32_t endstop_a = 0;
uint32_t endstop_b = 0;

extern "C"
{
  ICACHE_RAM_ATTR void ppcb()
  {
      if(paused == 0)
      {
        if(direction == 1)
          location++;
        else
          location--;
      }
  }

  ICACHE_RAM_ATTR void pdcb()
  {
    command_done = 1;
    if(paused == 1)
    {
      paused = 0;
      digitalWrite(GPIO_STEP_ENABLE, 0);
    }
    system_os_post(ACK_TASK_PRIO, 0, location);
  }

  ICACHE_RAM_ATTR void endstop_a_interrupt()
  {
      stopWaveform();
      if(digitalRead(GPIO_IO_A) == 0)
      {
          endstop_a = 1;
          command_done = 0;
          direction = direction == 0 ? 1 : 0;
          digitalWrite(GPIO_STEP_DIR, direction);
          startWaveform(10, (1 / ((float)10) * 1000000) - 10, UINT32_MAX);
      }
      else
      {
          endstop_a = 0;
          command_done = 1;
          system_os_post(ESTOP_TASK_PRIO, 0, 0);
      }
  }

    ICACHE_RAM_ATTR void endstop_b_interrupt()
  {
      stopWaveform();
      if(digitalRead(GPIO_IO_B) == 0)
      {
          endstop_b = 1;
          command_done = 0;
          direction = direction == 0 ? 1 : 0;
          digitalWrite(GPIO_STEP_DIR, direction);
          startWaveform(10, (1 / ((float)10) * 1000000) - 10, UINT32_MAX);
      }
      else
      {
          endstop_b = 0;
          command_done = 1;
          system_os_post(ESTOP_TASK_PRIO, 0, 1);
      }
  }
}

stepper_driver::stepper_driver() : motor_driver()
{
    init_motor_gpio();
    setPerPulseCallback(ppcb);
    setPulsesDepletedCallback(pdcb);
}

void stepper_driver::init_motor_gpio()
{
    
    pinMode(GPIO_STEP, OUTPUT);
    pinMode(GPIO_STEP_ENABLE, OUTPUT);
    pinMode(GPIO_STEP_DIR, OUTPUT);
    pinMode(GPIO_USTEP_A, OUTPUT);
    pinMode(GPIO_IO_A, INPUT_PULLUP);
    pinMode(GPIO_IO_B, INPUT_PULLUP);

    attachInterrupt(GPIO_IO_A, endstop_a_interrupt, CHANGE);
    attachInterrupt(GPIO_IO_B, endstop_b_interrupt, CHANGE);

    setWaveformPulseCountPin(GPIO_STEP);

    digitalWrite(GPIO_USTEP_A, 0);
    digitalWrite(GPIO_STEP_ENABLE, 0);
}

void stepper_driver::opcode_move(signed int step_num, unsigned short step_rate, uint8_t motor_id)
{
    //Serial.printf("move %d steps at %d steps per sec", step_num, step_rate);
    if(endstop_a || endstop_b)
        return;

    stopWaveform();
    
    uint32_t limit = abs(step_num);
    direction = step_num > 0 ? 1 : 0;
    digitalWrite(GPIO_STEP_DIR, direction);
    if(limit > 0)
    {
        command_done = 0;
        startWaveform(10, (1 / ((float)step_rate) * 1000000) - 10, limit);
    }
    else
    {
        command_done = 1;
        system_os_post(ACK_TASK_PRIO, 0, 0);
    }
}


void stepper_driver::opcode_move_cont(signed int step_num, unsigned short step_rate, uint8_t motor_id)
{
    if(endstop_a || endstop_b)
        return;

    stopWaveform();
    
    uint32_t limit = abs(step_num);
    direction = step_num > 0 ? 1 : 0;
    digitalWrite(GPIO_STEP_DIR, direction);
    if(limit > 0)
    {
        command_done = 0;
        startWaveformContinuous(10, (1 / ((float)step_rate) * 1000000) - 10, limit, 1);
    }
    else
    {
        command_done = 1;
        system_os_post(ACK_TASK_PRIO, 0, 0);
    }
}


void stepper_driver::opcode_goto(signed int step_num, unsigned short step_rate, uint8_t motor_id)
{
    if(endstop_a || endstop_b)
        return;
        
    stopWaveform();
    //Serial.printf("goto %d %d\n", step_num, step_rate);
    
    uint32_t limit = 0;

    if(location > step_num)
    {
        limit = (location - step_num);
        direction = 0;
    }
    else if(location < step_num)
    {
        limit = (step_num - location);
        direction = 1;
    }
    digitalWrite(GPIO_STEP_DIR, direction);
    if(limit > 0)
    {
        command_done = 0;
        startWaveform(10, (1 / ((float)step_rate) * 1000000) - 10, limit);
    }
    else
    {
        command_done = 1;
        system_os_post(ACK_TASK_PRIO, 0, 0);
    }
}

void stepper_driver::opcode_stop(signed int wait_time, unsigned short precision, uint8_t motor_id)
{
    if(endstop_a || endstop_b)
        return;
        
    stopWaveform();
    //Serial.printf("stop %d %d\n", wait_time, precision);
    uint32_t limit = abs(wait_time);
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
        startWaveform(10, (1 / ((float)precision) * 1000000) - 10, limit);
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

void stepper_driver::change_motor_setting(config_setting setting, uint32_t data1, uint32_t data2)
{
  if(setting == config_setting::MICROSTEPPING)
  {
    digitalWrite(GPIO_USTEP_A, data1 > 0);
  }
}