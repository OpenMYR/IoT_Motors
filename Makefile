CC = xtensa-lx106-elf-gcc
CFLAGS = -I. -mlongcalls
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -Wl,--end-group -lgcc
LDFLAGS = -Teagle.app.v6.ld

OBJECTS = main.o hw_timer.o gpio_driver.o command_layer.o udp.o wifi.o op_queue.o
STEPPER_OUTPUT = stepper_driver stepper_driver.o stepper_driver-0x00000.bin stepper_driver-0x40000.bin
SERVO_OUTPUT = servo_driver servo_driver.o servo_driver-0x00000.bin servo_driver-0x40000.bin

all: servo stepper

servo: servo_driver-0x00000.bin

stepper: stepper_driver-0x00000.bin

servo_driver-0x00000.bin: servo_driver
	esptool.py elf2image $^
	
stepper_driver-0x00000.bin: stepper_driver
	esptool.py elf2image $^
	
stepper_driver: $(OBJECTS) stepper_driver.o

servo_driver: $(OBJECTS) servo_driver.o 

main.o: main.c
hw_timer.o: hw_timer.c
gpio_driver.o: gpio_driver.c
command_layer.o: command_layer.c
servo_driver.o: servo_driver.c
stepper_driver.o: stepper_driver.c
udp.o: udp.c
wifi.o: wifi.c
op_queue.o: op_queue.c

flash-servo: clean-servo servo_driver-0x00000.bin
	esptool.py --port /dev/ttyAMA0 write_flash 0 servo_driver-0x00000.bin 0x40000 servo_driver-0x40000.bin
	
flash-stepper: clean-stepper stepper_driver-0x00000.bin
	esptool.py --port /dev/ttyAMA0 write_flash 0 stepper_driver-0x00000.bin 0x40000 stepper_driver-0x40000.bin

clean:
	rm -f $(OBJECTS) $(SERVO_OUTPUT) $(STEPPER_OUTPUT)
	
clean-stepper:
	rm -f $(OBJECTS) $(STEPPER_OUTPUT)

clean-servo:
	rm -f $(OBJECTS) $(SERVO_OUTPUT) 