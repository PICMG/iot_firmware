//*****************************************************************
// config.h
//
// This file was auto-generated by the PICMG firmware builder
// utility.  It includes macro definitions and data declarations
// for a custom firmware build
#pragma once
#include "platform_defs.h"

//====================
// Module-Related Macros
#define ATMEGA328PB

//====================
// PDR-Related Macros
extern PDR_BYTE_TYPE __pdr_data[] PDR_DATA_ATTRIBUTES;
#define PDR_TOTAL_SIZE 343
#define PDR_NUMBER_OF_RECORDS 11
#define PDR_MAX_RECORD_SIZE 96

//====================
// FRU-Related Macros
extern FRU_BYTE_TYPE __fru_data[] FRU_DATA_ATTRIBUTES;
#define FRU_TABLE_MAXIMUM_SIZE 0
#define FRU_TOTAL_SIZE 12
#define FRU_TOTAL_RECORD_SETS 1
#define FRU_NUMBER_OF_RECORDS 1
#define FRU_MAX_RECORD_SIZE 12

//====================
// Channel-Related Macros
#define CHANNEL_INTERLOCK_IN
#define CHANNEL_INTERLOCK_OUT
#define CHANNEL_TRIGGER_IN
#define CHANNEL_TRIGGER_OUT
#define CHANNEL_AIN2_5V
#define CHANNEL_DIGITAL_IN1

//====================
// Logical Entity-Related Macros
#define ENTITY_SIMPLE1
#define ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR
#define ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_BINDINGTYPE_STATESENSOR
#define ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_SENSORID 1
#define ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_BOUNDCHANNEL interlock_in
#define ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_USEDSTATES 3
#define ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_STATEWHENHIGH 2
#define ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_STATEWHENLOW 1
#define ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER
#define ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_BINDINGTYPE_STATEEFFECTER
#define ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_EFFECTERID 1
#define ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_BOUNDCHANNEL interlock_out
#define ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_USEDSTATES 3
#define ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_STATEWHENHIGH 2
#define ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_STATEWHENLOW 1
#define ENTITY_SIMPLE1_TRIGGERSENSOR
#define ENTITY_SIMPLE1_TRIGGERSENSOR_BINDINGTYPE_STATESENSOR
#define ENTITY_SIMPLE1_TRIGGERSENSOR_SENSORID 2
#define ENTITY_SIMPLE1_TRIGGERSENSOR_BOUNDCHANNEL trigger_in
#define ENTITY_SIMPLE1_TRIGGERSENSOR_USEDSTATES 3
#define ENTITY_SIMPLE1_TRIGGERSENSOR_STATEWHENHIGH 2
#define ENTITY_SIMPLE1_TRIGGERSENSOR_STATEWHENLOW 1
#define ENTITY_SIMPLE1_TRIGGEREFFECTER
#define ENTITY_SIMPLE1_TRIGGEREFFECTER_BINDINGTYPE_STATEEFFECTER
#define ENTITY_SIMPLE1_TRIGGEREFFECTER_EFFECTERID 2
#define ENTITY_SIMPLE1_TRIGGEREFFECTER_BOUNDCHANNEL trigger_out
#define ENTITY_SIMPLE1_TRIGGEREFFECTER_USEDSTATES 3
#define ENTITY_SIMPLE1_TRIGGEREFFECTER_STATEWHENHIGH 2
#define ENTITY_SIMPLE1_TRIGGEREFFECTER_STATEWHENLOW 1
#define ENTITY_SIMPLE1_SENSOR1
#define ENTITY_SIMPLE1_SENSOR1_BINDINGTYPE_NUMERICSENSOR
extern LINTABLE_TYPE __lintable_ain2_5v[] LINTABLE_DATA_ATTRIBUTES;
#define ENTITY_SIMPLE1_SENSOR1_BOUNDCHANNEL_PRECISION 10
#define ENTITY_SIMPLE1_SENSOR1_SENSORID 3
#define ENTITY_SIMPLE1_SENSOR1_BOUNDCHANNEL ain2_5v
#define ENTITY_SIMPLE1_SENSOR1_NORMALMIN 20
#define ENTITY_SIMPLE1_SENSOR1_NORMALMAX 30
#define ENTITY_SIMPLE1_SENSOR1_UPPERTHRESHOLDWARNING 35
#define ENTITY_SIMPLE1_SENSOR1_UPPERTHRESHOLDCRITICAL 40
#define ENTITY_SIMPLE1_SENSOR1_UPPERTHRESHOLDFATAL 45
#define ENTITY_SIMPLE1_SENSOR1_LOWERTHRESHOLDWARNING 15
#define ENTITY_SIMPLE1_SENSOR1_LOWERTHRESHOLDCRITICAL 10
#define ENTITY_SIMPLE1_SENSOR1_LOWERTHRESHOLDFATAL 5
#define ENTITY_SIMPLE1_SENSOR1_ENABLEDTHRESHOLDS 126
#define ENTITY_SIMPLE1_SENSOR2
#define ENTITY_SIMPLE1_SENSOR2_BINDINGTYPE_STATESENSOR
#define ENTITY_SIMPLE1_SENSOR2_SENSORID 3
#define ENTITY_SIMPLE1_SENSOR2_BOUNDCHANNEL digital_in1
#define ENTITY_SIMPLE1_SENSOR2_USEDSTATES 3
#define ENTITY_SIMPLE1_SENSOR2_STATEWHENHIGH 2
#define ENTITY_SIMPLE1_SENSOR2_STATEWHENLOW 1
