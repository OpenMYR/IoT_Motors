#include "udp.h"
#include "motor_driver.h"
#include "c_types.h"
#include "ip_addr.h"
#include "espconn.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

static int udp_remote_port = 11000;
static int udp_local_port = 11000;
static uint8 remote_ip[4] = { 0, 0, 0, 0 };

static struct _esp_udp udp_params;
static struct espconn udp_server;

void (*motor_packet_callback)(struct stepper_command_packet *, uint8 *ip_addr) = NULL;
void (*wifi_packet_callback)(struct wifi_command_packet *, uint8 *ip_addr) = NULL;

uint16 ntohs( uint16 input )
{
    return ((input & 0xFF00) >> 8) + ((input & 0x00FF) << 8);
}

int32 ntohl( int32 input )
{
    return ((input & 0xFF000000) >> 24) + ((input & 0x00FF0000) >> 8) +
        ((input & 0x0000FF00) << 8) + ((input & 0x000000FF) << 24);
}

void udp_recv_callback(void *arg, char *pdata, unsigned short len)
{
	struct espconn *orig_connection = (struct espconn*)arg;
	remot_info *remote = NULL;
	espconn_get_connection_info(orig_connection, &remote, 0);
	if(len == CTRL_PACKET_LEN_BYTES)
	{
		motor_packet_callback(pdata,  remote->remote_ip);
	}
	else if(len == WIFI_PACKET_LEN_BYTES)
	{
		wifi_packet_callback(pdata,  remote->remote_ip);
	}
	else
	{
		os_printf("Packet incorrect size! Frog blast the vent core!\n");
	}
	
}

void udp_send_callback(void *arg)
{
    
    //os_printf("callback sent\n");
}

void udp_setup(void)
{
    //Set memory and copy port info
    os_memset( &udp_params, 0, sizeof(struct _esp_udp ));
    //os_memcpy( &udp_params.remote_port, udp_remote_port, 32 );
    udp_params.remote_port = udp_remote_port;
    //os_memcpy( &udp_params.local_port, udp_local_port, 32 );
    udp_params.local_port = udp_local_port; 

    //Extract local IP and store in an array of 4 8-bit ints
    //The local IP is stored backward for some inane reason so reverse it
    //for the UDP struct
    uint8 udp_local_ip[4];
    struct ip_info local_info;
    wifi_get_ip_info ( STATION_IF, &local_info );
    unsigned int local_ip = local_info.ip.addr;
    int x = 0;
    for(x; x < 4; x++)
    {
        udp_local_ip[4-x] = local_ip & 0xff;
        local_ip >>= 8;
    }

    //Copy IPs into struct
    os_memcpy( &udp_params.local_ip, udp_local_ip, 32);
    os_memcpy( &udp_params.remote_ip, &remote_ip, 32);

    udp_server.type = ESPCONN_UDP;
    udp_server.state = ESPCONN_NONE;
    udp_server.proto.udp = (esp_udp*)os_zalloc(sizeof(esp_udp));
    udp_server.proto.udp = &udp_params;
    espconn_regist_recvcb(&udp_server, udp_recv_callback);
    espconn_regist_sentcb(&udp_server, udp_send_callback);

    espconn_create(&udp_server);
    
    
}

void udp_send_ack( char opcode, int position, uint8 *ip_addr, unsigned short port)
{
    os_memcpy(udp_server.proto.udp->remote_ip, ip_addr, 4);
	//os_printf("%d.%d.%d.%d - %d\n", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3], port);
    udp_server.proto.udp->remote_port = port;
	struct command_response_packet ack;
	ack.opcode = opcode;
	ack.position = ntohl(position);
	espconn_send(&udp_server, &ack, 5);
}

void register_motor_packet_callback(void (*motor_process_callback)(struct stepper_command_packet *, uint8 *ip_addr))
{
	motor_packet_callback = motor_process_callback;
}

void register_wifi_packet_callback(void (*wifi_process_callback)(struct wifi_command_packet *, uint8 *ip_addr))
{
	wifi_packet_callback = wifi_process_callback;
}