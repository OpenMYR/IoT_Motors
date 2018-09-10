#include "include/quad_servo_driver.h"
#include "include/task_prio.h"
#include <algorithm>

quad_servo_driver::quad_servo_driver() : motor_driver()
{
    init_motor_gpio();
}

void quad_servo_driver::init_motor_gpio()
{
    srv[1].attach(4);
    srv[2].attach(14);
    srv[3].attach(12);
    srv[4].attach(13);
}

void quad_servo_driver::opcode_move(signed int step_num, unsigned short step_rate, uint8_t motor_id)
{
    int16_t adj_location = current_location[motor_id] + step_num;
    goal_location[motor_id] = (adj_location > 180) ? 180 : (adj_location < 0 ? 0 : adj_location);
    rate_counter[motor_id] = 0;
    rate_incrementor[motor_id] = calculate_step_incrementor(step_rate);
    direction[motor_id] = goal_location[motor_id] >= current_location[motor_id] ? FORWARDS : BACKWARDS;
    degree_pool[motor_id] = direction[motor_id] * (goal_location[motor_id] - current_location[motor_id]);
    //store off the opcode
    if(goal_location[motor_id] == current_location[motor_id])
    {
        system_os_post(ACK_TASK_PRIO, motor_id, 0);
    }
    else
    {
        command_done[motor_id] = 0;
    }
}

void quad_servo_driver::opcode_goto(signed int step_num, unsigned short step_rate, uint8_t motor_id)
{
    //if new location is in range, set the goal, oterwise preserve current goal
    goal_location[motor_id] = ((step_num <= 180) && (step_num >= 0)) ? step_num : goal_location[motor_id];
    rate_counter[motor_id] = 0;
    rate_incrementor[motor_id] = calculate_step_incrementor(step_rate);
    direction[motor_id] = goal_location[motor_id] >= current_location[motor_id] ? FORWARDS : BACKWARDS;
    degree_pool[motor_id] = direction[motor_id] * (goal_location[motor_id] - current_location[motor_id]);
    //store off the opcode
    if(goal_location[motor_id] == current_location[motor_id])
    {
        system_os_post(ACK_TASK_PRIO, motor_id, 0);
    }
    else
    {
        command_done[motor_id] = 0;
    }
}

void quad_servo_driver::opcode_stop(signed int wait_time, unsigned short precision, uint8_t motor_id)
{
    direction[motor_id] = PAUSED;
    goal_location[motor_id] = 181;
    degree_pool[motor_id] = abs(wait_time) / calculate_step_incrementor(precision);
    rate_incrementor[motor_id] = 1;
    rate_counter[motor_id] = 0;
    //save opcode
    if(wait_time <= 0)
    {
        system_os_post(ACK_TASK_PRIO, motor_id, 0);
    }
    else
    {
        command_done[motor_id] = 0;
    }
}

bool quad_servo_driver::is_motor_running(uint8_t motor_id)
{
    return !command_done[motor_id];
}

float quad_servo_driver::calculate_step_incrementor(unsigned short input_step_rate)
{
    float inc = input_step_rate / 50.0;
    Serial.printf("%u,%f\n",input_step_rate, inc);
    return inc;
}

void quad_servo_driver::driver_logic_task(os_event_t *events)
{
    rate_counter[events->sig] += rate_incrementor[events->sig];

    if(rate_counter[events->sig] >= 1)
    { 
        unsigned int degrees_to_move = rate_counter[events->sig];
        rate_counter[events->sig] -= degrees_to_move;
        if(degrees_to_move >= degree_pool[events->sig])
        {
            if(direction[events->sig] == PAUSED)
                goal_location[events->sig] = current_location[events->sig];
            next_location[events->sig] = goal_location[events->sig];
        }
        else
        {
            next_location[events->sig] += degrees_to_move * direction[events->sig];
            degree_pool[events->sig] -= degrees_to_move;
        }
    }
}

void quad_servo_driver::driver()
{
    for(int i = 0; i < 4; i++)
    {
        if(!command_done[i])
        {
            if(current_location[i] != goal_location[i])
            {
                current_location[i] = next_location[i];
                srv[i].write(next_location[i]);
                system_os_post(MOTOR_DRIVER_TASK_PRIO, i, 0);
            }
            else
            {
                system_os_post(ACK_TASK_PRIO, i, 0);
                command_done[i] = 1;
            }
        }
    }
}