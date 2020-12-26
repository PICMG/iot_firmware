EXECUTABLE := uart_test.elf
LIBINCLUDES := -L../../../lib/include -L/usr/lib/avr/include
LIBPATH := ../../../lib /usr/lib/avr
INCLUDES := -I. -I../../../lib/include -I/usr/lib/avr/include
OBJECTS := main.o uart.o simulavr_info.o
CXX_FLAGS := -g -O0 -Wall -mmcu=atmega328p -DF_CPU=16000000UL

build : $(OBJECTS)  
	avr-g++ -o $(EXECUTABLE) $(CXX_FLAGS) $(OBJECTS) -mmcu=atmega328p -Wl,-Map=solution.map,--cref
	
%.o : %.c
	avr-gcc $(CXX_FLAGS) -c $< $(INCLUDES) $(LIBINCLUDES)

clean:
	-rm *.o
	-rm *.elf
