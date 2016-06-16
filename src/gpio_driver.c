/* Portions of this code is demo code by
 * Tom Trebisky 
 * http://cholla.mmto.org/esp8266/sdk/blink_any.c
 * 
 * New code will replace this in public release
 *
 */
 
#define ICACHE_FLASH

#include "c_types.h"
#include "gpio_driver.h"
#include "eagle_soc.h"
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "gpio.h"

/* place holder for unused channels */
#define Z	0

/* 16 possible GPIO
 * see eagle_soc.h
 * Also NodeMCU:platform/pin_map.c
 */
static const int bits[] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80,
	0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000 };

static const int mux[] = {
    PERIPHS_IO_MUX_GPIO0_U,	/* 0 - D3 */
    PERIPHS_IO_MUX_U0TXD_U,	/* 1 - uart */
    PERIPHS_IO_MUX_GPIO2_U,	/* 2 - D4 */
    PERIPHS_IO_MUX_U0RXD_U,	/* 3 - uart */
    PERIPHS_IO_MUX_GPIO4_U,	/* 4 - D2 */
    PERIPHS_IO_MUX_GPIO5_U,	/* 5 - D1 */
    Z,	/* 6 */
    Z,	/* 7 */
    Z,	/* 8 */
    PERIPHS_IO_MUX_SD_DATA2_U,	/* 9   - D11 (SD2) */
    PERIPHS_IO_MUX_SD_DATA3_U,	/* 10  - D12 (SD3) */
    Z,	/* 11 */
    PERIPHS_IO_MUX_MTDI_U,	/* 12 - D6 */
    PERIPHS_IO_MUX_MTCK_U,	/* 13 - D7 */
    PERIPHS_IO_MUX_MTMS_U,	/* 14 - D5 */
    PERIPHS_IO_MUX_MTDO_U	/* 15 - D8 */
};

static const int func[] = { 0, 3, 0, 3,   0, 0, Z, Z,   Z, 3, 3, Z,   3, 3, 3, 3 };

/* These are for one reason or other, ill advised.
 *  1 and 3 are used for the uart
 *  6, 7, 8, and 11 are unknown mysteries thus far
 *  9 and 10 hang the unit yielding a wdt reset
 */
static const int evil[] = { 0, 1, 0, 1,   0, 0, 1, 1,   1, 1, 1, 1,   0, 0, 0, 0 };

int
eio_read ( int gpio )
{
	return GPIO_REG_READ ( GPIO_OUT_ADDRESS ) & (1<<gpio);
}

void
eio_high ( int gpio )
{
	register int mask = bits[gpio];

	gpio_output_set(mask, 0, mask, 0);
}

void
eio_low ( int gpio )
{
	register int mask = bits[gpio];

	gpio_output_set ( 0, mask, mask, 0);
}

void ICACHE_FLASH_ATTR
eio_setup ( int gpio )
{
	if ( evil[gpio] ) {
	    os_printf ( "!!! Don't try to use gpio %d\n", gpio );
	    return;
	}

	PIN_FUNC_SELECT ( mux[gpio], func[gpio] );
}

void eio_quad_shift(unsigned int input)
{
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, input);
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, ~input);
    //os_printf("Set %04x Clear %04x Register %04x\n", input, ~input, GPIO_REG_READ(GPIO_OUT_ADDRESS));
}

void eio_quad_enable()
{
    GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, 0x7010);
}