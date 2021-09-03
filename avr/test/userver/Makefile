#*******************************************************************
#    MAKEFILE
#
#    This file recursively builds the atmega code for the PICMG
#    PLDM reference code.
#
#    Copyright (C) 2020,  PICMG
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https:#www.gnu.org/licenses/>.
#
EXECUTABLE  := userver.elf
HEXFILE     := userver.hex
LIBINCLUDES := -L/usr/lib/avr/include 
LIBPATH     := /usr/lib/avr
INCLUDES    := -I.  
OBJECTS     := main.o simulavr_info.o node.o config.o vprofiler.o systemtimer.o stepdir_out.o interpolator.o channels.o adc.o entityStepper1.o entitySimple1.o NumericEffecter.o StateEffecter.o StateSensor.o NumericSensor.o EventGenerator.o mctp.o uart.o crc8.o fcs.o
CXX_FLAGS   := -Wall -mmcu=atmega328p -DF_CPU=16000000UL
OUTPUT_DIR  := $(CURDIR)
UUID_BYTES := $(shell ./getuuid.sh)

export      CXX_FLAGS
export      OUTPUT_DIR

# clean and build the project
build: CXX_FLAGS += -g -O0 
build: clean $(OBJECTS)
	avr-g++ -o $(EXECUTABLE) $(CXX_FLAGS) $(OBJECTS) -Wl,-Map=solution.map,--cref
	
# clean, build and run the project on atmega 328p hardware
run_builder: CXX_FLAGS += -O2
run_builder: clean cfg_builder $(OBJECTS)
	avr-g++ -o $(EXECUTABLE) $(CXX_FLAGS) $(OBJECTS) -Wl,-Map=solution.map,--cref
	avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex $(EXECUTABLE) $(HEXFILE)

# clean, build and run the project on atmega 328p hardware
run_simple: CXX_FLAGS += -O2
run_simple: clean cfg_simple $(OBJECTS)
	avr-g++ -o $(EXECUTABLE) $(CXX_FLAGS) $(OBJECTS) -Wl,-Map=solution.map,--cref
	avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex $(EXECUTABLE) $(HEXFILE)
	avrdude -p m328p -c Arduino -P COM13 -U flash:w:$(HEXFILE)

# clean, build and run the project on atmega 328p hardware
run_stepper: CXX_FLAGS += -Os
run_stepper: clean cfg_stepper $(OBJECTS)
	avr-g++ -o $(EXECUTABLE) $(CXX_FLAGS) $(OBJECTS) -Wl,-Map=solution.map,--cref
	avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex $(EXECUTABLE) $(HEXFILE)
	avrdude -p m328p -c Arduino -P COM13 -U flash:w:$(HEXFILE)

# build non-library object files and place them in this folder
%.o : %.c
	avr-gcc $(CXX_FLAGS) -DUUID=$(UUID_BYTES) -c $< $(INCLUDES) $(LIBINCLUDES)

# clean this folder and the library folder of any object files that currently exist
clean:
	-rm *.o
	-rm *.elf
	-rm *.hex

cfg_simple:
	cp ./configurations/pdrdata_simple.c config.c
	cp ./configurations/pdrdata_simple.h config.h

cfg_stepper:
	cp ./configurations/pdrdata_stepper.c config.c
	cp ./configurations/pdrdata_stepper.h config.h

cfg_builder:
	cp  ../../../../iot_builder/src/builder/config.c config.c
	cp  ../../../../iot_builder/src/builder/config.h config.h
