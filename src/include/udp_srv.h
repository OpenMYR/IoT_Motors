#ifndef UDP_SRV_H
#define UDP_SRV_H

#include "ESPAsyncUDP.h"

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

#endif