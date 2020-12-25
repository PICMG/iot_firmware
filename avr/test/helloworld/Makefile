EXECUTABLE := hello_avr.elf
LIBINCLUDES := ../../lib/include
LIBPATH := ../../lib
INCLUDES := .
OBJECTS := main.o
CXX_FLAGS := -Wall -mmcu=atmega328p -DF_CPU=16000000UL

build : $(OBJECTS)  
	avr-g++ -o $(EXECUTABLE) $(CXX_FLAGS) $(OBJECTS) -mmcu=atmega328p -Wl,-Map=solution.map,--cref libserial.a
	
%.o : %.c
	avr-gcc $(CXX_FLAGS) -c $< -I$(INCLUDES) -I$(LIBINCLUDES)

clean:
	-rm *.o
	-rm *.elf
