#ifndef OP_QUEUE_H
#define OP_QUEUE_H

#include "command_packets.h"

#if MOTOR_TYPE == 0
#define MOTOR_COUNT 4
#else
#define MOTOR_COUNT 1
#endif
#define BUFFER_SIZE (1000 / MOTOR_COUNT)

class op_queue
{
    public:
        op_queue(){};
        void store_command(struct stepper_command_packet *, IPAddress *, uint8_t);
        stepper_command_data* get_command(uint8_t);
        int remove_first_command(uint8_t);
        void clear_queue(uint8_t);
        bool is_queue_empty(uint8_t);
        unsigned short get_length(uint8_t motor_id)
        {
            return queue_length[motor_id];
        }

    private:
        struct stepper_command_data command_queue[MOTOR_COUNT][BUFFER_SIZE];
        unsigned short queue_length[MOTOR_COUNT] = {0};
        unsigned short first_element[MOTOR_COUNT] = {1};
        unsigned short last_element[MOTOR_COUNT] = {0};
};

#endif