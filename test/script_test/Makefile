EXECUTABLE := script_test
LIBINCLUDES := ../../lib/include
LIBPATH := ../../lib
INCLUDES := .
OBJECTS := script_test.o
CXX_FLAGS := -std=c++11 -ggdb

build : $(OBJECTS)  
	g++ -o $(EXECUTABLE) $(CXX_FLAGS) $(OBJECTS)
	
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
