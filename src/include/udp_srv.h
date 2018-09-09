#pragma once

#include "ESPAsyncUDP.h"
#define CTRL_PACKET_LEN_BYTES 11

class udp_srv
{
    public:
        udp_srv();
        void begin();
        void prompt_broadcast();

    private:
        void udp_packet_callback(AsyncUDPPacket& packet);

        AsyncUDP udp;
};
