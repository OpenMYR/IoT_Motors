#pragma once

#include <c_types.h>

struct stepper_command_packet {
    unsigned short port;
    char opcode;
    uint8_t queue;
    signed int step_num;
    unsigned short step_rate;
    uint8_t motor_id;

    stepper_command_packet()
    {

    }

    stepper_command_packet(uint8_t* data)
    {
        port = (data[0] << 8) | data[1];
        opcode = data[2];
        queue = data[3];
        step_num = (((data[4] << 8) | data[5]) << 16) | ((data[6] << 8) | data[7]);
        step_rate = (data[8] << 8) | data[9];
        motor_id = data[10]; 
    }
};

struct command_response_packet {
	char opcode;
	int position;
};

struct wifi_command_packet {
	char opcode;
	char ssid[32] ;
	char password[63];
};