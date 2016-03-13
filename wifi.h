#ifndef WIFI_H
#define WIFI_H

#include "user_interface.h"

void print_ip ( unsigned int ip );
void show_ip ( void );
void wifi_event ( System_Event_t *e );
void wifi_init();

#endif