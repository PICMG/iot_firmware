//    NumericEffecter.h
//
//    This header file declares functions related to the the NumericEffecter 
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
#ifndef NUMERICEFFECTER_H_INCLUDED
#define NUMERICEFFECTER_H_INCLUDED
#include "node.h"

typedef struct {
    FIXEDPOINT_24_8 value;           // the current value set for the effecter
                                     // this will be linearized and sent to
                                     // the channel
    unsigned char operationalState;  // the operational state of the sensor
    FIXEDPOINT_24_8 maxSettable;     // the maximum settable value
    FIXEDPOINT_24_8 minSettable;     // the minimum settable value
} NumericEffecterInstance;

void            numericeffecter_init(NumericEffecterInstance *inst);
unsigned char   numericeffecter_setValue(NumericEffecterInstance *inst, FIXEDPOINT_24_8 val);
FIXEDPOINT_24_8 numericeffecter_getValue(NumericEffecterInstance *inst);
unsigned char   numericeffecter_setOperationalState(NumericEffecterInstance *inst, unsigned char state);
unsigned char   numericeffecter_getOperationalState(NumericEffecterInstance *inst);

#endif // NUMERICEFFECTER_H_INCLUDED