LIBFILE := libfcsc.a
LIBINCLUDES := ../include
INCLUDES := .
OBJECTS := fcs.o

build : $(OBJECTS)
	ar -rc $(LIBFILE) $(OBJECTS)

%.o : %.cpp
	g++ -ggdb -c $< -I$(INCLUDES) -I$(LIBINCLUDES)

%.o : %.c
	g++ -ggdb -c $< -I$(INCLUDES) -I$(LIBINCLUDES)

clean:
	-rm *.o
	-rm *.a

