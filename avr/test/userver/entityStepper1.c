//    EntityStepper1.c
//
//    This header file defines functions related to the the Stepper1  
//    Logical Entity Type.  Much of this code is conditionally compiled
//    based on macro definitions from the configuraiton header file.
//
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
// TODO: place this definition in a common header or compile macro
#ifndef __AVR_ATmega328P__ 
#define __AVR_ATmega328P__
#endif
#include "avr/io.h"
#include "config.h"

#ifdef ENTITY_STEPPER1
    #include "StateSensor.h"
    #include "NumericSensor.h"
    #include "StateEffecter.h"
    #include "NumericEffecter.h"
    #include "channels.h"
    #include "node.h"
    #include "vprofiler.h"
    #include "stepdir_out.h"

    #define SINT32_TYPE 5

    #define CONCATENATE(x,y) x ## y
    #define CALL_CHANNEL_FUNCTION(channel, function) CONCATENATE(channel, function)

    #define STATE_IDLE       1
    #define STATE_COND       2
    #define STATE_ERROR      3
    #define STATE_RUNNINGV   4
    #define STATE_RUNNING    5
    #define STATE_WAITING    6
    #define STATE_DONE       7
    #define STATE_STOPPINGV  (16+STATE_RUNNINGV)
    #define STATE_STOPPING   (16+STATE_RUNNING)


    #define MOTOR_CMD_NONE   0
    #define MOTOR_CMD_RUN    1
    #define MOTOR_CMD_STOP   2
    #define MOTOR_CMD_DONE   3
    #define MOTOR_CMD_ERR    4
    #define MOTOR_CMD_COND   5

    #define MOTOR_MODE_NOWAIT  0
    #define MOTOR_MODE_WAIT    1

    #define MOTOR_FLAGS_ERROR      0x80
    #define MOTOR_FLAGS_INTERLOCK  0x40
    #define MOTOR_FLAGS_WARNING    0x20
    #define MOTOR_FLAGS_TRIGGER    0x10
    #define MOTOR_FLAGS_POSLIMIT   0x08
    #define MOTOR_FLAGS_NEGLIMIT   0x04
    #define MOTOR_FLAGS_REVERSE    0x02
    #define MOTOR_FLAGS_VMODE      0x01

    #define SWITCH_STATE_PRESSED_ON   0x01
    #define SWITCH_STATE_RELEASED_OFF 0x02

    unsigned char servo_cmd   = MOTOR_CMD_NONE;
    unsigned char servo_mode  = MOTOR_MODE_NOWAIT;
    unsigned char servo_flags = 0x00;

    // buffered values for the requested position, velocity and acceleration
    //long requested_position     = 1000000L;
    //FP16 requested_velocity     = TO_FP16(511);
    //FP16 requested_acceleration = TO_FP16(1);
    //FP16 requested_kffa;
    static char mode_scurve    = 0;
    static unsigned char state = STATE_IDLE;
    static int deltax_t0           = 0;  // the position steps that will be made this frame
    static int deltax_t1           = 0;  // the position steps that were made last frame;

    static FP16 vel = TO_FP16(0);

    //===============================================================
    // Sensor-Specific Code
    //===============================================================
    static StateSensorInstance globalInterlockSensorInst; 
    static void globalInterlockSensor_sendEvent(PldmRequestHeader *rxHeader, unsigned char more)
    {
        node_sendStateSensorEvent(rxHeader,more,&(globalInterlockSensorInst.eventGen),
            ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_SENSORID,
            statesensor_getSensorPreviousState(&globalInterlockSensorInst));
    }

    static StateSensorInstance triggerSensorInst; 
    static void triggerSensor_sendEvent(PldmRequestHeader *rxHeader, unsigned char more)
    {
        node_sendStateSensorEvent(rxHeader,more,&(triggerSensorInst.eventGen),
            ENTITY_STEPPER1_TRIGGERSENSOR_SENSORID,
            statesensor_getSensorPreviousState(&triggerSensorInst));
    }

    static StateSensorInstance motionStateSensorInst; 
    static void motionStateSensor_sendEvent(PldmRequestHeader *rxHeader, unsigned char more)
    {
        node_sendStateSensorEvent(rxHeader,more,&(motionStateSensorInst.eventGen),
            ENTITY_STEPPER1_MOTIONSTATE_SENSORID,
            statesensor_getSensorPreviousState(&motionStateSensorInst));
    }

    #ifdef ENTITY_STEPPER1_POSITIVELIMIT
        static StateSensorInstance positiveLimitSensorInst; 
        static void positiveLimitSensor_sendEvent(PldmRequestHeader *rxHeader, unsigned char more)
        {
            node_sendStateSensorEvent(rxHeader,more,&(positiveLimitSensorInst.eventGen),
                ENTITY_STEPPER1_POSITIVELIMIT_SENSORID,
                statesensor_getSensorPreviousState(&positiveLimitSensorInst));
        }
    #endif

    #ifdef ENTITY_STEPPER1_NEGATIVELIMIT
        static StateSensorInstance negativeLimitSensorInst; 
        static void negativeLimitSensor_sendEvent(PldmRequestHeader *rxHeader, unsigned char more)
        {
            node_sendStateSensorEvent(rxHeader,more,&(negativeLimitSensorInst.eventGen),
                ENTITY_STEPPER1_NEGATIVELIMIT_SENSORID,
                statesensor_getSensorPreviousState(&negativeLimitSensorInst)
            );
        } 
    #endif

    #ifdef ENTITY_STEPPER1_POSITION
        static NumericSensorInstance positionSensorInst;
        static void positionSensor_sendEvent(PldmRequestHeader *rxHeader, unsigned char more)
        {
            node_sendNumericSensorEvent(rxHeader,more,&(positionSensorInst.eventGen),
                ENTITY_STEPPER1_POSITION_SENSORID,
                numericsensor_getSensorPreviousState(&positionSensorInst),
                positionSensorInst.value
            );
        } 
    #endif 

    //===============================================================
    // Global variables related to effecters
    //===============================================================
    static StateEffecterInstance globalInterlockEffecterInst; 
    static StateEffecterInstance triggerEffecterInst;
    static NumericEffecterInstance outputEffecterInst;
    static StateEffecterInstance commandEffecterInst;
    static NumericEffecterInstance pfinalEffecterInst;
    static NumericEffecterInstance vprofileEffecterInst;
    static NumericEffecterInstance aprofileEffecterInst;
    #ifdef ENTITY_STEPPER1_OUTPUTENABLE
        static StateEffecterInstance outputEnableEffecterInst;
    #endif 
    #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
        static StateEffecterInstance brakeEffecterInst;
    #endif 
     
    //===============================================================
    // entityStepper1_init()
    //
    // initialize all the sensors and effecters associated with the
    // stepper1 logical entity.  Some sensors/effecters are always 
    // present, others are present based on the firmware configuration.
    // This function uses firmware configuration macros to switch
    // in the proper sensors and effecters for the firmware build.
    //
    // parameters: none
    // returns: nothing
    void entityStepper1_init() 
    {
        // initilize the globalInterlockSensor
        statesensor_init(&globalInterlockSensorInst);
        globalInterlockSensorInst.stateWhenHigh = ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_STATEWHENHIGH;
        globalInterlockSensorInst.stateWhenLow = ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_STATEWHENLOW;
        globalInterlockSensorInst.eventGen.sendEvent = globalInterlockSensor_sendEvent;
 
        // initialize the triggerSensor
        statesensor_init(&triggerSensorInst);
        triggerSensorInst.stateWhenHigh = ENTITY_STEPPER1_TRIGGERSENSOR_STATEWHENHIGH;
        triggerSensorInst.stateWhenLow = ENTITY_STEPPER1_TRIGGERSENSOR_STATEWHENLOW;
        triggerSensorInst.eventGen.sendEvent = triggerSensor_sendEvent;

        // initialize the motionStateSensor
        statesensor_init(&motionStateSensorInst);
        motionStateSensorInst.eventGen.sendEvent = motionStateSensor_sendEvent;

        #ifdef ENTITY_STEPPER1_POSITIVELIMIT
            statesensor_init(&positiveLimitSensorInst);
            positiveLimitSensorInst.stateWhenHigh = ENTITY_STEPPER1_POSITIVELIMIT_STATEWHENHIGH;
            positiveLimitSensorInst.stateWhenLow  = ENTITY_STEPPER1_POSITIVELIMIT_STATEWHENLOW;
            positiveLimitSensorInst.eventGen.sendEvent = positiveLimitSensor_sendEvent;
        #endif

        #ifdef ENTITY_STEPPER1_NEGATIVELIMIT
            statesensor_init(&negativeLimitSensorInst);
            negativeLimitSensorInst.stateWhenHigh = ENTITY_STEPPER1_NEGATIVELIMIT_STATEWHENHIGH;
            negativeLimitSensorInst.stateWhenLow  = ENTITY_STEPPER1_NEGATIVELIMIT_STATEWHENLOW;
            negativeLimitSensorInst.eventGen.sendEvent = negativeLimitSensor_sendEvent;
        #endif

        // initialize the global interlock effecter
        stateeffecter_init(&globalInterlockEffecterInst); 
        globalInterlockEffecterInst.stateWhenHigh = ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_STATEWHENHIGH;
        globalInterlockEffecterInst.stateWhenLow = ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_STATEWHENLOW;
        globalInterlockEffecterInst.allowedStatesMask = 
            (1<<(ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_STATEWHENHIGH-1))|
            (1<<(ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_STATEWHENLOW-1));
        globalInterlockEffecterInst.defaultState = globalInterlockEffecterInst.stateWhenHigh;

        // initialize the trigger effecter
        stateeffecter_init(&triggerEffecterInst);
        triggerEffecterInst.stateWhenHigh = ENTITY_STEPPER1_TRIGGEREFFECTER_STATEWHENHIGH;
        triggerEffecterInst.stateWhenLow = ENTITY_STEPPER1_TRIGGEREFFECTER_STATEWHENLOW;
        triggerEffecterInst.allowedStatesMask = 
            (1<<(ENTITY_STEPPER1_TRIGGEREFFECTER_STATEWHENHIGH-1))|
            (1<<(ENTITY_STEPPER1_TRIGGEREFFECTER_STATEWHENLOW-1));
        triggerEffecterInst.defaultState = triggerEffecterInst.stateWhenHigh;

        // initialize the command effecter
        stateeffecter_init(&commandEffecterInst);
        commandEffecterInst.allowedStatesMask = 7; 
        commandEffecterInst.defaultState = 2;   // default state = stop

        // initialize the output enable effecter
        #ifdef ENTITY_STEPPER1_OUTPUTENABLE
            stateeffecter_init(&outputEnableEffecterInst);
            outputEnableEffecterInst.allowedStatesMask = 3;
            outputEnableEffecterInst.stateWhenHigh = 2;
            outputEnableEffecterInst.stateWhenLow = 1;
            outputEnableEffecterInst.defaultState = 1;
            DDRD |= (1<<PD5);
        #endif 

        // initialize the brake enable effecter
        #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
            stateeffecter_init(&brakeEffecterInst);
            brake.stateWhenHigh = 2;
            brake.stateWhenLow = 1;
            brakeEffecterInst.allowedStatesMask = 3; 
            brakeEffecterInst.defaultState = 1;
        #endif 

        #ifdef ENTITY_STEPPER1_POSITION
            numericsensor_init(&positionSensorInst);
            positionSensorInst.thresholdEnables = ENTITY_STEPPER1_POSITION_ENABLEDTHRESHOLDS; 
            numericsensor_setThresholds(&positionSensorInst,
                ENTITY_STEPPER1_POSITION_UPPERTHRESHOLDFATAL,
                ENTITY_STEPPER1_POSITION_UPPERTHRESHOLDCRITICAL,
                ENTITY_STEPPER1_POSITION_UPPERTHRESHOLDWARNING,
                ENTITY_STEPPER1_POSITION_LOWERTHRESHOLDWARNING,
                ENTITY_STEPPER1_POSITION_LOWERTHRESHOLDCRITICAL,
                ENTITY_STEPPER1_POSITION_LOWERTHRESHOLDFATAL);
            positionSensorInst.eventGen.sendEvent = positionSensor_sendEvent;
        #endif 

        // initialize the output effecter
        numericeffecter_init(&outputEffecterInst);
        outputEffecterInst.maxSettable = 0x7FFFFFFF;
        outputEffecterInst.minSettable = -0x7FFFFFFF;
        outputEffecterInst.defaultValue = ENTITY_STEPPER1_OUTPUTEFFECTER_DEFAULTVALUE;

        // initialize the pfinal effecter
        numericeffecter_init(&pfinalEffecterInst);
        pfinalEffecterInst.maxSettable = 0x7FFFFFFF;
        pfinalEffecterInst.minSettable = -0x7FFFFFFF;
        pfinalEffecterInst.value = ENTITY_STEPPER1_PFINAL_DEFAULTVALUE;
        pfinalEffecterInst.defaultValue = ENTITY_STEPPER1_PFINAL_DEFAULTVALUE;

        // initialize the vprofile effecter
        numericeffecter_init(&vprofileEffecterInst);
        vprofileEffecterInst.maxSettable = 0x7FFFFFFF;
        vprofileEffecterInst.minSettable = -0x7FFFFFFF;
        vprofileEffecterInst.value = ENTITY_STEPPER1_VPROFILE_DEFAULTVALUE;
        vprofileEffecterInst.defaultValue = ENTITY_STEPPER1_VPROFILE_DEFAULTVALUE;

        // initialize the aprofile effecter
        numericeffecter_init(&aprofileEffecterInst);
        aprofileEffecterInst.maxSettable = 0x7FFFFFFF;
        aprofileEffecterInst.minSettable = -0x7FFFFFFF;
        aprofileEffecterInst.value = ENTITY_STEPPER1_APROFILE_DEFAULTVALUE;
        aprofileEffecterInst.defaultValue = ENTITY_STEPPER1_APROFILE_DEFAULTVALUE;
    }

    //===============================================================
    // entityStepper1_readChannels()
    //
    // this function causes each channel used by the logical entity to 
    // read its current value and store it in it's channel data.
    void entityStepper1_readChannels() {
        // read the globalInterlockSensor's channel
        CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_BOUNDCHANNEL,_sample());
        statesensor_setValueFromChannelBit(&globalInterlockSensorInst,
            CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_BOUNDCHANNEL,_getRawData())
        );
        // read the triggerSensor' channel
        CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_TRIGGERSENSOR_BOUNDCHANNEL,_sample());
        statesensor_setValueFromChannelBit(&triggerSensorInst,
            CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_TRIGGERSENSOR_BOUNDCHANNEL,_getRawData())
        );
        
        // read the motionStateSensor's channel
        // do nothing - this channel is virtual

        #ifdef ENTITY_STEPPER1_POSITIVELIMIT_BOUNDCHANNEL
            // read the positive limit sensor's channel
            CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_POSITIVELIMIT_BOUNDCHANNEL,_sample());
            statesensor_setValueFromChannelBit(&positiveLimitSensorInst,
                CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_POSITIVELIMIT_BOUNDCHANNEL,_getRawData())
            );
        #endif

        #ifdef ENTITY_STEPPER1_NEGATIVELIMIT_BOUNDCHANNEL
            // read the negative limit sensor's channel
            CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_NEGATIVELIMIT_BOUNDCHANNEL,_sample());
            statesensor_setValueFromChannelBit(&negativeLimitSensorInst,
                CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_NEGATIVELIMIT_BOUNDCHANNEL,_getRawData())
            );
        #endif

        #ifdef ENTITY_STEPPER1_POSITION_BOUNDCHANNEL
            // read the position sensor's channel
            CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_POSITION_BOUNDCHANNEL,_sample);
        #else
            positionSensorInst.value += deltax_t1;
        #endif
    }

    //===============================================================
    // entityStepper1_updateEvents()
    //
    // this function updates the event state for each sensor that is
    // not currently in the "SENT" state.
    void entityStepper1_updateEvents(char *fifoInsertId) {
        if (!eventgenerator_isEventSent(&(globalInterlockSensorInst.eventGen))) {
            eventgenerator_updateEventStateMachine(&(globalInterlockSensorInst.eventGen));
            if (eventgenerator_isEventPending(&(globalInterlockSensorInst.eventGen)) && 
               (globalInterlockSensorInst.eventGen.priority < 0)) {
                // this event has not yet been placed in the fifo
                globalInterlockSensorInst.eventGen.priority = *fifoInsertId;
                *fifoInsertId = ((*fifoInsertId)+1)&0xF;
            }
        }
        
        if (!eventgenerator_isEventSent(&(triggerSensorInst.eventGen))) {
            eventgenerator_updateEventStateMachine(&(triggerSensorInst.eventGen));
            if (eventgenerator_isEventPending(&(triggerSensorInst.eventGen)) && 
               (triggerSensorInst.eventGen.priority < 0)) {
                // this event has not yet been placed in the fifo
                triggerSensorInst.eventGen.priority = *fifoInsertId;
                *fifoInsertId = ((*fifoInsertId)+1)&0xF;
            }
        }
        
        #ifdef ENTITY_STEPPER1_POSITIVELIMIT_BOUNDCHANNEL
            if (!eventgenerator_isEventSent(&(positiveLimitSensorInst.eventGen))) {
                eventgenerator_updateEventStateMachine(&(positiveLimitSensorInst.eventGen));
                if (eventgenerator_isEventPending(&(positiveLimitSensorInst.eventGen)) && 
                (positiveLimitSensorInst.eventGen.priority < 0)) {
                    // this event has not yet been placed in the fifo
                    positiveLimitSensorInst.eventGen.priority = *fifoInsertId;
                    *fifoInsertId = ((*fifoInsertId)+1)&0xF;
                }
            }
        #endif

        #ifdef ENTITY_STEPPER1_NEGATIVELIMIT_BOUNDCHANNEL
            if (!eventgenerator_isEventSent(&(negativeLimitSensorInst.eventGen))) {
                eventgenerator_updateEventStateMachine(&(negativeLimitSensorInst.eventGen));
                if (eventgenerator_isEventPending(&(negativeLimitSensorInst.eventGen)) && 
                (negativeLimitSensorInst.eventGen.priority < 0)) {
                    // this event has not yet been placed in the fifo
                    negativeLimitSensorInst.eventGen.priority = *fifoInsertId;
                    *fifoInsertId = ((*fifoInsertId)+1)&0xF;
                }
            }
        #endif

        if (!eventgenerator_isEventSent(&(motionStateSensorInst.eventGen))) {
            eventgenerator_updateEventStateMachine(&(motionStateSensorInst.eventGen));
            if (eventgenerator_isEventPending(&(motionStateSensorInst.eventGen)) && 
            (motionStateSensorInst.eventGen.priority < 0)) {
                // this event has not yet been placed in the fifo
                motionStateSensorInst.eventGen.priority = *fifoInsertId;
                *fifoInsertId = ((*fifoInsertId)+1)&0xF;
            }
        }

        if (!eventgenerator_isEventSent(&(positionSensorInst.eventGen))) {
            eventgenerator_updateEventStateMachine(&(positionSensorInst.eventGen));
            if (eventgenerator_isEventPending(&(positionSensorInst.eventGen)) && 
            (positionSensorInst.eventGen.priority < 0)) {
                // this event has not yet been placed in the fifo
                positionSensorInst.eventGen.priority = *fifoInsertId;
                *fifoInsertId = ((*fifoInsertId)+1)&0xF;
            }
        }
    }

    //===============================================================
    // entityStepper1_acknowledgeEvent()
    //
    // this function acknowledges the event for each sensor that is
    // currently in the "SENT" state.
    void entityStepper1_acknowledgeEvent() {
        if (eventgenerator_isEventSent(&(globalInterlockSensorInst.eventGen))) {
            eventgenerator_acknowledge(&(globalInterlockSensorInst.eventGen));
        }
        
        if (eventgenerator_isEventSent(&(triggerSensorInst.eventGen))) {
            eventgenerator_acknowledge(&(triggerSensorInst.eventGen));
        }
        
       #ifdef ENTITY_STEPPER1_POSITIVELIMIT_BOUNDCHANNEL
            if (eventgenerator_isEventSent(&(positiveLimitSensorInst.eventGen))) {
                eventgenerator_acknowledge(&(positiveLimitSensorInst.eventGen));
            }
        #endif

        #ifdef ENTITY_STEPPER1_NEGATIVELIMIT_BOUNDCHANNEL
            if (eventgenerator_isEventSent(&(negativeLimitSensorInst.eventGen))) {
                eventgenerator_acknowledge(&(negativeLimitSensorInst.eventGen));
            }
        #endif

        if (eventgenerator_isEventSent(&(motionStateSensorInst.eventGen))) {
            eventgenerator_acknowledge(&(motionStateSensorInst.eventGen));
        }

        if (eventgenerator_isEventSent(&(positionSensorInst.eventGen))) {
            eventgenerator_acknowledge(&(positionSensorInst.eventGen));
        }
    }

    //===============================================================
    // entityStepper1_respondToPollEvent()
    //
    // this function responds to a poll event request by sending the 
    // event response from the proper sensor.
    void entityStepper1_respondToPollEvent(PldmRequestHeader *rxHeader, char fifoInsertId, char fifoExtractId) {
        unsigned char moreEvents = 1;
        if (((fifoExtractId+1)&0x0f)==fifoInsertId) moreEvents = 0;
        
        if (globalInterlockSensorInst.eventGen.priority==fifoExtractId) {
            eventgenerator_startSending(&(globalInterlockSensorInst.eventGen),rxHeader,moreEvents);
        }
        
        if (triggerSensorInst.eventGen.priority==fifoExtractId) {
            eventgenerator_startSending(&(triggerSensorInst.eventGen),rxHeader,moreEvents);
        }
        
        #ifdef ENTITY_STEPPER1_POSITIVELIMIT_BOUNDCHANNEL
            if (positiveLimitSensorInst.eventGen.priority==fifoExtractId) {
                eventgenerator_startSending(&(positiveLimitSensorInst.eventGen),rxHeader,moreEvents);
            }
        #endif

        #ifdef ENTITY_STEPPER1_NEGATIVELIMIT_BOUNDCHANNEL
            if (negativeLimitSensorInst.eventGen.priority==fifoExtractId) {
                eventgenerator_startSending(&(negativeLimitSensorInst.eventGen),rxHeader,moreEvents);
            }
        #endif

        if (motionStateSensorInst.eventGen.priority==fifoExtractId) {
            eventgenerator_startSending(&(motionStateSensorInst.eventGen),rxHeader,moreEvents);
        }

        if (positionSensorInst.eventGen.priority==fifoExtractId) {
            eventgenerator_startSending(&(positionSensorInst.eventGen),rxHeader,moreEvents);
        }
    }


    //*******************************************************************
    // setStateEfffecterStates()
    //
    // set the value of a numeric state effecter if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entityStepper1_setStateEffecterStates(PldmRequestHeader* rxHeader) {
        // extract the information from the body
        unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
        unsigned char effecter_count = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2);

        // if the user is trying to set more than one state, return with an error
        if (effecter_count != 1) return RESPONSE_INVALID_STATE_VALUE; 


        unsigned char action = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2 + 1);
        unsigned char req_state = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2 + 1 + 1);
        unsigned char response = RESPONSE_SUCCESS;
        switch (effecter_id) {
        #ifdef ENTITY_STEPPER1_COMMAND_EFFECTERID
        case ENTITY_STEPPER1_COMMAND_EFFECTERID:
            // only try to update the state if action is requestSet
            if (action) {
                if (!stateeffecter_setPresentState(&commandEffecterInst,req_state)) { 
                    response = RESPONSE_UNSUPPORTED_EFFECTERSTATE;
                }
            }
            break;
        #endif
        #ifdef ENTITY_STEPPER1_TRIGGEREFFECTER_EFFECTERID
            case ENTITY_STEPPER1_TRIGGEREFFECTER_EFFECTERID:
                // only try to update the state if action is requestSet
                if (action) {
                    if (!stateeffecter_setPresentState(&triggerEffecterInst,req_state)) { 
                        response = RESPONSE_UNSUPPORTED_EFFECTERSTATE;
                    }
                }
                break;
        #endif
        #ifdef ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_EFFECTERID
            case ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_EFFECTERID:
                // only try to update the state if action is requestSet
                if (action) {
                    if (!stateeffecter_setPresentState(&globalInterlockEffecterInst,req_state)) { 
                        response = RESPONSE_UNSUPPORTED_EFFECTERSTATE;
                    }
                }
                break;
        #endif
        default:
            response = RESPONSE_INVALID_EFFECTER_ID; 
            break;
        }
        return response;
    } 

    //*******************************************************************
    // entityStepper1_setStateEfffecterEnables()
    //
    // set the value of a state effecter enable if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entityStepper1_setStateEffecterEnables(PldmRequestHeader* rxHeader) {
        // extract the information from the body
        unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
        unsigned char effecter_count = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2);
        unsigned char effecter_op_state     = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+3);
        unsigned char effecter_event_enable = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+4);
        unsigned char response = RESPONSE_SUCCESS; 
        
        if (effecter_count != 1) return RESPONSE_INVALID_STATE_VALUE;
        if (effecter_op_state>2) return RESPONSE_INVALID_STATE_VALUE; 
        if ((effecter_event_enable==0)||(effecter_event_enable==1)) return RESPONSE_EVENT_GENERATION_NOT_SUPPORTED; 
        
        switch (effecter_id) {
        #ifdef ENTITY_STEPPER1_COMMAND_EFFECTERID
            case ENTITY_STEPPER1_COMMAND_EFFECTERID:
                if (!stateeffecter_setOperationalState(&commandEffecterInst, effecter_op_state)) { 
                    response = RESPONSE_UNSUPPORTED_EFFECTERSTATE;
                }
                break;
        #endif
        #ifdef ENTITY_STEPPER1_TRIGGEREFFECTER_EFFECTERID
            case ENTITY_STEPPER1_TRIGGEREFFECTER_EFFECTERID:
                if (!stateeffecter_setOperationalState(&triggerEffecterInst, effecter_op_state)) { 
                    response = RESPONSE_UNSUPPORTED_EFFECTERSTATE;
                }
                break;
        #endif
        #ifdef ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_EFFECTERID
            case ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_EFFECTERID:
                if (!stateeffecter_setOperationalState(&globalInterlockEffecterInst, effecter_op_state)) { 
                    response = RESPONSE_UNSUPPORTED_EFFECTERSTATE;
                }
                break;
        #endif
        default:
            response = RESPONSE_INVALID_EFFECTER_ID; 
            break;
        }
        return response;
    } 

    //*******************************************************************
    // entityStepper1_getStateSensorReading()
    //
    // get the value of a state sensor state if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entityStepper1_getStateSensorReading(PldmRequestHeader* rxHeader, unsigned char *responseBody, unsigned char *size) {
        // extract the information from the body
        unsigned int  sensor_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));

        // set a few default values
        unsigned char response = RESPONSE_SUCCESS; 
        responseBody[0] = 1;    // the number of sensor states
        *size = 4;              // the size of the body (not including the response code)

        switch (sensor_id) {
        #ifdef ENTITY_STEPPER1_MOTIONSTATE_SENSORID
            case ENTITY_STEPPER1_MOTIONSTATE_SENSORID:
                responseBody[1] = statesensor_getOperationalState(&motionStateSensorInst);   
                responseBody[2] = statesensor_getPresentState(&motionStateSensorInst);
                responseBody[3] = statesensor_getPresentState(&motionStateSensorInst);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_POSITIVELIMIT_SENSORID
            case ENTITY_STEPPER1_POSITIVELIMIT_SENSORID:
                responseBody[1] = statesensor_getOperationalState(&positiveLimitSensorInst);   
                responseBody[2] = statesensor_getPresentState(&positiveLimitSensorInst);
                responseBody[3] = statesensor_getPresentState(&positiveLimitSensorInst);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_NEGATIVELIMIT_SENSORID
            case ENTITY_STEPPER1_NEGATIVELIMIT_SENSORID:
                responseBody[1] = statesensor_getOperationalState(&negativeLimitSensorInst);   
                responseBody[2] = statesensor_getPresentState(&negativeLimitSensorInst);
                responseBody[3] = statesensor_getPresentState(&negativeLimitSensorInst);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_TRIGGERSENSOR_SENSORID
            case ENTITY_STEPPER1_TRIGGERSENSOR_SENSORID:
                responseBody[1] = statesensor_getOperationalState(&triggerSensorInst);   
                responseBody[2] = statesensor_getPresentState(&triggerSensorInst);
                responseBody[3] = statesensor_getPresentState(&triggerSensorInst);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_SENSORID
            case ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_SENSORID:
                responseBody[1] = statesensor_getOperationalState(&globalInterlockSensorInst);   
                responseBody[2] = statesensor_getPresentState(&globalInterlockSensorInst);
                responseBody[3] = statesensor_getPresentState(&globalInterlockSensorInst);
                break;
        #endif
        default:
            response = RESPONSE_INVALID_EFFECTER_ID; 
            *size = 0;
            break;
        }
        return response;
    } 

    //*******************************************************************
    // entityStepper1_setStateEfffecterEnables()
    //
    // set the value of a state effecter enable if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entityStepper1_setStateSensorEnables(PldmRequestHeader* rxHeader) {
        // extract the information from the body
        unsigned int  sensor_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
        unsigned char sensor_count = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2);
        unsigned char sensor_op_state     = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+3);
        unsigned char sensor_event_enable = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+4);
        unsigned char response = RESPONSE_SUCCESS; 
        
        if (sensor_count != 1) return RESPONSE_INVALID_STATE_VALUE;
        if (sensor_op_state>1) return RESPONSE_INVALID_STATE_VALUE; 
        if ((sensor_event_enable!=0)&&(sensor_event_enable!=1)&&(sensor_event_enable!=4)) return RESPONSE_EVENT_GENERATION_NOT_SUPPORTED; 
        
        switch (sensor_id) {
        #ifdef ENTITY_STEPPER1_MOTIONSTATE_SENSORID
            case ENTITY_STEPPER1_MOTIONSTATE_SENSORID:
                response = statesensor_setOperationalState(&motionStateSensorInst,sensor_op_state, sensor_event_enable);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_POSITIVELIMIT_SENSORID
            case ENTITY_STEPPER1_POSITIVELIMIT_SENSORID:
                response = statesensor_setOperationalState(&positiveLimitSensorInst,sensor_op_state, sensor_event_enable);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_NEGATIVELIMIT_SENSORID
            case ENTITY_STEPPER1_NEGATIVELIMIT_SENSORID:
                response = statesensor_setOperationalState(&negativeLimitSensorInst,sensor_op_state, sensor_event_enable);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_TRIGGERSENSOR_SENSORID
            case ENTITY_STEPPER1_TRIGGERSENSOR_SENSORID:
                response = statesensor_setOperationalState(&triggerSensorInst,sensor_op_state, sensor_event_enable);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_SENSORID
            case ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_SENSORID:
                response = statesensor_setOperationalState(&globalInterlockSensorInst,sensor_op_state, sensor_event_enable);
                break;
        #endif
        default:
            response = RESPONSE_INVALID_EFFECTER_ID; 
            break;
        }
        return response;
    } 

    //*******************************************************************
    // entityStepper1_getStateEfffecterStates()
    //
    // get the value of a numeric state effecter state if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entityStepper1_getStateEffecterStates(PldmRequestHeader* rxHeader, unsigned char *responseBody, unsigned char *size) {
        // extract the information from the body
        unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));

        // set a few default values;
        unsigned char response = RESPONSE_SUCCESS;
        *size = 4;              // size of the body (not including the response code)
        responseBody[0] = 1;    

        switch (effecter_id) {
        #ifdef ENTITY_STEPPER1_COMMAND_EFFECTERID
            case ENTITY_STEPPER1_COMMAND_EFFECTERID:
                responseBody[1] = stateeffecter_getOperationalState(&commandEffecterInst);   
                responseBody[2] = stateeffecter_getPresentState(&commandEffecterInst);
                responseBody[3] = stateeffecter_getPresentState(&commandEffecterInst);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_TRIGGEREFFECTER_EFFECTERID
            case ENTITY_STEPPER1_TRIGGEREFFECTER_EFFECTERID:
                responseBody[1] = stateeffecter_getOperationalState(&triggerEffecterInst);   
                responseBody[2] = stateeffecter_getPresentState(&triggerEffecterInst);
                responseBody[3] = stateeffecter_getPresentState(&triggerEffecterInst);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_EFFECTERID
            case ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_EFFECTERID:
                responseBody[1] = stateeffecter_getOperationalState(&globalInterlockEffecterInst);   
                responseBody[2] = stateeffecter_getPresentState(&globalInterlockEffecterInst);
                responseBody[3] = stateeffecter_getPresentState(&globalInterlockEffecterInst);
                break;
        #endif
        default:
            response = RESPONSE_INVALID_EFFECTER_ID;
            *size = 0; 
            break;
        }
        return response;
    } 

    //*******************************************************************
    // entityStepper1_setNumericEfffecterValue()
    //
    // set the value of a numeric state effecter if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entityStepper1_setNumericEffecterValue(PldmRequestHeader* rxHeader) {
        // extract the information from the body
        unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
        unsigned char effecter_numtype = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2);

        // set default valus
        unsigned char response = RESPONSE_SUCCESS; 

        if (effecter_numtype != SINT32_TYPE) return RESPONSE_ERROR_INVALID_DATA;
        FIXEDPOINT_24_8 newvalue = *((long*)(((char*)rxHeader) + sizeof(PldmRequestHeader)+2+1));

        switch (effecter_id) {
        #ifdef ENTITY_STEPPER1_APROFILE_EFFECTERID
            case ENTITY_STEPPER1_APROFILE_EFFECTERID:
                response = numericeffecter_setValue(&aprofileEffecterInst,newvalue);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_VPROFILE_EFFECTERID
            case ENTITY_STEPPER1_VPROFILE_EFFECTERID:
                response = numericeffecter_setValue(&vprofileEffecterInst,newvalue); 
                break;
        #endif
        #ifdef ENTITY_STEPPER1_PFINAL_EFFECTERID
            case ENTITY_STEPPER1_PFINAL_EFFECTERID:
                response = numericeffecter_setValue(&pfinalEffecterInst,newvalue); 
                break;
        #endif
        #ifdef ENTITY_STEPPER1_ACCELERATIONGAIN_EFFECTERID
            case ENTITY_STEPPER1_ACCELERATIONGAIN_EFFECTERID:
                response = numericeffecter_setValue(&accelerationGainEffecterInst,newvalue); 
                break;
        #endif
        default:
            response = RESPONSE_INVALID_EFFECTER_ID;
            break;
        }
        return response;
    }

    //*******************************************************************
    // entityStepper1_getNumericEfffecterValue()
    //
    // set the value of a numeric state effecter if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entityStepper1_getNumericEffecterValue(PldmRequestHeader* rxHeader, unsigned char *responseBody, unsigned char *size) {
        // extract the information from the body
        unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));

        // set some default values;
        unsigned char response = RESPONSE_SUCCESS;
        responseBody[0] = SINT32_TYPE;
        *size = 10;
    
        switch (effecter_id) {
        #ifdef ENTITY_STEPPER1_APROFILE_EFFECTERID
            case ENTITY_STEPPER1_APROFILE_EFFECTERID:
                responseBody[1] = numericeffecter_getOperationalState(&aprofileEffecterInst);
                *((FIXEDPOINT_24_8 *)&(responseBody[2])) = numericeffecter_getValue(&aprofileEffecterInst);
                *((FIXEDPOINT_24_8 *)&(responseBody[6])) = numericeffecter_getValue(&aprofileEffecterInst);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_VPROFILE_EFFECTERID
            case ENTITY_STEPPER1_VPROFILE_EFFECTERID:
                responseBody[1] = numericeffecter_getOperationalState(&vprofileEffecterInst);
                *((FIXEDPOINT_24_8*)&(responseBody[2])) = numericeffecter_getValue(&vprofileEffecterInst);
                *((FIXEDPOINT_24_8*)&(responseBody[6])) = numericeffecter_getValue(&vprofileEffecterInst);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_PFINAL_EFFECTERID
            case ENTITY_STEPPER1_PFINAL_EFFECTERID:
                responseBody[1] = numericeffecter_getOperationalState(&pfinalEffecterInst);
                *((FIXEDPOINT_24_8*)&(responseBody[2])) = numericeffecter_getValue(&pfinalEffecterInst);
                *((FIXEDPOINT_24_8*)&(responseBody[6])) = numericeffecter_getValue(&pfinalEffecterInst);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_ACCELERATIONGAIN_EFFECTERID
            case ENTITY_STEPPER1_ACCELERATIONGAIN_EFFECTERID:
                responseBody[1] = numericeffecter_getOperationalState(&acclerationGainEffecterInst)
                *((FIXEDPOINT_24_8*)&(responseBody[2])) = numericeffecter_getValue(&accelerationGainEffecterInst);
                *((FIXEDPOINT_24_8*)&(responseBody[6])) = numericeffecter_getValue(&accelerationGainEffecterInst);
                break;
        #endif
        default:
            response = RESPONSE_INVALID_EFFECTER_ID;
            *size = 0;
            break;
        }
        return response;
    }

    //*******************************************************************
    // entityStepper1_getSensorReading()
    //
    // return the value of a numeric sensor.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entityStepper1_getSensorReading(PldmRequestHeader* rxHeader, unsigned char *responseBody, unsigned char *size) {
        // extract the information from the body
        unsigned int  sensor_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
        unsigned char rearm = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)+2));

        // set some default values
        unsigned char response = RESPONSE_SUCCESS;
        *size = 10;
        responseBody[0] = SINT32_TYPE;

        switch (sensor_id) {
        #ifdef ENTITY_STEPPER1_POSITION_SENSORID
            case ENTITY_STEPPER1_POSITION_SENSORID:
                responseBody[1] = numericsensor_getOperationalState(&positionSensorInst);
                if (eventgenerator_isEnabled(&(positionSensorInst.eventGen))) responseBody[2] = 2;
                else responseBody[2] = 1;
                responseBody[3] = numericsensor_getPresentState(&positionSensorInst);
                responseBody[4] = numericsensor_getSensorPreviousState(&positionSensorInst);
                responseBody[5] = numericsensor_getEventState(&positionSensorInst);
                *((FIXEDPOINT_24_8*)&(responseBody[6])) = numericsensor_getValue(&positionSensorInst);
                
                // rearm the sensor if requested
                if (rearm) numericsensor_sensorRearm(&positionSensorInst);
                break;
        #endif
        default:
            response = RESPONSE_INVALID_SENSOR_ID;   // completion code
            *size = 0;
            break;
        }
        return response;
    }

    //*******************************************************************
    // entityStepper1_setNumericEffecterEnable()
    //
    // set the enable for a numeric effecter.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entityStepper1_setNumericEffecterEnable(PldmRequestHeader* rxHeader) {
        // extract the information from the body
        unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
        unsigned int enable_state = *((char*)(((char*)rxHeader)+sizeof(PldmRequestHeader)+2));
        
        if (enable_state>2) return RESPONSE_INVALID_STATE_VALUE; 

        // set some default values
        unsigned char response = RESPONSE_SUCCESS;

        switch (effecter_id) {
        #ifdef ENTITY_STEPPER1_APROFILE_EFFECTERID
            case ENTITY_STEPPER1_APROFILE_EFFECTERID:
                numericeffecter_setOperationalState(&aprofileEffecterInst,enable_state);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_VPROFILE_EFFECTERID
            case ENTITY_STEPPER1_VPROFILE_EFFECTERID:
                numericeffecter_setOperationalState(&vprofileEffecterInst,enable_state);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_PFINAL_EFFECTERID
            case ENTITY_STEPPER1_PFINAL_EFFECTERID:
                numericeffecter_setOperationalState(&pfinalEffecterInst,enable_state);
                break;
        #endif
        #ifdef ENTITY_STEPPER1_ACCELERATIONGAIN_EFFECTERID
            case ENTITY_STEPPER1_ACCELERATIONGAIN_EFFECTERID:
                numericeffecter_setOperationalState(&accelerationGainEffecterInst,enable_state);
                break;
        #endif
        default:
            response = RESPONSE_INVALID_EFFECTER_ID;   // completion code
            break;
        }
        return response;
    }

    //*******************************************************************
    // entityStepper1_setNumericEfffecterEnables()
    //
    // set the value of a state effecter enable if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entityStepper1_setNumericSensorEnable(PldmRequestHeader* rxHeader) {
        // extract the information from the body
        unsigned int  sensor_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
        unsigned char sensor_op_state     = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2);
        unsigned char sensor_event_enable = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+3);
        unsigned char response = RESPONSE_SUCCESS; 
        
        if (sensor_op_state>1) return RESPONSE_INVALID_STATE_VALUE; 
        if ((sensor_event_enable!=0)&&(sensor_event_enable!=1)&&(sensor_event_enable!=4)) return RESPONSE_EVENT_GENERATION_NOT_SUPPORTED; 
        
        switch (sensor_id) {
        #ifdef ENTITY_STEPPER1_POSITION_SENSORID
            case ENTITY_STEPPER1_POSITION_SENSORID:
                response = numericsensor_setOperationalState(&positionSensorInst,sensor_op_state, sensor_event_enable);
                break;
        #endif
        default:
            response = RESPONSE_INVALID_EFFECTER_ID; 
            break;
        }
        return response;
    } 



/////////////////////////////////////////////////////////////////////////////
// THIS IS WORK IN PROGRESS AND NEEDS TO BE CLEANED UP
/////////////////////////////////////////////////////////////////////////////

//****************************************************************
// this is the high-priority update loop for the servo motor
// control function - it runs in priority mode and should not
// rely on interrupt-updates to change variable states as interrupts
// are disabled when this function runs.
//
void entityStepper1_updateControl() {
    // output changes from previous interation
    #ifdef ENTITY_STEPPER1_OUTPUTENABLE
        // output the ENABLE
        if (outputEnableEffecterInst.state == outputEnableEffecterInst.stateWhenHigh) {
            PORTD |= (1<<PD5);
        }
        else {
            PORTD &= (~(1<<PD5));
        }
    #endif
    // update the global Iterlock Effecter output
    if (stateeffecter_isEnabled(&globalInterlockEffecterInst))
        CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_BOUNDCHANNEL,_enable());
    else
        CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_BOUNDCHANNEL,_disable());
    CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_GLOBALINTERLOCKEFFECTER_BOUNDCHANNEL,_setOutput(stateeffecter_getOutput(&globalInterlockEffecterInst)));
    // Update the trigger Effecter output
    if (stateeffecter_isEnabled(&triggerEffecterInst))
        CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_TRIGGEREFFECTER_BOUNDCHANNEL,_enable());
    else
        CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_TRIGGEREFFECTER_BOUNDCHANNEL,_disable());
    CALL_CHANNEL_FUNCTION(ENTITY_STEPPER1_TRIGGEREFFECTER_BOUNDCHANNEL,_setOutput(stateeffecter_getOutput(&triggerEffecterInst)));
    deltax_t1 = deltax_t0; 
    deltax_t0 = step_dir_out1_setOutput(current_velocity);
    
    // read new values for all the sensors
    entityStepper1_readChannels();

    // check to see if there was a requested state change
    unsigned char reqState = commandEffecterInst.state;
    if (reqState == 1) {  // run requested
        if ((state == STATE_IDLE)||(state==STATE_RUNNINGV)) {
            // run command is only valid from the idle or runningv states
            servo_cmd = MOTOR_CMD_RUN;
            servo_mode = 0;
        }
    } else if (reqState == 2) {  // stop requested 
        servo_cmd = MOTOR_CMD_STOP; 
    } else if (reqState == 3) { // wait requested
        if (state == STATE_IDLE) {
            // wait command is only valid from the idle state
            servo_cmd = MOTOR_CMD_RUN;
            servo_mode = MOTOR_MODE_WAIT;
        }    
    }
    commandEffecterInst.state = 0;  // unknown state
    
    //=======================================================
    // update flags based on current state of sensors
    // start by clearling all flags but the motion direction
    // this is set at the start of motion.
    servo_flags &= MOTOR_FLAGS_REVERSE;
    if ((statesensor_isEnabled(&globalInterlockSensorInst))&&
        (globalInterlockSensorInst.value == globalInterlockSensorInst.stateWhenLow)) {
            servo_flags |= MOTOR_FLAGS_INTERLOCK;
            servo_flags |= MOTOR_FLAGS_ERROR;
    }
    if ((statesensor_isEnabled(&triggerSensorInst))&&
        (triggerSensorInst.value == triggerSensorInst.stateWhenLow)) servo_flags |= MOTOR_FLAGS_TRIGGER;
    #ifdef ENTITY_STEPPER1_POSITIVELIMIT
        if ((statesensor_isEnabled(&positiveLimitSensorInst) == 1)&&
            (positiveLimitSensorInst.value == SWITCH_STATE_PRESSED_ON)) 
            servo_flags |= MOTOR_FLAGS_POSLIMIT;
    #endif
    #ifdef ENTITY_STEPPER1_NEGATIVELIMIT
        if (statesensor_isEnabled(&negativeLimitSensorInst)&&
            (negativeLimitSensorInst.value == SWITCH_STATE_PRESSED_ON)) 
            servo_flags |= MOTOR_FLAGS_NEGLIMIT;
    #endif
    #ifdef ENTITY_STEPPER1_POSITION
        if (
                (numericsensor_isEnabled(&positionSensorInst)) &&
                (
                    (numericsensor_isCritical(&positionSensorInst)) ||
                    (numericsensor_isFatal(&positionSensorInst))
                )
            ) { 
            servo_flags |= MOTOR_FLAGS_ERROR;
        }
    #endif
    #ifdef ENTITY_STEPPER1_POSITION
        if (
                (numericsensor_isEnabled(&positionSensorInst)) &&
                    (numericsensor_isWarning(&positionSensorInst))
                ) {
            servo_flags |= MOTOR_FLAGS_WARNING;
        }
    #endif

    //=============================================================
    // update the state machine
    switch (state) {
    case STATE_IDLE:
        if (servo_flags & MOTOR_FLAGS_ERROR) {
            // perform actions for entry to error state
            // error condition has priority over any other state
            // transistion

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif

            state = STATE_ERROR;
        }
        else if ((servo_cmd == MOTOR_CMD_RUN)&&(servo_mode != MOTOR_MODE_NOWAIT)) {
            // check to see if all the required effecters are enabled
            if (!numericeffecter_isEnabled(&vprofileEffecterInst)) break;
            if (!numericeffecter_isEnabled(&aprofileEffecterInst)) break;
            
            // different profile behavior based on whether position effecter is enabled
            servo_flags = 0;
            if (numericeffecter_isEnabled(&pfinalEffecterInst)) {                
                // position/velocity motion
                // set the motion parameters to the most recently requested
                long requested_deltax = pfinalEffecterInst.value - positionSensorInst.value;
                if (requested_deltax<0) servo_flags |= MOTOR_FLAGS_REVERSE;
                vprofiler_setParameters(requested_deltax, vprofileEffecterInst.value, aprofileEffecterInst.value, mode_scurve);
            } else {
                // velocity only move
                if (current_velocity<0) servo_flags |= MOTOR_FLAGS_REVERSE;
                vprofiler_setParameters(vprofileEffecterInst.value, vprofileEffecterInst.value, aprofileEffecterInst.value, mode_scurve);
            }
            // transition to the waiting state
            state = STATE_WAITING;
        }
        else if ((servo_cmd == MOTOR_CMD_RUN) && (servo_mode == MOTOR_MODE_NOWAIT)) {
            // check to see if all the required effecters are enabled
            if (!numericeffecter_isEnabled(&vprofileEffecterInst)) break;
            if (!numericeffecter_isEnabled(&aprofileEffecterInst)) break;

            // disable the brake if it is set
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenLow);
            #endif

            // enable the motor if it is disabled
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenHigh);
            #endif

            // different profile behavior based on whether position effecter is enabled
            servo_flags = 0;
            if (numericeffecter_isEnabled(&pfinalEffecterInst)) {                
                // position/velocity motion
                // set the motion parameters to the most recently requested and
                long requested_deltax = pfinalEffecterInst.value - positionSensorInst.value;
                if (requested_deltax<0) servo_flags |= MOTOR_FLAGS_REVERSE;
                vprofiler_setParameters(requested_deltax, vprofileEffecterInst.value, aprofileEffecterInst.value, mode_scurve);
                
                // start the velocity profiler
                vprofiler_start();

                // transition to the running state
                state = STATE_RUNNING;
            } else {
                servo_flags |= MOTOR_FLAGS_VMODE;
                if (current_velocity<0) servo_flags |= MOTOR_FLAGS_REVERSE;
                vprofiler_setParameters(vprofileEffecterInst.value, vprofileEffecterInst.value, aprofileEffecterInst.value, mode_scurve);

                // start the velocity profiler
                vprofiler_startv();

                // transition to the runningv state
                state = STATE_RUNNINGV;
            }
        }
        break;
    case STATE_RUNNING:
        // update the velocity profiler position - running is the only mode
        // in which this happens
        vprofiler_update();
        if (servo_flags & MOTOR_FLAGS_ERROR) {
            // perform actions for entry to error state
            // error condition has priority over any other state
            // transistion

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif

            state = STATE_ERROR;
        }
        else if ((servo_flags & MOTOR_FLAGS_TRIGGER ) || 
            ((servo_flags & MOTOR_FLAGS_NEGLIMIT) && (servo_flags & MOTOR_FLAGS_REVERSE)) ||
            ((servo_flags & MOTOR_FLAGS_POSLIMIT) && ((servo_flags & MOTOR_FLAGS_REVERSE)==0))
            )
        {
            // perform actions for entry to condition stop state
            // warning conition has priority over all but error
            // transition

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINCONDITIONSTOP_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINCONDITIONSTOP_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif

            state = STATE_COND;
        }
        else if (servo_cmd == MOTOR_CMD_STOP) {
            // transition to the stopping state
            state = STATE_STOPPING;
        } 
        else if (vprofiler_isDone()) {
            // set the motion parameters to the done settings (follow/coast/brake)

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINDONE_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINDONE_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif

            // transition to the done state
            state = STATE_DONE;
        }
        break;
    case STATE_RUNNINGV:
        // update the velocity profiler position - running is the only mode
        // in which this happens
        vprofiler_updatev();
        servo_flags &= (~MOTOR_FLAGS_REVERSE);
        if (current_velocity<0) servo_flags |= MOTOR_FLAGS_REVERSE;
        if (servo_flags & MOTOR_FLAGS_ERROR) {
            // perform actions for entry to error state
            // error condition has priority over any other state
            // transistion

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif

            state = STATE_ERROR;
        }
        else if ((servo_flags & MOTOR_FLAGS_TRIGGER ) || 
            ((servo_flags & MOTOR_FLAGS_NEGLIMIT) && (servo_flags & MOTOR_FLAGS_REVERSE)) ||
            ((servo_flags & MOTOR_FLAGS_POSLIMIT) && ((servo_flags & MOTOR_FLAGS_REVERSE)==0))
            )
        {
            // perform actions for entry to condition stop state
            // warning conition has priority over all but error
            // transition

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINCONDITIONSTOP_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINCONDITIONSTOP_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif

            state = STATE_COND;
        }
        else if (servo_cmd == MOTOR_CMD_STOP) {
            // transition to the idle state
            state = STATE_STOPPINGV;
        } else if (servo_cmd == MOTOR_CMD_RUN) {
            // request to slew to a new velocity

            // check to see if all the required effecters are enabled
            if (!numericeffecter_isEnabled(&vprofileEffecterInst)) break;
            if (!numericeffecter_isEnabled(&aprofileEffecterInst)) break;

            servo_flags = 0;
            servo_flags |= MOTOR_FLAGS_VMODE;
            if (current_velocity<0) servo_flags |= MOTOR_FLAGS_REVERSE;
            vprofiler_setParameters(vprofileEffecterInst.value, vprofileEffecterInst.value, aprofileEffecterInst.value, mode_scurve);

            // start the velocity profiler
            vprofiler_startv();
        }
        break;
    case STATE_STOPPING:
    case STATE_STOPPINGV:
        if (servo_flags & MOTOR_FLAGS_ERROR) {
            // perform actions for entry to error state
            // error condition has priority over any other state
            // transistion

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif

            state = STATE_ERROR;
        }
        else if ((servo_flags & MOTOR_FLAGS_TRIGGER ) || 
            ((servo_flags & MOTOR_FLAGS_NEGLIMIT) && (servo_flags & MOTOR_FLAGS_REVERSE)) ||
            ((servo_flags & MOTOR_FLAGS_POSLIMIT) && ((servo_flags & MOTOR_FLAGS_REVERSE)==0))
            )
        {
            // perform actions for entry to condition stop state
            // warning conition has priority over all but error
            // transition

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINCONDITIONSTOP_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINCONDITIONSTOP_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif

            state = STATE_COND;
        }
        else {
            // reduce the velocity by the requested acceleration until it reaches 0
            if (current_velocity>0) {
                current_velocity -= (aprofileEffecterInst.value<0)?-aprofileEffecterInst.value:aprofileEffecterInst.value;
                if (current_velocity<0) current_velocity = 0;
            } else if (current_velocity<0) {
                current_velocity += (aprofileEffecterInst.value<0)?-aprofileEffecterInst.value:aprofileEffecterInst.value;
                if (current_velocity>0) current_velocity = 0;        
            } else if (current_velocity==0) {
                // velocity is zero - transition to IDLE state.
                // turn on the brake if required
                #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                    #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINIDLE_BRAKE
                        stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                    #endif
                #endif

                // disable the motor required
                #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                    #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINIDLE_COAST
                        stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                    #endif
                #endif
                state = STATE_IDLE;
            }
            servo_flags &= (~MOTOR_FLAGS_REVERSE);
            if (current_velocity<0) servo_flags |= MOTOR_FLAGS_REVERSE;
        }
        break;
    case STATE_WAITING:
        if (servo_flags & MOTOR_FLAGS_ERROR) {
            // perform actions for entry to error state
            // error condition has priority over any other state
            // transistion

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif

            state = STATE_ERROR;
        }
        else if (!(servo_flags & MOTOR_FLAGS_TRIGGER )) {
            // when waiting, transition to running mode on negative
            // edge of global trigger 

            // disable the brake if it is set
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenLow);
            #endif

            // enable the motor if it is disabled
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenHigh);
            #endif

            if (servo_flags & MOTOR_FLAGS_VMODE) {
                vprofiler_startv();
                state = STATE_RUNNINGV;
            } else {    
                vprofiler_start();
                state = STATE_RUNNING;
            }
        }
        else if (servo_cmd == MOTOR_CMD_STOP) {
            // set the motion parameters to idle settings (follow/coast/brake)

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINIDLE_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINIDLE_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif
            
            // transition to the idle state
            state = STATE_IDLE;
        } 
        break;
    case STATE_DONE:
        if (servo_flags & MOTOR_FLAGS_ERROR) {
            // perform actions for entry to error state
            // error condition has priority over any other state
            // transistion

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif

            state = STATE_ERROR;
        }
        else if (servo_flags & MOTOR_FLAGS_TRIGGER ) {
            // perform actions for entry to warnding state
            // warning conition has priority over all but error
            // transition

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINCONDITIONSTOP_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINCONDITIONSTOP_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif
            state = STATE_COND;
        }
        else if (servo_cmd == MOTOR_CMD_STOP) {
            // set the motion parameters to idle settings (follow/coast/brake)

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINIDLE_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINIDLE_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif
            
            // transition to the idle state
            state = STATE_IDLE;
        } 
    case STATE_ERROR:
        if (servo_cmd == MOTOR_CMD_STOP) {
            // set the motion parameters to idle settings (follow/coast/brake)

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINIDLE_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINIDLE_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif
            
            // transition to the idle state
            state = STATE_IDLE;
        } 
        break;
    case STATE_COND:
        if (servo_flags & MOTOR_FLAGS_ERROR) {
            // perform actions for entry to error state
            // error condition has priority over any other state
            // transistion

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINERRORSTOP_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif
            state = STATE_ERROR;
        }
        else if (servo_cmd == MOTOR_CMD_STOP) {
            // set the motion parameters to idle settings (follow/coast/brake)

            // turn on the brake if required
            #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINIDLE_BRAKE
                    stateeffecter_setPresentState(&brakeEffecterInst, brakeEffecterInst.stateWhenHigh);
                #endif
            #endif

            // disable the motor required
            #ifdef ENTITY_STEPPER1_OUTPUTENABLE
                #ifdef ENTITY_STEPPER1_PARAM_OUTPUTINIDLE_COAST
                    stateeffecter_setPresentState(&outputEnableEffecterInst, outputEnableEffecterInst.stateWhenLow);
                #endif
            #endif
            
            // transition to the idle state
            state = STATE_IDLE;
        } 
        break;
    default:
        // this case should never be reached - transition to ERROR_STOP
        state = STATE_ERROR;
    }
    servo_cmd = MOTOR_CMD_NONE; 
    motionStateSensorInst.value = state&0xF;      
}

#endif // ENTITY_STEPPER1
