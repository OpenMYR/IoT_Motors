#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#define ICACHE_FLASH

#include "c_types.h"

int eio_read ( int gpio );
void eio_high ( int gpio );
void eio_low ( int gpio );
void ICACHE_FLASH_ATTR eio_setup ( int gpio );
void eio_quad_shift(unsigned int input);
void eio_quad_enable();

#endif