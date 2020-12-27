LIBFILE := libfcsc.a
LIBINCLUDES := -I. -I../../lib/include -I/usr/lib/avr/include
PROJLIBS := ../../lib
INCLUDES := .
OBJECTS := fcs.o mctp.o uart.o

build : $(OBJECTS)
	;

run : $(OBJECTS)
	;

fcs.o : $(PROJLIBS)/fcs/fcs.c
	avr-gcc -c $< -o $(OUTPUT_DIR)/$@ $(CXX_FLAGS) -I$(INCLUDES) $(LIBINCLUDES)
     
mctp.o : $(PROJLIBS)/mctp/mctp.c
	avr-gcc -c $< -o $(OUTPUT_DIR)/$@ $(CXX_FLAGS) -I$(INCLUDES) $(LIBINCLUDES)

%.o : %.c
	avr-gcc -c $< -o $(OUTPUT_DIR)/$@ -I$(INCLUDES) $(CXX_FLAGS) $(LIBINCLUDES)
	
clean:
	-rm *.o

