#include "c_types.h"
#include "ip_addr.h"
#include "espconn.h"
#include "tcp.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"

#define TCP_PORT 80
#define MAX_CONNECTIONS 4

static struct _esp_tcp tcp_params;
static struct espconn tcp_server;



void tcp_setup( void )
{
	os_memset( &tcp_params, 0, sizeof(struct _esp_tcp ));
	tcp_params.local_port = TCP_PORT;

	uint8 tcp_local_ip[4];
    struct ip_info local_info;
    wifi_get_ip_info ( STATION_IF, &local_info );
    unsigned int local_ip = local_info.ip.addr;
    int x = 0;
    for(x; x < 4; x++)
    {
        tcp_local_ip[4-x] = local_ip & 0xff;
        local_ip >>= 8;
    }

    os_memcpy( &tcp_params.local_ip, tcp_local_ip, 32);

    tcp_server.type = ESPCONN_TCP;
    tcp_server.state = ESPCONN_NONE;
    tcp_server.proto.tcp = (esp_tcp*)os_zalloc(sizeof(esp_tcp));
    tcp_server.proto.tcp = &tcp_params;


    espconn_regist_connectcb(&tcp_server, tcp_connect_callback);


    espconn_accept(&tcp_server);
}

void tcp_recv_callback(void *arg, char *pdata, unsigned short len)
{

}

void tcp_send_callback(void *arg)
{

}

void tcp_connect_callback(void *arg)
{
	os_printf("Connection attempt!\n");
    //espconn_regist_recvcb(&tcp_server, tcp_recv_callback);
    //espconn_regist_sentcb(&tcp_server, tcp_send_callback);
    //espconn_regist_reconcb(&tcp_server, tcp_reconnect_callback);
    //espconn_regist_write_finish(&tcp_server, tcp_write_finish_callback);
    //espconn_regist_disconcb(&tcp_server, tcp_disconnect_callback);
}

void tcp_reconnect_callback(void *arg, sint8 err)
{

}

void tcp_disconnect_callback(void *arg)
{

}

void tcp_write_finish_callback(void *arg)
{

}
