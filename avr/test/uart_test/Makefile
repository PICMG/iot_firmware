EXECUTABLE := uart_test.elf
HEXFILE := uart_test.hex
LIBINCLUDES := -L../../../lib/include -L/usr/lib/avr/include
LIBPATH := ../../../lib /usr/lib/avr
INCLUDES := -I. -I../../../lib/include -I/usr/lib/avr/include
OBJECTS := main.o uart.o simulavr_info.o fcs.o mctp.o
CXX_FLAGS := -Wall -mmcu=atmega328p -DF_CPU=16000000UL

build: CXX_FLAGS += -g -O0 
build: $(OBJECTS)  
	avr-g++ -o $(EXECUTABLE) $(CXX_FLAGS) $(OBJECTS) -mmcu=atmega328p -Wl,-Map=solution.map,--cref
	
%.o : %.c
	avr-gcc $(CXX_FLAGS) -c $< $(INCLUDES) $(LIBINCLUDES)

run: CXX_FLAGS += -O3
run: $(OBJECTS)
	avr-g++ -o $(EXECUTABLE) $(CXX_FLAGS) $(OBJECTS) -mmcu=atmega328p -Wl,-Map=solution.map,--cref
	avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex $(EXECUTABLE) $(HEXFILE)
	avrdude -p m328p -c Arduino -P /dev/ttyUSB0 -U flash:w:$(HEXFILE)

clean:
	-rm *.o
	-rm *.elf
