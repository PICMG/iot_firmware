//*******************************************************************
//    node.h
//
//    This file includes definitions for a simulated PLDM connector node.
//    This node behaves like an extenal embedded device, that communicates
//    with PLDM commands and responses.   This is only intended for use 
//    in PLDM testing.
//
//    Portions of this code are based on the Platform Level Data Model
//    (PLDM) specifications from the Distributed Management Task Force 
//    (DMTF).  More information about PLDM can be found on the DMTF
//    web site (www.dmtf.org).
//
//    More information on the PICMG IoT data model can be found within
//    the PICMG family of IoT specifications.  For more information,
//    please visit the PICMG web site (www.picmg.org)
//
//    Copyright (C) 2020,  PICMG
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#pragma once
#include "mctp.h"
#include "pldm.h"
#include "config.h"
#include "EventGenerator.h"

void node_init();
void node_putCommand(PldmRequestHeader* hdr, unsigned char* command, unsigned int size);
unsigned char* node_getResponse(void);
void node_sendNumericSensorEvent(PldmRequestHeader *rxHeader, unsigned char more, EventGeneratorInstance* egi, unsigned int sensorId, 
                                    unsigned char previousEventState, FIXEDPOINT_24_8 presentReading);
void node_sendStateSensorEvent(PldmRequestHeader *rxHeader, unsigned char more, EventGeneratorInstance* egi, unsigned int sensorId, 
                                    unsigned char previousEventState);
void node_updateEvents();

