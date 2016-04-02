#ifndef COMMAND_LAYER_H
#define COMMAND_LAYER_H

#include "udp.h"
#include "user_interface.h"

void initialize_command_layer();
void motor_process_command(struct stepper_command_packet *, uint8 *ip_addr);
void wifi_process_command(struct wifi_command_packet *, uint8 *ip_addr);
void fetch_command();
void issue_command();
void acknowledge_command(os_event_t *events);

#endif