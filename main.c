#define USE_US_TIMER

/* Try out timers on the ESP8266
 * get an LED blinking
 * Tom Trebisky  12-27-2015
 */
#include "c_types.h"
#include "gpio_driver.h"
#include "wifi.h"
#include "osapi.h"
#include "user_config.h"
#include "user_interface.h"

void user_init()
{
    uart_div_modify(0, UART_CLK_FREQ / 115200);
	initialize_command_layer();
    // And this is used to print some information
    //os_printf("\n");
    //os_printf("SDK version:%s\n", system_get_sdk_version());
	
    wifi_init();
	
    os_printf("\n");
    os_printf("SDK version:%s\n", system_get_sdk_version());
    system_print_meminfo();
	
    eio_setup ( GPIO_STEP );
    eio_setup ( 0);
    eio_setup ( 14 );
    eio_setup ( 12 );
    eio_setup ( GPIO_STEP_DIR);
	eio_setup(GPIO_STEP_ENABLE);
	os_printf ( "Ready 1\n");

    eio_low ( GPIO_STEP );
	eio_low( 0 );
    eio_low ( 14 );
    eio_low ( 12 );
    eio_low ( GPIO_STEP_DIR);
	eio_low(GPIO_STEP_ENABLE);
	os_printf ( "Ready 2\n");
	
    //hw_timer_init(FRC1_SOURCE, 1);
    //hw_timer_set_func(step_driver);
    //hw_timer_arm(RESOLUTION_US);

}

/* THE END */