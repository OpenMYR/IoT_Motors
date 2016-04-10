#define ICACHE_FLASH

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
#define MAX_CONNECTIONS 5

static struct _esp_tcp tcp_params;
static struct espconn tcp_server;

static struct espconn *tcp_connections[MAX_CONNECTIONS];

void (*json_packet_callback)(char *) = NULL;

void ICACHE_FLASH_ATTR tcp_setup( void )
{
	int p = 0;
	for(p = 0; p < MAX_CONNECTIONS; p++)
	{
		tcp_connections[p] = (struct espconn*)NULL;
	}

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

	espconn_regist_time(&tcp_server, 5, 0);
    espconn_regist_connectcb(&tcp_server, tcp_connect_callback);

	int test = espconn_accept(&tcp_server);
    
	if(test > 0)
	{
		os_printf("TCP Server setup failure! Code: %d", test);
	}
	
	espconn_tcp_set_max_con(MAX_CONNECTIONS);
    //espconn_regist_sentcb(&tcp_server, tcp_send_callback);
    espconn_regist_recvcb(&tcp_server, tcp_recv_callback);
}

void ICACHE_FLASH_ATTR tcp_recv_callback(void *arg, char *pdata, unsigned short len)
{
	struct espconn current = *(struct espconn*)arg;
	int conn = find_connection(current.proto.tcp->remote_ip, current.proto.tcp->remote_port);
	os_memcpy(tcp_server.proto.tcp->remote_ip, current.proto.tcp->remote_ip, 4 );
	tcp_server.proto.tcp->remote_port = current.proto.tcp->remote_port;
	if(os_strstr(pdata, "GET /") != NULL){
		if(os_strstr(pdata, "GET /favicon.ico") != NULL)
		{
			espconn_send(&tcp_server, favicon_ico, FAVICON_LEN + FAV_OK_LEN);
		}
		else if(os_strstr(pdata, "GET /css/default.css") != NULL)
		{
			espconn_send(&tcp_server, default_css, DEFAULT_CSS_LEN + CSS_OK_LEN);
		}
		else if(os_strstr(pdata, "GET /images/myr_logo.png") != NULL)
		{
			espconn_send(&tcp_server, myr_logo_png, MYR_LOGO_LEN + PNG_OK_LEN);
		}
		else if(os_strstr(pdata, "GET /images/body_bg.jpg") != NULL)
		{
			espconn_send(&tcp_server, body_bg_jpg, BODY_BG_LEN + JPG_OK_LEN);
		}
		else if(os_strstr(pdata, "GET /config.html") != NULL)
		{
			espconn_send(&tcp_server, config_page, CONFIG_PAGE_LEN + CONFIG_OK_LEN);
		}
		else if(os_strstr(pdata, "GET /control.html") != NULL)
		{
			espconn_send(&tcp_server, control_page, CONTROL_PAGE_LEN + CONTROL_OK_LEN);
		}
		else
		{
			espconn_send(&tcp_server, index_page, INDEX_PAGE_LEN + INDEX_OK_LEN);
		}
	}
	else if(os_strstr(pdata, "POST /") != NULL)
	{
		pdata = os_strstr(pdata, "{");
		if (pdata != NULL)
		{
			json_packet_callback(pdata);
		}
		espconn_send(&tcp_server, post_redirect, REDIR_LEN);
	}
}

void ICACHE_FLASH_ATTR tcp_send_callback(void *arg)
{
	os_printf("Data Sent!\n");
}

void ICACHE_FLASH_ATTR tcp_connect_callback(void *arg)
{
	int x = 0;
	while((x < MAX_CONNECTIONS) && (tcp_connections[x] != NULL)){ x++; }
	if(x == MAX_CONNECTIONS)
	{
		os_printf("Connection attempt: No connection slots!\n");
		espconn_disconnect((struct espconn*)arg);
	}
	else
	{
		tcp_connections[x] = os_malloc(sizeof(struct espconn));
		*(tcp_connections[x]) = *(struct espconn*)arg;
		//espconn_regist_reconcb(tcp_connections[x], tcp_reconnect_callback);
		//espconn_regist_write_finish(tcp_connections[x], tcp_write_finish_callback);
		espconn_regist_disconcb(tcp_connections[x], tcp_disconnect_callback);
	}
}

void ICACHE_FLASH_ATTR tcp_reconnect_callback(void *arg, sint8 err)
{
	os_printf("Reconnection attempt!\n");
}

void ICACHE_FLASH_ATTR tcp_disconnect_callback(void *arg)
{
	struct espconn current = *(struct espconn*)arg;
	int conn = find_connection(current.proto.tcp->remote_ip, current.proto.tcp->remote_port);
	if(conn < 0)
	{
		os_printf("TCP Disconnect: Connection %d not found!\n", conn);
	}
	else
	{
		os_free(tcp_connections[conn]);
		tcp_connections[conn] = NULL;
	}
}

void ICACHE_FLASH_ATTR tcp_write_finish_callback(void *arg)
{
	os_printf("TCP Write Finished!\n");
}

void ICACHE_FLASH_ATTR register_tcp_json_callback(void (*json_callback)(char* json_string))
{
	json_packet_callback = json_callback;
}

int ICACHE_FLASH_ATTR find_connection(uint8* current_ip_address, int current_port)
{
	int x = 0;
	int break_while = 0;
	while((x < MAX_CONNECTIONS) && !break_while)
	{ 
		if(tcp_connections[x] != NULL)
		{
			if((os_memcmp(tcp_connections[x]->proto.tcp->remote_ip, current_ip_address, 4 ) == 0) &&
				(tcp_connections[x]->proto.tcp->remote_port == current_port))
			{
				break_while = 1;
			}
			else
			{
				x++;
			}
		}
		else
		{
			x++;
		}			
	}
	if(x == MAX_CONNECTIONS)
	{
		return -1;
	}
	return x;
}