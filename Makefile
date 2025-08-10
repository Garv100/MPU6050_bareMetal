MCU=atmega328p
F_CPU=16000000UL
CC=avr-gcc
OBJCOPY=avr-objcopy
CFLAGS=-std=c11 -Wall -Os -DF_CPU=$(F_CPU) -mmcu=$(MCU)

all: main.hex

main.elf: mpu6050_test.c 
	$(CC) $(CFLAGS) -o $@ $^

main.hex: main.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

flash: main.hex
	avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 115200 -U flash:w:main.hex

clean:
	rm -f *.elf *.hex
