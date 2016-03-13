IoT Motor Firmware
by OpenMYR
Software by Brandon Brown and Kyle Berezin
Hardware design by Chris Adams
Hardware licensed under CERN Hardware License Version 1.2
Software licensed under MIT License

This repository is the source code for firmware designed to drive various MYR motor controllers with an ESP8266-based platform.
	
Reconfiguration of the WiFi mode and login is not yet supported; the SSID and password of the network to connect to is baked into the firmware via user_config.h.

The GPIO and WiFi handlers contain demonstration code by Tom Trebisky, used without permission. This code will be replaced by the time the project goes public.
Tom's original GPIO code: http://cholla.mmto.org/esp8266/sdk/blink_any.c
Tom's original wifi code: http://cholla.mmto.org/esp8266/sdk/wifi.c

The hardware timer driver (hw_timer.c) is primarily code by Espressif Systems, located in the driver_lib folder of the examples in the ESP8266 IoT SDK. Used without permission.

Currently the implemented drivers support:
-180 degree servo motors (tested on breadboard, no board prototype yet)
-Stepper motors (some testing with board prototype)

Further iterations of the firmware will support these products:
-DC motor
-Encoded DC Motor
And many more

Compiling this project requires the esp-open-sdk, which is available here: https://github.com/pfalcon/esp-open-sdk

The native development environment is a Raspberry Pi 2, as the GPIO port can be used to communicate directly with the ESP8266 without the use of a USB device.
To use the Pi with the ESP8266, disable serial port login and serial bootup info. The ESP8266 can then be used with the /dev/ttyAMA0 port for both burning and serial communications.
For more information, see this guide: www.extragsm.com/blog/2014/12/03/connect-esp8266-to-raspberry-pi/

The makefile provided is designed to flash to the ESP via the Raspberry Pi.
"make flash-servo" cleans, rebuilds and reflashes the servo build
"make flash-stepper" cleans, rebuilds and reflashes the stepper build
"make all" builds all versions of the firmware
"make stepper" builds the stepper build
"make servo" builds the servo build
"make clean-stepper" cleans the stepper build
"make clean-servo" cleans the servo build
"make clean" cleans all builds

