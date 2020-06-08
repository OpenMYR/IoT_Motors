# IoT_Motor Firmware 
by [OpenMYR](http://www.OpenMYR.com/) 

OpenMYR WiFi motors can be controled via WiFi, or programed with an Arduino sketch. At the core of the system is an Espressif ESP8266.

This project utilizes PlatformIO with Visual Studio Code

Use with [ESP12-Hobby-Servo (ESP12E.3)](https://github.com/OpenMYR/ESP12E.3), [ESP12-Stepper (ESP12E.6))](https://github.com/OpenMYR/ESP12E.6), or with your own ESP8266 boards.

***

## Driver Support
* An array of up to 4 180-degree servo motors
* Stepper motor

***

## Installation 

1. [Install PlatformIO and Visual Studio Code](https://platformio.org/install/ide?install=vscode)
2. [Clone our repository](https://help.github.com/en/github/creating-cloning-and-archiving-repositories/cloning-a-repository)
3. Open local project folder in Visual Studio Code
4. Using the [PlatformIO Toolbar](https://docs.platformio.org/en/latest/integration/ide/vscode.html#platformio-toolbar) build all environments with the "PlatformIO: Build" command
5. Upload the code and file system using Serial or OTA  
    Upload both code and file system with new devices to ensure proper operation 
