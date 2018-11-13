#pragma once

#include <os_type.h>
#include <Ticker.h>
#include "IPAddress.h"
#include "command_packets.h"
#include "motor_driver.h"
#include "op_queue.h"
#include <ArduinoJson.h>

class command_layer
{
    public:
        static void init_motor_driver();
        static void motor_process_command(struct stepper_command_packet, IPAddress);
        static void wifi_process_command(struct wifi_command_packet, IPAddress);
        static void json_process_command(const char *json_input);
        static void acknowledge_command(os_event_t *events);
        static void motor_driver_task_passthrough(os_event_t *events);
        static void stop_motor();

    private:
        command_layer();
        static bool ota_active;
        static void fetch_command(uint8_t);
        static void issue_command(uint8_t);

        static void issue_stop_packet(uint8_t motor_id);

        static stepper_command_packet current_command[4];
        static IPAddress current_addr[4];

        static motor_driver* motor;
        static Ticker motor_drv_timer;
        static void motor_drv_isr();

        static op_queue command_queue;

        static StaticJsonBuffer<JSON_OBJECT_SIZE(100)> jsonBuf;
};