//    StateSensor.h
//
//    This header file declares functions related to the the StateSensor 
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
#ifndef STATESENSOR_H_INCLUDED
#define STATESENSOR_H_INCLUDED
#include "node.h"
#include "EventGenerator.h"

typedef struct {
    unsigned char value;             // the current raw value read from the channel
    unsigned char operationalState;  // the operational state of the sensor
    unsigned char previousState;     // the state prior to the most recent
    unsigned char eventState;        // the state that generated the event
    unsigned char stateWhenHigh;     // the state when the input is high
    unsigned char stateWhenLow;      // the state when the input is low
    EventGeneratorInstance eventGen; 
} StateSensorInstance;

void          statesensor_init(StateSensorInstance *inst);
void          statesensor_setValue(StateSensorInstance *inst, unsigned char val);
void          statesensor_setValueFromChannelBit(StateSensorInstance *inst, unsigned char bit);
void          statesensor_updateSensorState(StateSensorInstance *inst);
unsigned char statesensor_setEnables(StateSensorInstance *inst, unsigned char enable);
unsigned char statesensor_isEnabled(StateSensorInstance *inst);
void          statesensor_sensorRearm(StateSensorInstance *inst);
unsigned char statesensor_isTriggered(StateSensorInstance *inst);
unsigned char statesensor_setOperationalState(StateSensorInstance *inst, unsigned char state, unsigned char eventMessageEnable);
unsigned char statesensor_setPresentState(StateSensorInstance *inst, unsigned char state);
unsigned char statesensor_getOperationalState(StateSensorInstance *inst);
unsigned char statesensor_getPresentState(StateSensorInstance *inst);
unsigned char statesensor_getEventState(StateSensorInstance *inst);
unsigned char statesensor_getSensorPreviousState(StateSensorInstance *inst);

#endif // STATESENSOR_H_INCLUDED