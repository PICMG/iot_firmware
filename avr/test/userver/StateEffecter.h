//    StateEffecter.h
//
//    This header file declares functions related to the the StateEffecter 
//    "base class".  
//    This code is intended to be used as part of the PICMG reference code 
//    for IoT.
//    
//    More information on the PICMG IoT data model can be found within
//    the PICMG family of IoT specifications.  For more information,
//    please visit the PICMG web site (www.picmg.org)
//
//    Copyright (C) 2021,  PICMG
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
#ifndef STATEEFFECTER_H_INCLUDED
#define STATEEFFECTER_H_INCLUDED
#include "node.h"

typedef struct {
    unsigned char state;             // the current state of the effecter
    unsigned char operationalState;  // the operational state of the sensor
    unsigned char stateWhenHigh;     // the state when the output is high
    unsigned char stateWhenLow;      // the state when the output is low
} StateEffecterInstance;

void          stateeffecter_init(StateEffecterInstance *inst);
unsigned char stateeffecter_getOutput(StateEffecterInstance *inst);
unsigned char stateeffecter_setOperationalState(StateEffecterInstance *inst, unsigned char state);
unsigned char stateeffecter_setPresentState(StateEffecterInstance *inst, unsigned char state);
unsigned char stateeffecter_getOperationalState(StateEffecterInstance *inst);
unsigned char stateeffecter_getPresentState(StateEffecterInstance *inst);

#endif // STATEEFFECTER_H_INCLUDED