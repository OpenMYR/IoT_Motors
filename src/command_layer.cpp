#include "include/command_layer.h"
#include "include/quad_servo_driver.h"

stepper_command_packet command_layer::current_command[4];
IPAddress command_layer::current_addr[4];
motor_driver* command_layer::motor = nullptr;
Ticker command_layer::motor_drv_timer;

command_layer::command_layer()
{

}

void command_layer::init_motor_driver()
{
    motor = new quad_servo_driver();
    motor_drv_timer.attach_ms(20,command_layer::motor_drv_isr);
}

void command_layer::motor_driver_task_passthrough(os_event_t *events)
{
    motor->driver_logic_task(events);
}

void command_layer::motor_process_command(struct stepper_command_packet packet, IPAddress addr)
{
    if(packet.motor_id < 4)
    {
        current_command[packet.motor_id] = packet;
        current_addr[packet.motor_id] = addr;
        issue_command(packet.motor_id);
    }
}

void command_layer::wifi_process_command(struct wifi_command_packet packet, IPAddress addr)
{

}

void command_layer::json_process_command(char *json_input)
{

}

void command_layer::acknowledge_command(os_event_t *events)
{

}

void command_layer::fetch_command(uint8_t)
{

}

void command_layer::issue_command(uint8_t motor_id)
{
    if(current_command[motor_id].opcode == 'S')
    {
        motor->opcode_stop(current_command[motor_id].step_num, current_command[motor_id].step_rate, motor_id);
    }
    else if(current_command[motor_id].opcode == 'M')
    {
        motor->opcode_move(current_command[motor_id].step_num, current_command[motor_id].step_rate, motor_id);
    }
    else if(current_command[motor_id].opcode == 'G')
    {
        motor->opcode_goto(current_command[motor_id].step_num, current_command[motor_id].step_rate, motor_id);
    }
    else if(current_command[motor_id].opcode == 'U')
    {
        //motor->change_motor_setting microstepping
    }
    //TODO: change motor setting min and max degree
}

void command_layer::motor_drv_isr()
{
    motor->driver();
}