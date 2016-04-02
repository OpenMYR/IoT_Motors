/* Portions of this code is demo code by
 * Tom Trebisky 
 * http://cholla.mmto.org/esp8266/sdk/wifi.c
 * 
 * New code will replace this in public release
 *
 */

#include "wifi.h"
#include "udp.h"
#include "tcp.h"
#include "ip_addr.h"
#include "user_interface.h"
#include "user_config.h"
#include "osapi.h"
#include "mem.h"

static int num_retries = 0;

void print_ip ( unsigned int ip )
{
    int n1, n2, n3, n4;

    n1 = ip & 0xff;
    ip >>= 8;
    n2 = ip & 0xff;
    ip >>= 8;
    n3 = ip & 0xff;
    ip >>= 8;
    n4 = ip & 0xff;
    os_printf ( "%d.%d.%d.%d", n1, n2, n3, n4 );
}

void show_ip ( void )
{
    struct ip_info info;

    wifi_get_ip_info ( STATION_IF, &info );
    os_printf ( "IP: %08x\n", info.ip );
    os_printf ( "IP: " );
	print_ip ( info.ip.addr );
	os_printf ( "\n" );
}

void wifi_event ( System_Event_t *e )
{
	if(wifi_get_opmode() != SOFTAP_MODE)
	{
		int event = e->event;
		if ( event == EVENT_STAMODE_GOT_IP ) {
			os_printf ( "Event, got IP\n" );
			show_ip ();
			udp_setup();
			tcp_setup();
		} else if ( event == EVENT_STAMODE_CONNECTED ) {
			os_printf ( "Event, connected\n" );
		} else if ( event == EVENT_STAMODE_DISCONNECTED ) {
			os_printf ( "Event, disconnected\n" );
			if(num_retries <= 3)
			{
				num_retries++;
			}
			else
			{
				//wifi_station_set_reconnect_policy(0);
				num_retries = 0;
				change_opmode(BROADCAST, "", "");
			}
			os_printf("Reason: %d\n",wifi_station_get_connect_status());
		} else {
			os_printf ( "Unknown event %d !\n", event );
		}
	}
}

void change_opmode(mode_switch newmode, char *ssid, char *pass)
{
	if(wifi_get_opmode() != newmode) wifi_set_opmode(newmode);
	if(newmode == STATION_CONNECT)
	{
		struct station_config conf;
		os_memset ( &conf, 0, sizeof(struct station_config) );
		os_memcpy (&conf.ssid, ssid, 32);
		os_memcpy (&conf.password, pass, 64 );
		wifi_station_set_config (&conf); 
		wifi_station_connect();
	}
	else if(newmode == BROADCAST)
	{
		struct softap_config apConfig;
		wifi_softap_get_config(&apConfig);
		apConfig.channel = 7;
		apConfig.max_connection = 8;
		apConfig.ssid_hidden = 0;
		wifi_softap_set_config(&apConfig);
	}
		//system_restart();
}

void wifi_init()
{
	/*
	wifi_station_set_auto_connect(1);
	wifi_station_set_reconnect_policy(1); */
	struct softap_config apConfig;
	struct station_config conf;
	switch (wifi_get_opmode()) {
		case BROADCAST:
			wifi_softap_get_config(&apConfig);
			apConfig.channel = 7;
			apConfig.max_connection = 8;
			apConfig.ssid_hidden = 0;
			wifi_softap_set_config(&apConfig);
			break;
		case STATION_CONNECT:
			wifi_station_get_config(&conf);
			//wifi_station_set_config (&conf); 
			break;
		default:
			break;
	}
	udp_setup();
	tcp_setup();
	wifi_set_event_handler_cb ( wifi_event );

}