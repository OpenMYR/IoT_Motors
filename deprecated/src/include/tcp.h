#ifndef TCP_H
#define TCP_H

#define ICACHE_FLASH

#include "c_types.h"

void ICACHE_FLASH_ATTR tcp_setup(void);
void ICACHE_FLASH_ATTR tcp_recv_callback(void *arg, char *pdata, unsigned short len);
void ICACHE_FLASH_ATTR tcp_send_callback(void *arg);
void ICACHE_FLASH_ATTR tcp_connect_callback(void *arg);
void ICACHE_FLASH_ATTR tcp_reconnect_callback(void *arg, sint8 err);
void ICACHE_FLASH_ATTR tcp_disconnect_callback(void *arg);
void ICACHE_FLASH_ATTR tcp_write_finish_callback(void *arg);
void ICACHE_FLASH_ATTR register_tcp_json_callback(void (*json_callback)(char* json_string));
int ICACHE_FLASH_ATTR find_connection(uint8* current_ip_address, int current_port);

#endif