EXECUTABLE := mctp_c_test
LIBINCLUDES := ../../lib/include
LIBPATH := ../../lib
INCLUDES := .
OBJECTS := mctp_c_test.o $(LIBPATH)/mctpc/libmctp.a $(LIBPATH)/fcsc/libfcsc.a $(LIBPATH)/uartc/libuartc.a
CXX_FLAGS := -std=c++11 -ggdb

build : $(OBJECTS)  
	g++ -o $(EXECUTABLE) $(CXX_FLAGS) $(OBJECTS) -L$(LIBPATH)/mctpc -L$(LIBPATH)/fcsc -L$(LIBPATH)/uartc -lfcsc -luartc -lmctp
	
%.o : %.cpp
	g++ $(CXX_FLAGS) -c $< -I$(INCLUDES) -I$(LIBINCLUDES)

%.o : %.c
	g++ $(CXX_FLAGS) -c $< -I$(INCLUDES) -I$(LIBINCLUDES)

%.a : 
	cd $(@D) && make clean
	cd $(@D) && make build

clean:
	-rm *.json
	-rm *.o
