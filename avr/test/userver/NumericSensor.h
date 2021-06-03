//    NumericSensor.h
//
//    This header file declares functions related to the the NumericSensor 
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
#ifndef NUMERICSENSOR_H_INCLUDED
#define NUMERICSENSOR_H_INCLUDED
#include "node.h"
#include "EventGenerator.h"

typedef struct {
    FIXEDPOINT_24_8 value;           // the current value read from the channel
    unsigned char operationalState;  // the operational state of the sensor
    unsigned char previousState;     // the state prior to the most recent
    unsigned char eventState;        // the state that generated the event
    EventGeneratorInstance eventGen; 
    FIXEDPOINT_24_8 thresholdFatalHigh;    
    FIXEDPOINT_24_8 thresholdCriticalHigh;
    FIXEDPOINT_24_8 thresholdWarnHigh;
    FIXEDPOINT_24_8 thresholdWarnLow;
    FIXEDPOINT_24_8 thresholdCriticalLow;
    FIXEDPOINT_24_8 thresholdFatalLow;
    FIXEDPOINT_24_8 hysteresisValue;
    unsigned char   thresholdEnables;
    FIXEDPOINT_24_8 valueOffset;    
} NumericSensorInstance;

void            numericsensor_init(NumericSensorInstance *inst);
void            numericsensor_setValue(NumericSensorInstance *inst, FIXEDPOINT_24_8 val);
FIXEDPOINT_24_8 numericsensor_getValue(NumericSensorInstance *inst);
void            numericsensor_updateSensorState(NumericSensorInstance *inst);
void            numericsensor_sensorRearm(NumericSensorInstance *inst);
unsigned char   numericsensor_isTriggered(NumericSensorInstance *inst);
unsigned char   numericsensor_setOperationalState(NumericSensorInstance *inst, unsigned char state, unsigned char eventMessageEnable);
unsigned char   numericsensor_setPresentState(NumericSensorInstance *inst, unsigned char state);
unsigned char   numericsensor_getOperationalState(NumericSensorInstance *inst);
unsigned char   numericsensor_getPresentState(NumericSensorInstance *inst);
unsigned char   numericsensor_getEventState(NumericSensorInstance *inst);
unsigned char   numericsensor_getSensorPreviousState(NumericSensorInstance *inst);
FIXEDPOINT_24_8 numericsensor_getWarningHighThreshold(NumericSensorInstance *inst);
FIXEDPOINT_24_8 numericsensor_getCriticalHighThreshold(NumericSensorInstance *inst);
FIXEDPOINT_24_8 numericsensor_getFatalHighThreshold(NumericSensorInstance *inst);
FIXEDPOINT_24_8 numericsensor_getWarningLowThreshold(NumericSensorInstance *inst);
FIXEDPOINT_24_8 numericsensor_getCriticalLowThreshold(NumericSensorInstance *inst);
FIXEDPOINT_24_8 numericsensor_getFatalLowThreshold(NumericSensorInstance *inst);
unsigned char   numericsensor_setThresholds(NumericSensorInstance *inst,
                    FIXEDPOINT_24_8 fh,FIXEDPOINT_24_8 crh,FIXEDPOINT_24_8 wh,
                    FIXEDPOINT_24_8 wl,FIXEDPOINT_24_8 crl,FIXEDPOINT_24_8 fl);
#endif // NUMERICSENSOR_H_INCLUDED