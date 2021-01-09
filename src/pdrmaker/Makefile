EXECUTABLE := pdrmaker
LIBINCLUDES := ../../lib/include
LIBPATH := ../../lib
INCLUDES := .
OBJECTS := PdrMaker.o $(LIBPATH)/json/libjson.a $(LIBPATH)/pldm/libpldm.a
CXX_FLAGS := -std=c++11 -ggdb

build : $(OBJECTS)
	g++ -o $(EXECUTABLE) $(CXX_FLAGS) $(OBJECTS) -L$(LIBPATH)/json -L$(LIBPATH)/pldm -ljson -lpldm
	cp ../../samples/pldm_definitions.json ./pldm_definitions.json
	cp ../../samples/motion_control.json ./motion_control.json

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
