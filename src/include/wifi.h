#ifndef WIFI_H
#define WIFI_H

#include "user_interface.h"


typedef enum {
	MODE_NULL = 0,
	STATION_CONNECT = 1,
	BROADCAST = 2,
	STATION_AND_BROADCAST = 3
} mode_switch;

void print_ip ( unsigned int ip );
void show_ip ( void );
void wifi_event ( System_Event_t *e );
void wifi_init();
void change_opmode(mode_switch, char*, char*);


#endif