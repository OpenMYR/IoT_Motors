INCLUDE_FOLDER = ./include

CC = xtensa-lx106-elf-gcc
CFLAGS = -I$(INCLUDE_FOLDER) -mlongcalls
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -Wl,--end-group -lgcc
LDFLAGS = -Teagle.app.v6.ld

OBJECTS = main.o hw_timer.o command_layer.o udp.o tcp.o wifi.o jsmn.o
STEPPER_OUTPUT = stepper_driver stepper_driver.o stepper_driver-0x00000.bin stepper_driver-0x40000.bin op_queue.o
SERVO_OUTPUT = servo_driver servo_driver.o servo_driver-0x00000.bin servo_driver-0x40000.bin op_queue.o gpio_driver.o
QUAD_SERVO_OUTPUT = quad_servo_driver quad_servo_driver.o quad_servo_driver-0x00000.bin quad_servo_driver-0x40000.bin quad_op_queue.o
BRUSH_MOTOR_OUTPUT = brushed_motor_driver brushed_motor_driver.o brushed_motor_driver-0x00000.bin brushed_motor_driver-0x40000.bin op_queue.o

all: servo stepper quad brush

servo: servo_driver-0x00000.bin

stepper: stepper_driver-0x00000.bin

quad: quad_servo_driver-0x00000.bin

brush: brushed_motor_driver-0x00000.bin

quad_servo_driver-0x00000.bin: quad_servo_driver
	esptool.py elf2image $^

servo_driver-0x00000.bin: servo_driver
	esptool.py elf2image $^
	
stepper_driver-0x00000.bin: stepper_driver
	esptool.py elf2image $^

brushed_motor_driver-0x00000.bin: brushed_motor_driver
	esptool.py elf2image $^
	
stepper_driver: $(OBJECTS) stepper_driver.o op_queue.o

servo_driver: $(OBJECTS) servo_driver.o op_queue.o gpio_driver.o

quad_servo_driver: $(OBJECTS) quad_servo_driver.o quad_op_queue.o

brushed_motor_driver: $(OBJECTS) brushed_motor_driver.o op_queue.o

main.o: main.c
hw_timer.o: hw_timer.c $(INCLUDE_FOLDER)/hw_timer.h
gpio_driver.o: gpio_driver.c $(INCLUDE_FOLDER)/gpio_driver.h
command_layer.o: command_layer.c $(INCLUDE_FOLDER)/command_layer.h
servo_driver.o: servo_driver.c $(INCLUDE_FOLDER)/motor_driver.h
stepper_driver.o: stepper_driver.c $(INCLUDE_FOLDER)/motor_driver.h
quad_servo_driver.o: quad_servo_driver.c $(INCLUDE_FOLDER)/motor_driver.h
brushed_motor_driver.o: brushed_motor_driver.c $(INCLUDE_FOLDER)/motor_driver.h
udp.o: udp.c $(INCLUDE_FOLDER)/udp.h
tcp.o: tcp.c $(INCLUDE_FOLDER)/tcp.h $(INCLUDE_FOLDER)/web_data.h
wifi.o: wifi.c $(INCLUDE_FOLDER)/wifi.h
op_queue.o: op_queue.c $(INCLUDE_FOLDER)/op_queue.h
quad_op_queue.o: quad_op_queue.c $(INCLUDE_FOLDER)/op_queue.h
jsmn.o: jsmn.c

flash-servo: clean-servo servo_driver-0x00000.bin
	esptool.py --port /dev/ttyAMA0 write_flash 0 servo_driver-0x00000.bin 0x40000 servo_driver-0x40000.bin
	
flash-stepper: clean-stepper stepper_driver-0x00000.bin
	esptool.py --port /dev/ttyAMA0 write_flash 0 stepper_driver-0x00000.bin 0x40000 stepper_driver-0x40000.bin

flash-quad: clean-quad quad_servo_driver-0x00000.bin
	esptool.py --port /dev/ttyAMA0 write_flash 0 quad_servo_driver-0x00000.bin 0x40000 quad_servo_driver-0x40000.bin

flash-brush: clean-brushed brushed_motor_driver-0x00000.bin
	esptool.py --port /dev/ttyAMA0 write_flash 0 brushed_motor_driver-0x00000.bin 0x40000 brushed_motor_driver-0x40000.bin

clean:
	rm -f $(OBJECTS) $(SERVO_OUTPUT) $(STEPPER_OUTPUT) $(QUAD_SERVO_OUTPUT) $(BRUSH_MOTOR_OUTPUT)
	
clean-stepper:
	rm -f $(OBJECTS) $(STEPPER_OUTPUT)

clean-servo:
	rm -f $(OBJECTS) $(SERVO_OUTPUT) 
	
clean-quad:
	rm -f $(OBJECTS) $(QUAD_SERVO_OUTPUT)

clean-brush:
	rm -f $(OBJECTS) $(BRUSH_MOTOR_OUTPUT)