IoT Motor Firmware
==================
by OpenMYR

[OpenMYR Homepage](http://www.OpenMYR.com/)

Software by Brandon Brown and Kyle Berezin

Hardware design by Chris Adams

Hardware licensed under CERN Hardware License Version 1.2

Software licensed under MIT License

This repository is the source code for firmware designed to drive various MYR motor controllers with an ESP8266-based platform.
	
Reconfiguration of the WiFi mode and softAP login is supported via the built-in web server.

The WiFi handler contains [demonstration code by Tom Trebisky](http://cholla.mmto.org/esp8266/sdk/examples/wifi.html), his blog was a great boon for us when we first dove into coding for the esp.

The hardware timer driver (hw_timer.c) is primarily code by Espressif Systems, located in the driver_lib folder of the examples in the ESP8266 IoT SDK. Used without permission.

The JSON interpreter is [JSMN, by Serge](https://github.com/zserge/jsmn). Used under MIT License.

Currently the implemented drivers support:
* An array of up to 4 180-degree servo motors (testing with board prototype)
* Stepper motors (testing with board prototype)

Further iterations of the firmware will support these products:
* DC motor
* Encoded DC Motor
And many more

Building and Flashing
---------------------
Compiling this project requires the [esp-open-sdk](https://github.com/pfalcon/esp-open-sdk).

* "make flash-quad" cleans, rebuilds and reflashes the quad_servo build
* "make flash-stepper" cleans, rebuilds and reflashes the stepper build
* "make all" builds all versions of the firmware
* "make stepper" builds the stepper build
* "make quad" builds the quad_servo build
* "make clean-stepper" cleans the stepper build
* "make clean-quad" cleans the quad_servo build
* "make clean" cleans all builds


The "make flash" commands are intended to be used on Raspberry Pi 2, as the GPIO port can be used to communicate directly with the ESP8266 without the use of a USB device.
To use the Pi with the ESP8266, disable serial port login and serial bootup info. The ESP8266 can then be used with the /dev/ttyAMA0 port for both burning and serial communications.
For more information, see [this guide](www.extragsm.com/blog/2014/12/03/connect-esp8266-to-raspberry-pi/).
