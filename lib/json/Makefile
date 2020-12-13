LIBFILE := libjson.a
LIBINCLUDES := ../include
INCLUDES := .
OBJECTS := JsonArray.o JsonFactory.o JsonObject.o JsonValue.o

build : $(OBJECTS)
	ar -rc $(LIBFILE) $(OBJECTS)

%.o : %.cpp
	g++ -ggdb -c $< -I$(INCLUDES) -I$(LIBINCLUDES)

%.o : %.c
	g++ -ggdb -c $< -I$(INCLUDES) -I$(LIBINCLUDES)

clean:
	-rm *.o
	-rm *.a

