# Makefile for AVR-GCC project

# Microcontroller (change as needed)
MCU = atmega328p

# Clock frequency
F_CPU = 16000000UL

# Compiler
CC = avr-gcc

# Compiler flags
CFLAGS = -Os -DF_CPU=$(F_CPU) -mmcu=$(MCU)

# Source files
SRC = eggtimer.c 

# Object files
OBJ = $(SRC:.c=.o)

# Output hex file
TARGET = eggtimer.hex

# avrdude settings
AVRDUDE = avrdude
AVRDUDE_PROGRAMMER = arduino
AVRDUDE_PORT = /dev/ttyACM0  # Adjust this to your Arduino's port
AVRDUDE_BAUDRATE = 115200

# avrdude command
AVRDUDE_CMD = sudo $(AVRDUDE) -c $(AVRDUDE_PROGRAMMER) -p $(MCU) -P $(AVRDUDE_PORT) -b $(AVRDUDE_BAUDRATE) -U flash:w:$(TARGET):a

# Build target
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJ) $(TARGET)

# Upload to AVR target
upload: $(TARGET)
	$(AVRDUDE_CMD)

# Phony targets
.PHONY: all clean upload