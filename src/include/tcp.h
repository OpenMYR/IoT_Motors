#ifndef TCP_H
#define TCP_H

#include "c_types.h"

void tcp_setup(void);
void tcp_recv_callback(void *arg, char *pdata, unsigned short len);
void tcp_send_callback(void *arg);
void tcp_connect_callback(void *arg);
void tcp_reconnect_callback(void *arg, sint8 err);
void tcp_disconnect_callback(void *arg);
void tcp_write_finish_callback(void *arg);
void register_tcp_json_callback(void (*json_callback)(char* json_string));
int find_connection(uint8* current_ip_address, int current_port);

#endif