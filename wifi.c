/* Portions of this code is demo code by
 * Tom Trebisky 
 * http://cholla.mmto.org/esp8266/sdk/wifi.c
 * 
 * New code will replace this in public release
 *
 */

#include "wifi.h"
#include "udp.h"
#include "ip_addr.h"
#include "user_interface.h"
#include "user_config.h"
#include "osapi.h"
#include "mem.h"

static char *ssid = STATION_SSID;
static char *pass = STATION_PASS;

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

    if ( event == EVENT_STAMODE_GOT_IP ) {
	os_printf ( "Event, got IP\n" );
	show_ip ();
        udp_setup();
    } else if ( event == EVENT_STAMODE_CONNECTED ) {
	os_printf ( "Event, connected\n" );
    } else if ( event == EVENT_STAMODE_DISCONNECTED ) {
	os_printf ( "Event, disconnected\n" );
    } else {
	os_printf ( "Unknown event %d !\n", event );
    }
}

void wifi_init()
{
    struct station_config conf;

    wifi_set_opmode(STATION_MODE);

    os_memset ( &conf, 0, sizeof(struct station_config) );
    os_memcpy (&conf.ssid, ssid, 32);
    os_memcpy (&conf.password, pass, 64 );
    wifi_station_set_config (&conf);

    /* set a callback for wifi events */
    wifi_set_event_handler_cb ( wifi_event );
}