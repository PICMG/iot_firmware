//*******************************************************************
//    pdrdata.h
//
//    This is an example header file that can be used with code emitted
//    from pdrmaker. 
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
#include <avr/pgmspace.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLE_RATE 4000
typedef signed long FIXEDPOINT_24_8;

#define PDR_BYTE_TYPE const unsigned char
#define FRU_BYTE_TYPE const unsigned char
#define LINTABLE_TYPE const signed int
#define PDR_DATA_ATTRIBUTES PROGMEM
#define FRU_DATA_ATTRIBUTES PROGMEM
#define LINTABLE_DATA_ATTRIBUTES PROGMEM

//extern PDR_BYTE_TYPE __pdr_data[] PDR_DATA_ATTRIBUTES;
//extern unsigned int __pdr_total_size;
//extern unsigned int __pdr_number_of_records;
//extern unsigned int __pdr_max_record_size;

#ifdef __cplusplus
}
#endif

//============================================================================
//====================
// Module-Related Macros
#define ATMEGA328PB

//====================
// PDR-Related Macros
extern PDR_BYTE_TYPE __pdr_data[] PDR_DATA_ATTRIBUTES;
#define PDR_TOTAL_SIZE 953
#define PDR_NUMBER_OF_RECORDS 20
#define PDR_MAX_RECORD_SIZE 174

//====================
// FRU-Related Macros
extern FRU_BYTE_TYPE __fru_data[] FRU_DATA_ATTRIBUTES;
#define FRU_TABLE_MAXIMUM_SIZE 0
#define FRU_TOTAL_SIZE 43
#define FRU_TOTAL_RECORD_SETS 1
#define FRU_NUMBER_OF_RECORDS 2
#define FRU_MAX_RECORD_SIZE 29

//====================
// Channel-Related Macros
#define CHANNEL_INTERLOCK_OUT
#define CHANNEL_INTERLOCK_IN
#define CHANNEL_TRIGGER_OUT
#define CHANNEL_TRIGGER_IN
#define CHANNEL_DIGITAL_IN1
#define CHANNEL_DIGITAL_IN2
#define CHANNEL_STEP_DIR_OUT1

//====================
// Logical Entity-Related Macros
#define ENTITY_STEPPER1
#define ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER
#define ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_BINDINGTYPE_STATEEFFECTER
#define ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_EFFECTERID 1
#define ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_BOUNDCHANNEL interlock_out
#define ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_STATEWHENHIGH 2
#define ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_STATEWHENLOW 1
#define ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_DEFAULTSTATE 1
#define ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR
#define ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_BINDINGTYPE_STATESENSOR
#define ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_SENSORID 1
#define ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_BOUNDCHANNEL interlock_in
#define ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_STATEWHENHIGH 2
#define ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_STATEWHENLOW 1
#define ENTITY_STEPPER1_TRIGGEREFFECTER
#define ENTITY_STEPPER1_TRIGGEREFFECTER_BINDINGTYPE_STATEEFFECTER
#define ENTITY_STEPPER1_TRIGGEREFFECTER_EFFECTERID 2
#define ENTITY_STEPPER1_TRIGGEREFFECTER_BOUNDCHANNEL trigger_out
#define ENTITY_STEPPER1_TRIGGEREFFECTER_STATEWHENHIGH 2
#define ENTITY_STEPPER1_TRIGGEREFFECTER_STATEWHENLOW 1
#define ENTITY_STEPPER1_TRIGGEREFFECTER_DEFAULTSTATE 1
#define ENTITY_STEPPER1_TRIGGERSENSOR
#define ENTITY_STEPPER1_TRIGGERSENSOR_BINDINGTYPE_STATESENSOR
#define ENTITY_STEPPER1_TRIGGERSENSOR_SENSORID 2
#define ENTITY_STEPPER1_TRIGGERSENSOR_BOUNDCHANNEL trigger_in
#define ENTITY_STEPPER1_TRIGGERSENSOR_STATEWHENHIGH 2
#define ENTITY_STEPPER1_TRIGGERSENSOR_STATEWHENLOW 1
#define ENTITY_STEPPER1_POSITION
#define ENTITY_STEPPER1_POSITION_BINDINGTYPE_NUMERICSENSOR
#define ENTITY_STEPPER1_POSITION_SENSORID 7
#define ENTITY_STEPPER1_POSITION_NORMALMIN 0
#define ENTITY_STEPPER1_POSITION_NORMALMAX 0
#define ENTITY_STEPPER1_POSITION_UPPERTHRESHOLDWARNING 4000000
#define ENTITY_STEPPER1_POSITION_UPPERTHRESHOLDCRITICAL 0
#define ENTITY_STEPPER1_POSITION_UPPERTHRESHOLDFATAL 8000000
#define ENTITY_STEPPER1_POSITION_LOWERTHRESHOLDWARNING -4000000
#define ENTITY_STEPPER1_POSITION_LOWERTHRESHOLDCRITICAL 0
#define ENTITY_STEPPER1_POSITION_LOWERTHRESHOLDFATAL -8000000
#define ENTITY_STEPPER1_POSITION_ENABLEDTHRESHOLDS 96
#define ENTITY_STEPPER1_POSITIVELIMIT
#define ENTITY_STEPPER1_POSITIVELIMIT_BINDINGTYPE_STATESENSOR
#define ENTITY_STEPPER1_POSITIVELIMIT_SENSORID 8
#define ENTITY_STEPPER1_POSITIVELIMIT_BOUNDCHANNEL digital_in1
#define ENTITY_STEPPER1_POSITIVELIMIT_STATEWHENHIGH 2
#define ENTITY_STEPPER1_POSITIVELIMIT_STATEWHENLOW 1
#define ENTITY_STEPPER1_NEGATIVELIMIT
#define ENTITY_STEPPER1_NEGATIVELIMIT_BINDINGTYPE_STATESENSOR
#define ENTITY_STEPPER1_NEGATIVELIMIT_SENSORID 9
#define ENTITY_STEPPER1_NEGATIVELIMIT_BOUNDCHANNEL digital_in2
#define ENTITY_STEPPER1_NEGATIVELIMIT_STATEWHENHIGH 2
#define ENTITY_STEPPER1_NEGATIVELIMIT_STATEWHENLOW 1
#define ENTITY_STEPPER1_OUTPUTEFFECTER
#define ENTITY_STEPPER1_OUTPUTEFFECTER_BINDINGTYPE_NUMERICEFFECTER
extern LINTABLE_TYPE __lintable_step_dir_out1[] LINTABLE_DATA_ATTRIBUTES;
#define ENTITY_STEPPER1_OUTPUTEFFECTER_BOUNDCHANNEL step_dir_out1
#define ENTITY_STEPPER1_OUTPUTENABLE
#define ENTITY_STEPPER1_OUTPUTENABLE_BINDINGTYPE_STATEEFFECTER
#define ENTITY_STEPPER1_OUTPUTENABLE_DEFAULTSTATE NULL
#define ENTITY_STEPPER1_COMMAND
#define ENTITY_STEPPER1_COMMAND_BINDINGTYPE_STATEEFFECTER
#define ENTITY_STEPPER1_COMMAND_EFFECTERID 3
#define ENTITY_STEPPER1_COMMAND_DEFAULTSTATE 1
#define ENTITY_STEPPER1_MOTIONSTATE
#define ENTITY_STEPPER1_MOTIONSTATE_BINDINGTYPE_STATESENSOR
#define ENTITY_STEPPER1_MOTIONSTATE_SENSORID 3
#define ENTITY_STEPPER1_PFINAL
#define ENTITY_STEPPER1_PFINAL_BINDINGTYPE_NUMERICEFFECTER
#define ENTITY_STEPPER1_PFINAL_EFFECTERID 4
#define ENTITY_STEPPER1_VPROFILE
#define ENTITY_STEPPER1_VPROFILE_BINDINGTYPE_NUMERICEFFECTER
#define ENTITY_STEPPER1_VPROFILE_EFFECTERID 5
#define ENTITY_STEPPER1_APROFILE
#define ENTITY_STEPPER1_APROFILE_BINDINGTYPE_NUMERICEFFECTER
#define ENTITY_STEPPER1_APROFILE_EFFECTERID 6
#define ENTITY_STEPPER1_PERROR
#define ENTITY_STEPPER1_PERROR_BINDINGTYPE_NUMERICSENSOR
#define ENTITY_STEPPER1_PERROR_SENSORID 5
#define ENTITY_STEPPER1_PERROR_NORMALMIN 0
#define ENTITY_STEPPER1_PERROR_NORMALMAX 0
#define ENTITY_STEPPER1_PERROR_UPPERTHRESHOLDWARNING 5
#define ENTITY_STEPPER1_PERROR_UPPERTHRESHOLDCRITICAL 10
#define ENTITY_STEPPER1_PERROR_UPPERTHRESHOLDFATAL 15
#define ENTITY_STEPPER1_PERROR_LOWERTHRESHOLDWARNING -5
#define ENTITY_STEPPER1_PERROR_LOWERTHRESHOLDCRITICAL -10
#define ENTITY_STEPPER1_PERROR_LOWERTHRESHOLDFATAL -15
#define ENTITY_STEPPER1_PERROR_ENABLEDTHRESHOLDS 120
#define ENTITY_STEPPER1_PARAM_SAMPLERATE 4000
#define ENTITY_STEPPER1_PARAM_DONETIMECONSTANT 0.01
#define ENTITY_STEPPER1_PARAM_OUTPUTINDONE_COAST
#define ENTITY_STEPPER1_PARAM_OUTPUTINIDLE_COAST
#define ENTITY_STEPPER1_PARAM_OUTPUTINCONDITIONSTOP_COAST
#define ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_COAST
