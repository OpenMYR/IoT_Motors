/* Portions of this code is demo code by
 * Tom Trebisky 
 * http://cholla.mmto.org/esp8266/sdk/wifi.c
 * 
 * New code will replace this in public release
 *
 */

#include "eagle_soc.h"
#include "wifi.h"
#include "udp.h"
#include "tcp.h"
#include "ip_addr.h"
#include "user_interface.h"
#include "user_config.h"
#include "osapi.h"
#include "mem.h"

#define CONCURRENT_SOFTAP_CONNECTIONS 1

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
	int event = e->event;
	if(wifi_get_opmode() != SOFTAP_MODE)
	{
		if ( event == EVENT_STAMODE_GOT_IP ) {
			os_printf ( "Event, got IP\n" );
			show_ip ();
			udp_setup();
			tcp_setup();
		} else if ( event == EVENT_STAMODE_CONNECTED ) {
			os_printf ( "Event, connected\n" );
			WRITE_PERI_REG(RTC_GPIO_OUT, 0);
		} else if ( event == EVENT_STAMODE_DISCONNECTED ) {
			WRITE_PERI_REG(RTC_GPIO_OUT, 1);
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
	else if(wifi_get_opmode() == SOFTAP_MODE)
	{
		if (event == EVENT_SOFTAPMODE_STACONNECTED) {
			WRITE_PERI_REG(RTC_GPIO_OUT, 0);
		} else if (event == EVENT_SOFTAPMODE_STADISCONNECTED) {
			if(wifi_softap_get_station_num() < 1) WRITE_PERI_REG(RTC_GPIO_OUT, 1);
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
		wifi_station_disconnect();
		struct softap_config apConfig;
		wifi_softap_get_config(&apConfig);
		apConfig.channel = 7;
		apConfig.max_connection = CONCURRENT_SOFTAP_CONNECTIONS;
		apConfig.ssid_hidden = 0;
		wifi_softap_set_config(&apConfig);
	}
		//system_restart();
}

void wifi_init()
{
	WRITE_PERI_REG(PAD_XPD_DCDC_CONF, (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | 0x00000001);
	WRITE_PERI_REG(RTC_GPIO_CONF, (READ_PERI_REG(RTC_GPIO_CONF) & 0xfffffffe) | 0x00000000);
	WRITE_PERI_REG(RTC_GPIO_OUT, 1);
	WRITE_PERI_REG(RTC_GPIO_ENABLE, 1);
	/*
	wifi_station_set_auto_connect(1);
	wifi_station_set_reconnect_policy(1); */
	struct softap_config apConfig;
	struct station_config conf;
	switch (wifi_get_opmode()) {
		case BROADCAST:
			wifi_softap_get_config(&apConfig);
			apConfig.channel = 7;
			apConfig.authmode = 0;
			uint8 macaddr[6] = {0,0,0,0,0,0};
			uint8 macchars[6] = {0,0,0,0,0,0};
			int macplace = 0;
			wifi_get_macaddr(SOFTAP_IF, macaddr);
			for(macplace; macplace < 3; macplace++)
			{
				uint8 msb = (macaddr[macplace + 3] & 0xF0) >> 4;
				uint8 lsb = macaddr[macplace +3] & 0x0F;
				macchars[macplace+(1*macplace)] = (msb < 10) ? msb + 48 : msb + 55;
				macchars[macplace+(1*macplace)+1] = (lsb < 10) ? lsb + 48 : lsb + 55;
			}
			uint8 ssid_gen[15] = {'O', 'p', 'e', 'n', 'M', 'Y', 'R', '_', macchars[0], macchars[1], macchars[2], macchars[3], macchars[4], macchars[5], 0 };
			os_memcpy(&apConfig.ssid, ssid_gen, 32);
			apConfig.ssid_len = os_strlen(ssid_gen);
			apConfig.max_connection = CONCURRENT_SOFTAP_CONNECTIONS;
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
	int one = 1;
	wifi_softap_set_dhcps_offer_option(OFFER_ROUTER, &one);
	udp_setup();
	tcp_setup();
	wifi_set_event_handler_cb ( wifi_event );

}