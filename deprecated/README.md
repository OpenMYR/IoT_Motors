IoT Motor Firmware
==================
by [OpenMYR](http://www.OpenMYR.com/)

Source code for firmware designed to drive various MYR motor controllers with an ESP8266-based platform.

 * Software by Brandon Brown and Kyle Berezin (MIT License)
 * Hardware design by Chris Adams (CERN Hardware License Version 1.2)

### Features
	
Reconfiguration of the WiFi mode and softAP login is supported via the built-in web server.

### Borrows From

 * WiFi handler contains [demonstration code by Tom Trebisky](http://www.openmyr.com/blog/2016/06/esp8266-output-gpio-and-optimization/)
 * Hardware timer driver ([`hw_timer.c`](https://github.com/espressif/ESP8266_RTOS_SDK/blob/master/examples/driver_lib/driver/hw_timer.c)) (MIT License)
 * JSON interpreter [JSMN, by Serge](https://github.com/zserge/jsmn) (MIT License)

### Driver Support
* An array of up to 4 180-degree servo motors
* Stepper motors

### Future Support
* DC motor
* Encoded DC Motor
* ... and many more

### Building and Flashing
Compiling this project requires the [esp-open-sdk](https://github.com/pfalcon/esp-open-sdk), as well as [SCons](http://scons.org).

* `scons flash-quad` flashes the WiFi Hobby Servo Controller firmware, rebuilding if necessary
* `scons flash-stepper` flashes the WiFi Stepper Motor firmware, rebuilding if necessary
* `scons` builds both versions of the firmware
* `scons quad` builds the WiFi Hobby Servo Controller firmware
* `scons stepper` builds the WiFi Stepper Motor firmware
* `scons -c` cleans all builds
* `scons -c quad` cleans the WiFi Hobby Servo Controller build
* `scons -c stepper` cleans the WiFi Stepper Motor build

The "flash-quad" and "flash-servo" commands are intended to be used on Raspberry Pi 2, as the GPIO port can be used to communicate directly with the ESP8266 without the use of a USB device.
To use the Pi with the ESP8266, disable serial port login and serial bootup info. The ESP8266 can then be used with the /dev/ttyAMA0 port for both burning and serial communications.
For more information, see [this guide](http://www.extragsm.com/blog/2014/12/03/connect-esp8266-to-raspberry-pi/).
