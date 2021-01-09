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
LIBINCLUDES := -L/usr/lib/avr/include -L../../../lib/include 
LIBPATH     := ../../../lib /usr/lib/avr
AVRPROJLIBS := ../../lib
INCLUDES    := -I. -I/usr/lib/avr/include -I../../../lib/include 
OBJECTS     := main.o simulavr_info.o node.o pdr_data.o vprofiler.o control_servo.o timer1.o
LIBOBJECTS  := uart.o fcs.o mctp.o crc8.o
CXX_FLAGS   := -Wall -mmcu=atmega328p -DF_CPU=16000000UL
OUTPUT_DIR  := $(CURDIR)
export      CXX_FLAGS
export      OUTPUT_DIR

# clean and build the project
build: CXX_FLAGS += -g -O0 
build: clean $(OBJECTS) $(LIBOBJECTS)
	avr-g++ -o $(EXECUTABLE) $(CXX_FLAGS) $(OBJECTS) $(LIBOBJECTS) -Wl,-Map=solution.map,--cref
	
# clean, build and run the project on atmega 328p hardware
run: CXX_FLAGS += -Os
run: clean $(OBJECTS) $(LIBOBJECTS)
	avr-g++ -o $(EXECUTABLE) $(CXX_FLAGS) $(OBJECTS) $(LIBOBJECTS) -Wl,-Map=solution.map,--cref
	avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex $(EXECUTABLE) $(HEXFILE)
	avrdude -p m328p -c Arduino -P /dev/ttyUSB0 -U flash:w:$(HEXFILE)

# build library object files and place them in this folder
$(LIBOBJECTS):
	$(MAKE) -w -C $(AVRPROJLIBS) $@

# build non-library object files and place them in this folder
%.o : %.c
	avr-gcc $(CXX_FLAGS) -c $< $(INCLUDES) $(LIBINCLUDES)

# clean this folder and the library folder of any object files that currently exist
clean:
	$(MAKE) -C $(AVRPROJLIBS) clean
	-rm *.o
	-rm *.elf
	-rm *.hex
