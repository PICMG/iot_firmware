//    EntityStepper1.h
//
//    This header file decleres functions related to the the Stepper1  
//    Logical Entity Type.  
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
#pragma once

 void entityStepper1_init();
 void entityStepper1_readChannels();
 void entityStepper1_writeChannels();

 unsigned char entityStepper1_setStateEffecterStates(PldmRequestHeader* rxHeader);
 unsigned char entityStepper1_setStateEffecterEnables(PldmRequestHeader* rxHeader);
 unsigned char entityStepper1_getStateEffecterStates(PldmRequestHeader* rxHeader, unsigned char *responseBody, unsigned char *size);

 unsigned char entityStepper1_getStateSensorReading(PldmRequestHeader* rxHeader, unsigned char *responseBody, unsigned char *size);
 unsigned char entityStepper1_setStateSensorEnables(PldmRequestHeader* rxHeader);

 unsigned char entityStepper1_getSensorReading(PldmRequestHeader* rxHeader, unsigned char *responseBody, unsigned char *size);
 unsigned char entityStepper1_setNumericSensorEnable(PldmRequestHeader* rxHeader);

 unsigned char entityStepper1_setNumericEffecterValue(PldmRequestHeader* rxHeader);
 unsigned char entityStepper1_getNumericEffecterValue(PldmRequestHeader* rxHeader, unsigned char *responseBody, unsigned char *size);
 unsigned char entityStepper1_setNumericEffecterEnable(PldmRequestHeader* rxHeader);

 void entityStepper1_updateControl();
