//    StateSensor.c
//
//    This header file defines functions related to the the StateSensor 
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
#include <avr/io.h>
#include "node.h"
#include "StateSensor.h"

//====================================================
// sensor state machine states
#define DISABLED  1  // this value was chosen to match the PLDM OperationalState value
#define ENABLED   0  // this value was chosen to match the PLDM OperationalState value
#define TRIGGERED 2

//===================================================================
// statesensor_init()
// 
// Set state sensor to its initial state.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: nothing
void statesensor_init(StateSensorInstance *inst)
{
    inst->operationalState = DISABLED;
    inst->previousState = 0;           
    inst->eventState = 0;         
    eventgenerator_init(&(inst->eventGen));
    inst->eventGen.sendEvent = 0;
    inst->eventGen.eventOccurred = 0;
}

//===================================================================
// statesensor_setValue()
//
// set the value read from the channel.  This function should have no 
// action if the sensor operational state is set to “disabled”.
//
// value is a single character, so there is no need to disable interrupts
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: nothing
void statesensor_setValue(StateSensorInstance *inst, unsigned char val)
{
    inst->value = val;
}

//===================================================================
// statesensor_updateSensorState()
//
// update the state sensor state and manage the state-change state 
// machine.
//
// this function is called only from the high priority loop
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: nothing
void statesensor_updateSensorState(StateSensorInstance *inst)
{
    switch (inst->operationalState) {
        case DISABLED:
            // do nothing - wait to be enabled
            break;
        case ENABLED:
            if ((eventgenerator_isEnabled(&(inst->eventGen))) && 
                (inst->previousState != statesensor_getPresentState(inst))) {
                inst->operationalState = TRIGGERED;
            }
            break;
        case TRIGGERED:
            // rearm case already coverd, check to see if events have
            // been disabled.
            if (!eventgenerator_isEnabled(&(inst->eventGen))) {
                inst->operationalState = ENABLED;
            }
            break;
        default:
            inst->operationalState = DISABLED;
    }
    inst->previousState = statesensor_getPresentState(inst);
}

//===================================================================
// statesensor_sensorRearm()
//
// rearm the sensor so that it can generate another event 
// (if enabled)
//
// interrupts are disabled for a brief period during this call.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: nothing
void statesensor_sensorRearm(StateSensorInstance *inst)
{
    unsigned char sreg = SREG;
    __builtin_avr_cli();
    if (inst->operationalState == TRIGGERED) {
        inst->operationalState = ENABLED;
    }
    SREG = sreg;
}

//===================================================================
// statesensor_isTriggered()
//
// returns true if a state change has occured that triggered an event.
//
// This function is called only from the high-priority loop.
//  
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: true if triggered, otherwise, false
unsigned char statesensor_isTriggered(StateSensorInstance *inst)
{
    return inst->operationalState == TRIGGERED;
}

//===================================================================
// statesensor_setOperationalState()
//
// set the operational state of the sensor and return true.  Return 
// false if it is an invalid state.
//  
// interrupts are disabled brifly while the variable is updated.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: success response code, or error response code
unsigned char statesensor_setOperationalState(StateSensorInstance *inst, unsigned char state, unsigned char eventMessageEnable)
{
    if (state>1) return RESPONSE_ERROR;
  
    unsigned char sreg = SREG;
    __builtin_avr_cli();
    inst->operationalState = state;
    SREG = sreg;

    if (eventMessageEnable == 1) eventgenerator_setEnableAsyncEvents(&inst->eventGen,0);  // disable
    else if (eventMessageEnable == 4) eventgenerator_setEnableAsyncEvents(&inst->eventGen,1); // enable
    return RESPONSE_SUCCESS;
}

//===================================================================
// statesensor_setPresentState()
//
// set the present state of the sensor and return true. Return false 
// if it is an invalid state.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the present state of the sensor
unsigned char statesensor_setPresentState(StateSensorInstance *inst, unsigned char state)
{
    // do nothing and return true
    return 1;
}

//===================================================================
// statesensor_getOperationalState()
//
// return the current operational state of the sensor.
//
// since this value is changed only in the low priority loop, there
// is no need to disable interrupts.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the operational state of the sensor
unsigned char statesensor_getOperationalState(StateSensorInstance *inst)
{
    return inst->operationalState;
}

//===================================================================
// statesensor_getPresentState()
//
// return the present state of the sensor.
//
// since value is a single byte, there is no need to disable interrupts
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the present state of the sensor
unsigned char statesensor_getPresentState(StateSensorInstance *inst)
{
    if (inst->operationalState==DISABLED) {
        // sensor not enabled - return unknown
        return 0;
    }

    return inst->value;
}

//===================================================================
// statesensor_getEventState()
//
// return the state that caused the current event in for the sensor.
//
// since event state is a single byte, there is no need to disable
// interrupts.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the state that caused the event
unsigned char statesensor_getEventState(StateSensorInstance *inst)
{
    return inst->eventState;
}

//===================================================================
// statesensor_getSensorPreviousState()
//
// return the state of the sensor prior to the current event.
//
// since previousState is a single byte, there is no need to disable
// interrupts.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the previous state of the sensor.
unsigned char statesensor_getSensorPreviousState(StateSensorInstance *inst)
{
    return inst->previousState;
}

