//    NumericSensor.c
//
//    This header file defines functions related to the the NumericSensor 
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
#include "NumericSensor.h"

//====================================================
// sensor state machine states
#define DISABLED  1  // this value was chosen to match the PLDM OperationalState value
#define ENABLED   0  // this value was chosen to match the PLDM OperationalState value
#define TRIGGERED 2

//====================================================
// Threshold enable bit defintions from PLDM Spec
#define THRESHOLD_FATALLOW_SUPPORTED_BIT     0x40
#define THRESHOLD_FATALHIGH_SUPPORTED_BIT    0x20
#define THRESHOLD_CRITICALLOW_SUPPORTED_BIT  0x10
#define THRESHOLD_CRITICALHIGH_SUPPORTED_BIT 0x08

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
// numericsensor_init()
// 
// Set state sensor to its initial state.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: nothing
void numericsensor_init(NumericSensorInstance *inst)
{
    inst->operationalState = DISABLED;
    inst->previousState = 0;           
    inst->eventState = 0;         
    eventgenerator_init(&(inst->eventGen));
    inst->eventGen.sendEvent = 0;
    inst->eventGen.eventOccurred = 0;
}

//===================================================================
// numericsensor_setValue()
//
// set the value read from the channel.  This function should have no 
// action if the sensor operational state is set to “disabled”.
//
// This function should be called from the high priority loop only.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: nothing
void numericsensor_setValue(NumericSensorInstance *inst, FIXEDPOINT_24_8 val)
{
    inst->value = val;
}

//===================================================================
// getPresentStateWithHysteresis
// A helper function.
// return the new state of the controller taking into account hysteresis
// and state priority
//
// this function is only called during the high priority loop
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the present state of the sensor
static unsigned char numericsensor_getPresentStateWithHysteresis(NumericSensorInstance *inst)
{
    if (!eventgenerator_isEnabled(&(inst->eventGen))) {
        // sensor not enabled - return unknown
        return STATE_UNKNOWN;
    }

    FIXEDPOINT_24_8 valh = inst->value+inst->hysteresisValue;
    FIXEDPOINT_24_8 vall = inst->value-inst->hysteresisValue;

    if ((inst->thresholdEnables&THRESHOLD_FATALHIGH_SUPPORTED_BIT)&&(valh>=inst->thresholdFatalHigh)) return STATE_UPPERFATAL;
    if ((inst->thresholdEnables&THRESHOLD_FATALLOW_SUPPORTED_BIT)&&(vall<=inst->thresholdFatalLow)) return STATE_LOWERFATAL;
    if (inst->previousState==STATE_UPPERFATAL) return STATE_UPPERFATAL;
    if (inst->previousState==STATE_LOWERFATAL) return STATE_LOWERFATAL;
    if ((inst->thresholdEnables&THRESHOLD_CRITICALHIGH_SUPPORTED_BIT)&&(valh>=inst->thresholdCriticalHigh)) return STATE_UPPERCRITICAL;
    if ((inst->thresholdEnables&THRESHOLD_CRITICALLOW_SUPPORTED_BIT)&&(vall<=inst->thresholdCriticalLow)) return STATE_LOWERCRITICAL;
    if (inst->previousState==STATE_UPPERCRITICAL) return STATE_UPPERCRITICAL;
    if (inst->previousState==STATE_LOWERCRITICAL) return STATE_LOWERCRITICAL;
    if (valh>=inst->thresholdWarnHigh) return STATE_UPPERWARNING;
    if (valh<=inst->thresholdWarnLow) return STATE_LOWERWARNING;
    if (inst->previousState==STATE_UPPERWARNING) return STATE_UPPERWARNING;
    if (inst->previousState==STATE_LOWERWARNING) return STATE_LOWERWARNING;
    return STATE_NORMAL;
}

//===================================================================
// numericsensor_updateSensorState()
//
// update the state sensor state and manage the state-change state 
// machine.
//
// This function should be called from the high priority loop only.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: nothing
void numericsensor_updateSensorState(NumericSensorInstance *inst)
{
    switch (inst->operationalState) {
        case DISABLED:
            // do nothing - wait to be enabled
            break;
        case ENABLED:
            if ((eventgenerator_isEnabled(&(inst->eventGen)))&&
                (inst->previousState != numericsensor_getPresentStateWithHysteresis(inst))) {
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
    inst->previousState = numericsensor_getPresentStateWithHysteresis(inst);
}

//===================================================================
// numericsensor_sensorRearm()
//
// rearm the sensor so that it can generate another event 
// (if enabled)
//
// This function is called from the low priority loop.  Interrupts 
// are disabled for a short period of time when the variable is 
// changed.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: nothing
void numericsensor_sensorRearm(NumericSensorInstance *inst)
{
    unsigned char sreg = SREG;
    __builtin_avr_cli();
    if (inst->operationalState == TRIGGERED) {
        inst->operationalState = ENABLED;
    }
    SREG = sreg;
}

//===================================================================
// numericsensor_isTriggered()
//
// returns true if a state change has occured that triggered an event.
//
// this function should only be called from the high-priority loop.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: true if triggered, otherwise, false
unsigned char numericsensor_isTriggered(NumericSensorInstance *inst)
{
    return inst->operationalState == TRIGGERED;
}

//===================================================================
// numericsensor_setOperationalState()
//
// set the operational state of the sensor and return true.  Return 
// false if it is an invalid state.
//
// interrutps are disabled for a short period of time when the variable
// is changed.
//  
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: true on success, otherwise, false
unsigned char numericsensor_setOperationalState(NumericSensorInstance *inst, unsigned char state)
{
    if (state>1) return 0;
 
    unsigned char sreg = SREG;
    __builtin_avr_cli();
    inst->operationalState = state;
    SREG = sreg;
    return 1;
}

//===================================================================
// numericsensor_setPresentState()
//
// set the present state of the sensor and return true. Return false 
// if it is an invalid state.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the present state of the sensor
unsigned char numericsensor_setPresentState(NumericSensorInstance *inst, unsigned char state)
{
    // do nothing and return true
    return 1;
}

//===================================================================
// numericsensor_getOperationalState()
//
// return the current operational state of the sensor.
//
// The operational state is only changed from the low priority loop.
// This function can be called from either the low or high-priority 
// loops.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the operational state of the sensor
unsigned char numericsensor_getOperationalState(NumericSensorInstance *inst)
{
    return inst->operationalState;
}

//===================================================================
// numericsensor_getPresentState()
//
// return the present state of the sensor.
//
// this function disables interrupts for a short period of time when the
// variable is evaluated.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the present state of the sensor
unsigned char numericsensor_getPresentState(NumericSensorInstance *inst)
{
    if (!eventgenerator_isEnabled(&(inst->eventGen))) {
        // sensor not enabled - return unknown
        return STATE_UNKNOWN;
    }

    unsigned char sreg = SREG;
    __builtin_avr_cli();
    FIXEDPOINT_24_8 val = inst->value;
    SREG = sreg;

    if ((inst->thresholdEnables&THRESHOLD_FATALHIGH_SUPPORTED_BIT)&&(val>=inst->thresholdFatalHigh)) return STATE_UPPERFATAL;
    if ((inst->thresholdEnables&THRESHOLD_CRITICALHIGH_SUPPORTED_BIT)&&(val>=inst->thresholdCriticalHigh)) return STATE_UPPERCRITICAL;
    if (val>=inst->thresholdWarnHigh) return STATE_UPPERWARNING;
    if ((inst->thresholdEnables&THRESHOLD_FATALLOW_SUPPORTED_BIT)&&(val<=inst->thresholdFatalLow)) return STATE_LOWERFATAL;
    if ((inst->thresholdEnables&THRESHOLD_CRITICALLOW_SUPPORTED_BIT)&&(val<=inst->thresholdCriticalLow)) return STATE_LOWERCRITICAL;
    if (val<=inst->thresholdWarnLow) return STATE_LOWERWARNING;
    return STATE_NORMAL;
}

//===================================================================
// numericsensor_getEventState()
//
// return the state that caused the current event in for the sensor.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the state that caused the event
unsigned char numericsensor_getEventState(NumericSensorInstance *inst)
{
    // since this is a single byte, there is no need to disable interrupts
    return inst->eventState;
}

//===================================================================
// numericsensor_getSensorPreviousState()
//
// return the state of the sensor prior to the current event.
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the previous state of the sensor.
unsigned char numericsensor_getSensorPreviousState(NumericSensorInstance *inst)
{
    // since this is a single byte, there is no need to disable interrupts
    return inst->previousState;
}

//===================================================================
// numericsensor_getWarningHighThreshold()
//
// return the value of the High Warning threshold
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the value of the threshold.
FIXEDPOINT_24_8 numericsensor_getWarningHighThreshold(NumericSensorInstance *inst)
{
    // no need to disable interrupts because this value is only updated during
    // the low priority loop and interrupts are already disabled when updating
    return inst->thresholdWarnHigh;
}

//===================================================================
// numericsensor_getCriticalHighThreshold()
//
// return the value of the High Critical threshold
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the value of the threshold.
FIXEDPOINT_24_8 numericsensor_getCriticalHighThreshold(NumericSensorInstance *inst)
{
    // no need to disable interrupts because this value is only updated during
    // the low priority loop and interrupts are already disabled when updating
    return inst->thresholdCriticalHigh;
}

//===================================================================
// numericsensor_getFatalHighThreshold()
//
// return the value of the High Fatal threshold
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the value of the threshold.
FIXEDPOINT_24_8 numericsensor_getFatalHighThreshold(NumericSensorInstance *inst)
{
    // no need to disable interrupts because this value is only updated during
    // the low priority loop and interrupts are already disabled when updating
    return inst->thresholdFatalHigh;
}

//===================================================================
// numericsensor_getWarningLowThreshold()
//
// return the value of the Low Warning threshold
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the value of the threshold.
FIXEDPOINT_24_8 numericsensor_getWarningLowThreshold(NumericSensorInstance *inst)
{
    // no need to disable interrupts because this value is only updated during
    // the low priority loop and interrupts are already disabled when updating
    return inst->thresholdWarnLow;
}

//===================================================================
// numericsensor_getCriticalLowThreshold()
//
// return the value of the Low Critical threshold
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the value of the threshold.
FIXEDPOINT_24_8 numericsensor_getCriticalLowThreshold(NumericSensorInstance *inst)
{
    // no need to disable interrupts because this value is only updated during
    // the low priority loop and interrupts are already disabled when updating
    return inst->thresholdCriticalLow;
}

//===================================================================
// numericsensor_getFatalLowThreshold()
//
// return the value of the Low Fatal threshold
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
// returns: the value of the threshold.
FIXEDPOINT_24_8 numericsensor_getFatalLowThreshold(NumericSensorInstance *inst)
{
    // no need to disable interrupts because this value is only updated during
    // the low priority loop and interrupts are already disabled when updating
    return inst->thresholdFatalLow;
}

//===================================================================
// numericsensor_setThresholds()
//
// set the sensor thresholds to the specified values.  Values that are
// not used (as specified in the PDR) will not be used.
//
// interrupts are disabled for a short period of time during changing
// the variable values. 
//
// parameters:
//    inst - a pointer to the instance data for the sensor.
//    fh  - the value for the fatal high threshold.
//    crh - the value for the critical high threshold.
//    wh  - the value for the warning high threshold.
//    crl - the value for the critical low threshold.
//    fl  - the value for the fatal high threshold.
// returns: true if thresholds have been set.  false if not.
unsigned char   numericsensor_setThresholds(NumericSensorInstance *inst,FIXEDPOINT_24_8 fh,FIXEDPOINT_24_8 crh, FIXEDPOINT_24_8 wh,FIXEDPOINT_24_8 wl,FIXEDPOINT_24_8 crl,FIXEDPOINT_24_8 fl)
{
    // check to make sure that the new values are valid.  Higher thresholds with increasing
    // severity should have increasing values.  Low thresholds with increasing severity should
    // have decreasing values.
    if (inst->thresholdEnables&(THRESHOLD_FATALHIGH_SUPPORTED_BIT|THRESHOLD_CRITICALHIGH_SUPPORTED_BIT)) {
        if (fh<=crh) return 0;
    }
    if (inst->thresholdEnables&THRESHOLD_FATALHIGH_SUPPORTED_BIT) {
        if (fh<=wh) return 0;
    }
    if (inst->thresholdEnables&THRESHOLD_FATALHIGH_SUPPORTED_BIT) {
        if (fh<=wl) return 0;
    }
    if (inst->thresholdEnables&(THRESHOLD_FATALHIGH_SUPPORTED_BIT|THRESHOLD_CRITICALLOW_SUPPORTED_BIT)) {
        if (fh<=crl) return 0;
    }
    if (inst->thresholdEnables&(THRESHOLD_FATALHIGH_SUPPORTED_BIT|THRESHOLD_FATALLOW_SUPPORTED_BIT)) {
        if (fh<=fl) return 0;
    }
    if (inst->thresholdEnables&THRESHOLD_CRITICALHIGH_SUPPORTED_BIT) {
        if (crh<=wh) return 0;
    }
    if (inst->thresholdEnables&THRESHOLD_CRITICALHIGH_SUPPORTED_BIT) {
        if (crh<=wl) return 0;
    }
    if (inst->thresholdEnables&(THRESHOLD_CRITICALHIGH_SUPPORTED_BIT|THRESHOLD_CRITICALLOW_SUPPORTED_BIT)) {
        if (crh<=crl) return 0;
    }
    if (inst->thresholdEnables&(THRESHOLD_CRITICALHIGH_SUPPORTED_BIT|THRESHOLD_FATALLOW_SUPPORTED_BIT)) {
        if (crh<=fl) return 0;
    }
    if (1) {
        if (wh<=wl) return 0;
    }
    if (inst->thresholdEnables&THRESHOLD_CRITICALLOW_SUPPORTED_BIT) {
        if (wh<=crl) return 0;
    }
    if (inst->thresholdEnables&THRESHOLD_FATALLOW_SUPPORTED_BIT) {
        if (wh<=fl) return 0;
    }
    if (inst->thresholdEnables&THRESHOLD_CRITICALLOW_SUPPORTED_BIT) {
        if (wl<=crl) return 0;
    }
    if (inst->thresholdEnables&THRESHOLD_FATALLOW_SUPPORTED_BIT) {
        if (wl<=fl) return 0;
    }
    if (inst->thresholdEnables&(THRESHOLD_CRITICALLOW_SUPPORTED_BIT|THRESHOLD_FATALLOW_SUPPORTED_BIT)) {
        if (crl<=fl) return 0;
    }
    // range checking complete - change the values, disabling interrupts to prevent
    // potential interrupt during updates
    unsigned char sreg = SREG;
    __builtin_avr_cli();

    inst->thresholdFatalHigh    = fh;
    inst->thresholdCriticalHigh = crh;
    inst->thresholdWarnHigh     = wh;
    inst->thresholdFatalLow    = fl;
    inst->thresholdCriticalLow = crl;
    inst->thresholdWarnLow     = wl;

    // restore interrupts to priovious state
    SREG = sreg;
    return 1;
}
