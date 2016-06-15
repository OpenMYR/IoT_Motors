#define USE_US_TIMER

/* Try out timers on the ESP8266
 * get an LED blinking
 * Tom Trebisky  12-27-2015
 */
#include "c_types.h"
#include "gpio_driver.h"
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
	
	os_printf("\n");
	os_printf("SDK version:%s\n", system_get_sdk_version());
	system_print_meminfo();
}

/* THE END */