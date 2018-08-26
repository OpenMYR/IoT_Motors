#include "include/udp_srv.h"

udp_srv::udp_srv()
{

}

void udp_srv::begin()
{
    udp.connect(IPAddress(192,168,1,255),4140);
    udp.listen(4120);
    udp.onPacket([this](AsyncUDPPacket& packet){this->udp_packet_callback(packet);});
}

void udp_srv::udp_packet_callback(AsyncUDPPacket& packet)
{
    packet.printf("sup, I got %u bytes of datum", packet.length());
}

void udp_srv::prompt_broadcast()
{
    udp.broadcastTo("HEY EVERYBODY! I'M A MOTOR",4140);
}