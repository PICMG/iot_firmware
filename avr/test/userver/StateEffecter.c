//    StateEffecter.c
//
//    This header file defines functions related to the the StateEffecter 
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
#include "StateEffecter.h"

//====================================================
// effecter state machine states
#define DISABLED  2  // this value was chosen to match the PLDM OperationalState value
#define ENABLED   1  // this value was chosen to match the PLDM OperationalState value

//===================================================================
// stateeffecter_init()
// 
// Set state effecter to its initial state.
//
// parameters:
//    inst - a pointer to the instance data for the effecter.
// returns: nothing
void stateeffecter_init(StateEffecterInstance *inst)
{
    inst->operationalState = DISABLED;
    inst->state = 0;    // unknown state
}

//===================================================================
// stateeffecter_getOutput()
//
// get the value to output.  This function should return 
// 0 (unknown) if the effecter operational state is set to “disabled”.
//
// Since value is changed by the low priority loop, there is no need 
// to disable interrupts
//
// parameters:
//    inst - a pointer to the instance data for the effecter.
// returns: nothing
unsigned char stateeffecter_getOutput(StateEffecterInstance *inst)
{
    if (inst->state == inst->stateWhenHigh) return 1;
    return 0;
}

//===================================================================
// stateeffecter_setOperationalState()
//
// set the operational state of the effecter and return true.  Return 
// false if it is an invalid state.
//  
// interrupts are disabled brifly while the variable is updated.
//
// parameters:
//    inst - a pointer to the instance data for the effecter.
//    state - the state to set
// returns: true on success, otherwise, false
unsigned char stateeffecter_setOperationalState(StateEffecterInstance *inst, unsigned char state)
{
    // make sure the state is within range
    if (state>DISABLED) return 0;
  
    unsigned char sreg = SREG;
    __builtin_avr_cli();
    // if enabling, set the default state
    if ((state!=DISABLED) && (inst->operationalState==DISABLED)) {
        inst->state = inst->defaultState;
    }
    // set the operational mode
    if (state!=2) inst->operationalState = ENABLED;
    else inst->operationalState = DISABLED;
    SREG = sreg;
    return 1;
}

//===================================================================
// stateeffecter_setPresentState()
//
// set the present state of the effecter and return true. Return false 
// if it is an invalid state.
//
// parameters:
//    inst - a pointer to the instance data for the effecter.
// returns: 1 on success, 0 on failure
unsigned char stateeffecter_setPresentState(StateEffecterInstance *inst, unsigned char state)
{
    if (inst->allowedStatesMask & (1<<(state-1))) {
        inst->state = state;
        return 1;
    }
    return 0;
}

//===================================================================
// stateeffecter_getOperationalState()
//
// return the current operational state of the effecter.
//
// since this value is changed only in the low priority loop, there
// is no need to disable interrupts.
//
// parameters:
//    inst - a pointer to the instance data for the effecter.
// returns: the operational state of the effecter
unsigned char stateeffecter_getOperationalState(StateEffecterInstance *inst)
{
    return inst->operationalState;
}

//===================================================================
// stateeffecter_getPresentState()
//
// return the present state of the effecter.
//
// since value is a single byte, there is no need to disable interrupts
//
// parameters:
//    inst - a pointer to the instance data for the effecter.
// returns: the present state of the effecter
unsigned char stateeffecter_getPresentState(StateEffecterInstance *inst)
{
    if (inst->operationalState == DISABLED) return 0;
    return inst->state;
}


