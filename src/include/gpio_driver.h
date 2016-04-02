#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

int eio_read ( int gpio );
void eio_high ( int gpio );
void eio_low ( int gpio );
void eio_setup ( int gpio );

#endif