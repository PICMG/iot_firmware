//    NumericEffecter.c
//
//    This header file defines functions related to the the NumericEffecter 
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
#include "NumericEffecter.h"

//====================================================
// effecter state machine states
#define DISABLED  1  // this value was chosen to match the PLDM OperationalState value
#define ENABLED   0  // this value was chosen to match the PLDM OperationalState value
#define TRIGGERED 2

//====================================================
// Threshold enable bit defintions from PLDM Spec
#define THRESHOLD_ENABLE_FATAL_LOW      0x20
#define THRESHOLD_ENABLE_CRITICAL_LOW   0x10
#define THRESHOLD_ENABLE_WARNING_LOW    0x08
#define THRESHOLD_ENABLE_FATAL_HIGH     0x04
#define THRESHOLD_ENABLE_CRITICAL_HIGH  0x02
#define THRESHOLD_ENABLE_WARNING_HIGH   0x01

//====================================================
// State values
#define STATE_UNKNOWN        0
#define STATE_NORMAL         1
#define STATE_WARNING        2
#define STATE_CRITICAL       3
#define STATE_FATAL          4
#define STATE_LOWERWARNING   5
#define STATE_LOWERCRITICAL  6
#define STATE_LOWERFATAL     7
#define STATE_UPPERWARNING   8
#define STATE_UPPERCRITICAL  9
#define STATE_UPPERFATAL    10

//===================================================================
// numericeffecter_init()
// 
// Set state effecter to its initial state.
//
// parameters:
//    inst - a pointer to the instance data for the effecter.
// returns: nothing
void numericeffecter_init(NumericEffecterInstance *inst)
{
    inst->operationalState = DISABLED;
}

//===================================================================
// numericeffecter_setValue()
//
// set the value to write to the channel.  This function should have no 
// action if the effecter operational state is set to “disabled”.
//
// This function should be called from the low priority loop only.
//
// parameters:
//    inst - a pointer to the instance data for the effecter.
// returns: nothing
void numericeffecter_setValue(NumericEffecterInstance *inst, FIXEDPOINT_24_8 val)
{
    if (inst->operationalState == DISABLED) return;
    inst->value = val;
}

//===================================================================
// numericeffecter_getValue()
//
// return the value of the effecter.  This function should have no 
// action if the effecter operational state is set to “disabled”.
//
// This function can be called from either low or high priority loops.
// since the value is only updated by the low priority loop, there
// is no need to disable interrupts.
//
// parameters:
//    inst - a pointer to the instance data for the effecter.
// returns: the currect value of the effecter.
FIXEDPOINT_24_8 numericeffecter_getValue(NumericEffecterInstance *inst)
{
    return inst->value;
}

//===================================================================
// numericeffecter_setOperationalState()
//
// set the operational state of the effecter and return true.  Return 
// false if it is an invalid state.
//
// interrutps are disabled for a short period of time when the variable
// is changed.
//  
// parameters:
//    inst - a pointer to the instance data for the effecter.
// returns: true on success, otherwise, false
unsigned char numericeffecter_setOperationalState(NumericEffecterInstance *inst, unsigned char state)
{
    if (state>1) return 0;
 
    unsigned char sreg = SREG;
    __builtin_avr_cli();
    inst->operationalState = state;
    SREG = sreg;
    return 1;
}

//===================================================================
// numericeffecter_getOperationalState()
//
// return the current operational state of the effecter.
//
// The operational state is only changed from the low priority loop.
// This function can be called from either the low or high-priority 
// loops.
//
// parameters:
//    inst - a pointer to the instance data for the effecter.
// returns: the operational state of the effecter
unsigned char numericeffecter_getOperationalState(NumericEffecterInstance *inst)
{
    return inst->operationalState;
}

