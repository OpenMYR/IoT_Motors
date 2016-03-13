#ifndef UDP_H
#define UDP_H

#include "c_types.h"
#include "ip_addr.h"

#define PACKET_LEN_BYTES 10

uint16 ntohs( uint16 input );
int32 ntohl( int32 input );
void udp_recv_callback(void *arg, char *pdata, unsigned short len);
void udp_send_callback(void *arg);
void udp_setup( void );
void udp_send_ack( char opcode, int position, uint8 *ip_addr, unsigned short);

struct stepper_command_packet {
    unsigned short port;
    char opcode;
    uint8 queue;
    signed int step_num;
    unsigned short step_rate;
};

struct command_response_packet {
	char opcode;
	int position;
};

void register_udp_packet_callback(void (*packet_callback)(struct stepper_command_packet *, uint8 *ip_addr));

#endif