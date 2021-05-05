LIBFILE := libfcsc.a
LIBINCLUDES := -I. -I./include -I/usr/lib/avr/include
INCLUDES := .
OBJECTS := fcs.o mctp.o uart.o crc8.o

build : $(OBJECTS)
	;

run : $(OBJECTS)
	;

%.o : %.c
	avr-gcc -c $< -o $(OUTPUT_DIR)/$@ -I$(INCLUDES) $(CXX_FLAGS) $(LIBINCLUDES)
	
clean:
	-rm *.o

