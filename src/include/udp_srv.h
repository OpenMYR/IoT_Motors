#pragma once

#include "ESPAsyncUDP.h"
#include "command_packets.h"
#define CTRL_PACKET_LEN_BYTES 11
#define WIFI_PACKET_LEN_BYTES 96

class udp_srv
{
    public:
        udp_srv();
        void begin();
        void prompt_broadcast();
        void end();

    private:
        void udp_packet_callback(AsyncUDPPacket& packet);
        void broadcast_ack(command_response_packet&);

        AsyncUDP udp;
};
