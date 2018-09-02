#pragma once

#include <os_type.h>
#include "command_packets.h"

class command_layer
{
    public:
        static void motor_process_command(struct stepper_command_packet *, uint8 *ip_addr);
        static void wifi_process_command(struct wifi_command_packet *, uint8 *ip_addr);
        static void json_process_command(char *json_input);
        static void acknowledge_command(os_event_t *events);

    private:
        command_layer();
        static void fetch_command(char);
        static void issue_command(char);
};