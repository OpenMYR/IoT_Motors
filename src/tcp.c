#include "c_types.h"
#include "ip_addr.h"
#include "espconn.h"
#include "tcp.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"
#include "web_data.h"
#include "wifi.h"

#define TCP_PORT 80
#define MAX_CONNECTIONS 4

static struct _esp_tcp tcp_params;
static struct espconn tcp_server;

static struct espconn current_guy;

void (*json_packet_callback)(char *) = NULL;

static char *json_query = NULL;

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

	int test = espconn_accept(&tcp_server);
    
	if(test > 0)
	{
		os_printf("TCP Server setup failure! Code: %d", test);
	}
	
    espconn_regist_sentcb(&tcp_server, tcp_send_callback);
    espconn_regist_recvcb(&tcp_server, tcp_recv_callback);
}

void tcp_recv_callback(void *arg, char *pdata, unsigned short len)
{
	os_printf("Data Recieved!\n");
	if(os_strstr(pdata, "GET /") != NULL){
		if(os_strstr(pdata, "GET /favicon.ico") != NULL)
		{
			espconn_send(&tcp_server, favicon_ico, FAVICON_LEN + FAV_OKAY_LEN);
		}
		else if(os_strstr(pdata, "GET /css/default.css") != NULL)
		{
			espconn_send(&tcp_server, default_css, DEFAULT_CSS_LEN + CSS_OKAY_LEN);
		}
		else
		{
			espconn_send(&tcp_server, config_page, OKAY_LEN + CONFIG_PAGE_LEN);
		}
		//espconn_send(&tcp_server, okay, OK_LEN);
	}
	else if(os_strstr(pdata, "POST /") != NULL)
	{
		//os_printf(pdata);
		espconn_send(&tcp_server, post_redirect, REDIR_LEN);
		pdata = os_strstr(pdata, "{");
		if (pdata != NULL) json_packet_callback(pdata);
	}
}

void tcp_send_callback(void *arg)
{
	os_printf("Data Sent!\n");
}

void tcp_connect_callback(void *arg)
{
	current_guy = *(struct espconn*)arg;
	os_printf("Connection attempt!\n");
    //espconn_regist_reconcb(&current_guy, tcp_reconnect_callback);
    //espconn_regist_write_finish(&current_guy, tcp_write_finish_callback);
    espconn_regist_disconcb(&current_guy, tcp_disconnect_callback);
	//char *page = "<html>\n<header>\n<title>This is title</title>\n</header>\n<body>\n<h1>Hello World!</h1>\n<p>ESP8266 TCP\n</body>\n</html>";
	//espconn_send(&tcp_server, page, 114);
}

void tcp_reconnect_callback(void *arg, sint8 err)
{
	os_printf("Reconnection attempt!\n");
}

void tcp_disconnect_callback(void *arg)
{
	os_printf("TCP Disconnect!\n");
}

void tcp_write_finish_callback(void *arg)
{
	os_printf("TCP Write Finished!\n");
}

void register_tcp_json_callback(void (*json_callback)(char* json_string))
{
	json_packet_callback = json_callback;
}