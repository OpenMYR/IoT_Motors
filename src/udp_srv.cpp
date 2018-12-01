#include "include/udp_srv.h"
#include "include/command_layer.h"
#include <Arduino.h>
udp_srv::udp_srv()
{

}

void udp_srv::begin()
{
    std::function<void(command_response_packet&)> f = std::bind(&udp_srv::broadcast_ack, this, std::placeholders::_1);
    command_layer::register_udp_ack_func(f);
    udp.connect(IPAddress(192,168,1,255),4140);
    udp.listen(4120);
    udp.onPacket([this](AsyncUDPPacket& packet){this->udp_packet_callback(packet);});
}

void udp_srv::udp_packet_callback(AsyncUDPPacket& packet)
{
    packet.printf("sup, I got %u bytes of datum", packet.length());
    if(packet.length() == CTRL_PACKET_LEN_BYTES)
    {
        Serial.println("valid packet");
        stepper_command_packet motor_packet(packet.data());
        packet.printf("%u,%c,%u,%u,%d,%u",motor_packet.motor_id,motor_packet.opcode, motor_packet.port, motor_packet.queue, motor_packet.step_num, motor_packet.step_rate);
        Serial.printf("%u,%c,%u,%u,%d,%u\n",motor_packet.motor_id,motor_packet.opcode, motor_packet.port, motor_packet.queue, motor_packet.step_num, motor_packet.step_rate);
        command_layer::motor_process_command(motor_packet, packet.remoteIP());
    }
    else if(packet.length() == WIFI_PACKET_LEN_BYTES)
    {
        wifi_command_packet wifiPacket = *(wifi_command_packet*)(packet.data());
        command_layer::wifi_process_command(wifiPacket, packet.remoteIP());
    }
}

void udp_srv::prompt_broadcast()
{
    udp.broadcastTo("HEY EVERYBODY! I'M A MOTOR",4140);
}

void udp_srv::end()
{
    udp.close();
}

void udp_srv::broadcast_ack(command_response_packet& packet)
{
    udp.broadcastTo((uint8_t*)(&packet), sizeof(command_response_packet), 4140);
}