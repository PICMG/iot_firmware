//    EntitySimple1.c
//
//    This header file defines functions related to the the Simple1  
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
#include "pdrdata.h"

#ifdef ENTITY_SIMPLE1
    #include "StateSensor.h"
    #include "NumericSensor.h"
    #include "StateEffecter.h"
    #include "NumericEffecter.h"
    #include "channels.h"
    #include "adc.h"
    #include "node.h"
    #include "vprofiler.h"
    #include "stepdir_out.h"
    #include "interpolator.h"

    #define SINT32_TYPE 5

    #define INNERCAT(x, y) x##y
    #define CONCATENATE(x, y) INNERCAT(x, y)
    #define CALL_CHANNEL_FUNCTION(channel, function) CONCATENATE(channel, function)

    #define SWITCH_STATE_PRESSED_ON   0x01
    #define SWITCH_STATE_RELEASED_OFF 0x02

    //===============================================================
    // Global variables related to effecters
    //===============================================================
    static StateEffecterInstance globalInterlockEffecterInst; 
    static StateEffecterInstance triggerEffecterInst;
    #ifdef ENTITY_SIMPLE1_EFFECTER1
        static StateEffecterInstance effecter1EffecterInst;
    #endif 
    #ifdef ENTITY_SIMPLE1_EFFECTER2
        static StateEffecterInstance effecter2EffecterInst;
    #endif 
    
#pragma GCC push_options
#pragma GCC optimize "-O3"
    //===============================================================
    // Sensor-Specific Code
    //===============================================================
    static StateSensorInstance globalInterlockSensorInst; 
    static void globalInterlockSensor_sendEvent()
    {
        node_sendStateSensorEvent(&(globalInterlockSensorInst.eventGen),
            ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_SENSORID,
            statesensor_getSensorPreviousState(&globalInterlockSensorInst));
    }

    static StateSensorInstance triggerSensorInst; 
    static void triggerSensor_sendEvent()
    {
        node_sendStateSensorEvent(&(triggerSensorInst.eventGen),
            ENTITY_SIMPLE1_TRIGGERSENSOR_SENSORID,
            statesensor_getSensorPreviousState(&triggerSensorInst));
    }

    // numeric sensor
    #ifdef ENTITY_SIMPLE1_SENSOR1
        static NumericSensorInstance sensor1SensorInst;
        static void sensor1Sensor_sendEvent()
        {
            node_sendNumericSensorEvent(&(sensor1SensorInst.eventGen),
                ENTITY_SIMPLE1_SENSOR1_SENSORID,
                numericsensor_getSensorPreviousState(&sensor1SensorInst),
                sensor1SensorInst.value
            );
        } 
    #endif 

    // state sensor
    #ifdef ENTITY_SIMPLE1_SENSOR2
        static StateSensorInstance sensor2SensorInst; 
        static void sensor2Sensor_sendEvent()
        {
            node_sendStateSensorEvent(&(sensor2SensorInst.eventGen),
                ENTITY_SIMPLE1_SENSOR2_SENSORID,
                statesensor_getSensorPreviousState(&sensor2SensorInst)
            );
        } 
    #endif


    //===============================================================
    // entitySimple1_readChannels()
    //
    // this function causes each channel used by the logical entity to 
    // read its current value and store it in it's channel data.
    void entitySimple1_readChannels() {
        // read the globalInterlockSensor's channel
        CALL_CHANNEL_FUNCTION(ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_BOUNDCHANNEL,_sample());
        statesensor_setValueFromChannelBit(&globalInterlockSensorInst,
            CALL_CHANNEL_FUNCTION(ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_BOUNDCHANNEL,_getRawData())
        );
        // read the triggerSensor' channel
        CALL_CHANNEL_FUNCTION(ENTITY_SIMPLE1_TRIGGERSENSOR_BOUNDCHANNEL,_sample());
        statesensor_setValueFromChannelBit(&triggerSensorInst,
            CALL_CHANNEL_FUNCTION(ENTITY_SIMPLE1_TRIGGERSENSOR_BOUNDCHANNEL,_getRawData())
        );
        
        #ifdef ENTITY_SIMPLE1_SENSOR1_BOUNDCHANNEL
            // read the numeric sensor's channel
            CALL_CHANNEL_FUNCTION(ENTITY_SIMPLE1_SENSOR1_BOUNDCHANNEL,_sample());
            numericsensor_setValue(&sensor1SensorInst, interpolator_linearize(
                CALL_CHANNEL_FUNCTION(ENTITY_SIMPLE1_SENSOR1_BOUNDCHANNEL,_getRawData()),
                CONCATENATE(__lintable_, ENTITY_SIMPLE1_SENSOR1_BOUNDCHANNEL),
                ENTITY_SIMPLE1_SENSOR1_BOUNDCHANNEL_PRECISION            
            ));
            numericsensor_setOperationalState(&sensor1SensorInst,
                sensor1SensorInst.value,eventgenerator_isEnabled(&(sensor1SensorInst.eventGen))
            );
        #endif

        #ifdef ENTITY_SIMPLE1_SENSOR2_BOUNDCHANNEL
            // read the state sensor's channel
            CALL_CHANNEL_FUNCTION(ENTITY_SIMPLE1_SENSOR2_BOUNDCHANNEL,_sample());
            statesensor_setValueFromChannelBit(&sensor2SensorInst,
                CALL_CHANNEL_FUNCTION(ENTITY_SIMPLE1_SENSOR2_BOUNDCHANNEL,_getRawData())
            );
        #endif
    }
#pragma GCC pop_options

    //===============================================================
    // entitySimple1_init()
    //
    // initialize all the sensors and effecters associated with the
    // Simple1 logical entity.  Some sensors/effecters are always 
    // present, others are present based on the firmware configuration.
    // This function uses firmware configuration macros to switch
    // in the proper sensors and effecters for the firmware build.
    //
    // parameters: none
    // returns: nothing
    void entitySimple1_init() 
    {
        // initilize the globalInterlockSensor
        statesensor_init(&globalInterlockSensorInst);
        globalInterlockSensorInst.stateWhenHigh = ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_STATEWHENHIGH;
        globalInterlockSensorInst.stateWhenLow = ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_STATEWHENLOW;
        globalInterlockSensorInst.eventGen.sendEvent = globalInterlockSensor_sendEvent;
 
        // initialize the triggerSensor
        statesensor_init(&triggerSensorInst);
        triggerSensorInst.stateWhenHigh = ENTITY_SIMPLE1_TRIGGERSENSOR_STATEWHENHIGH;
        triggerSensorInst.stateWhenLow = ENTITY_SIMPLE1_TRIGGERSENSOR_STATEWHENLOW;
        triggerSensorInst.eventGen.sendEvent = triggerSensor_sendEvent;

        // numeric sensor
        #ifdef ENTITY_SIMPLE1_SENSOR1 
            numericsensor_init(&sensor1SensorInst);
            sensor1SensorInst.thresholdEnables = ENTITY_SIMPLE1_SENSOR1_ENABLEDTHRESHOLDS; 
            numericsensor_setThresholds(&sensor1SensorInst,
                ENTITY_SIMPLE1_SENSOR1_UPPERTHRESHOLDFATAL,
                ENTITY_SIMPLE1_SENSOR1_UPPERTHRESHOLDCRITICAL,
                ENTITY_SIMPLE1_SENSOR1_UPPERTHRESHOLDWARNING,
                ENTITY_SIMPLE1_SENSOR1_LOWERTHRESHOLDWARNING,
                ENTITY_SIMPLE1_SENSOR1_LOWERTHRESHOLDCRITICAL,
                ENTITY_SIMPLE1_SENSOR1_LOWERTHRESHOLDFATAL);
            sensor1SensorInst.eventGen.sendEvent = sensor1Sensor_sendEvent;
        #endif

        // state sensor
        #ifdef ENTITY_SIMPLE1_SENSOR2
            statesensor_init(&sensor2SensorInst);
            sensor2SensorInst.stateWhenHigh = ENTITY_SIMPLE1_SENSOR2_STATEWHENHIGH;
            sensor2SensorInst.stateWhenLow  = ENTITY_SIMPLE1_SENSOR2_STATEWHENLOW;
            sensor2SensorInst.eventGen.sendEvent = sensor2Sensor_sendEvent;
        #endif

        // initialize the global interlock effecter
        stateeffecter_init(&globalInterlockEffecterInst); 
        globalInterlockEffecterInst.stateWhenHigh = ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_STATEWHENHIGH;
        globalInterlockEffecterInst.stateWhenLow = ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_STATEWHENLOW;
        globalInterlockEffecterInst.allowedStatesMask = 
            (1<<(ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_STATEWHENHIGH-1))|
            (1<<(ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_STATEWHENLOW-1));
        globalInterlockEffecterInst.defaultState = globalInterlockEffecterInst.stateWhenHigh;

        // initialize the trigger effecter
        stateeffecter_init(&triggerEffecterInst);
        triggerEffecterInst.stateWhenHigh = ENTITY_SIMPLE1_TRIGGEREFFECTER_STATEWHENHIGH;
        triggerEffecterInst.stateWhenLow = ENTITY_SIMPLE1_TRIGGEREFFECTER_STATEWHENLOW;
        triggerEffecterInst.allowedStatesMask = 
            (1<<(ENTITY_SIMPLE1_TRIGGEREFFECTER_STATEWHENHIGH-1))|
            (1<<(ENTITY_SIMPLE1_TRIGGEREFFECTER_STATEWHENLOW-1));
        triggerEffecterInst.defaultState = triggerEffecterInst.stateWhenHigh;

        // numeric effecter
        #ifdef ENTITY_SIMPLE1_EFFECTER1
            numericeffecter_init(&effecter1EffecterInst);
            effecter1EffecterInst.maxSettable = 0x7FFFFFFF;
            effecter1EffecterInst.minSettable = -0x7FFFFFFF;
            effecter1EffecterInst.value = ENTITY_STEPPER1_EFFECTER1_DEFAULTVALUE;
            effecter1EffecterInst.defaultValue = ENTITY_STEPPER1_EFFECTER1_DEFAULTVALUE;
        #endif 

        // state effecter
        #ifdef ENTITY_SIMPLE1_EFFECTER2
            stateeffecter_init(&effecter2EffecterInst);
            effecter2EffecterInst.allowedStatesMask = 3;
            effecter2EffecterInst.stateWhenHigh = 2;
            effecter2EffecterInst.stateWhenLow = 1;
            effecter2EffecterInst.defaultState = 1;
        #endif 
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
    unsigned char entitySimple1_setStateEffecterStates(PldmRequestHeader* rxHeader) {
        // extract the information from the body
        unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
        unsigned char effecter_count = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2);

        // if the user is trying to set more than one state, return with an error
        if (effecter_count != 1) return RESPONSE_INVALID_STATE_VALUE; 

        unsigned char action = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2 + 1);
        unsigned char req_state = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2 + 1 + 1);
        unsigned char response = RESPONSE_SUCCESS;

        switch (effecter_id) {
        #ifdef ENTITY_SIMPLE1_TRIGGEREFFECTER_EFFECTERID
            case ENTITY_SIMPLE1_TRIGGEREFFECTER_EFFECTERID:
                // only try to update the state if action is requestSet
                if (action) {
                    if (!stateeffecter_setPresentState(&triggerEffecterInst,req_state)) { 
                        response = RESPONSE_UNSUPPORTED_EFFECTERSTATE;
                    }
                }
                break;
        #endif
        #ifdef ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_EFFECTERID
            case ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_EFFECTERID:
                // only try to update the state if action is requestSet
                if (action) {
                    if (!stateeffecter_setPresentState(&globalInterlockEffecterInst,req_state)) { 
                        response = RESPONSE_UNSUPPORTED_EFFECTERSTATE;
                    }
                }
                break;
        #endif
        #ifdef ENTITY_SIMPLE1_EFFECTER2_EFFECTERID
        case ENTITY_SIMPLE1_EFFECTER2_EFFECTERID:
            // only try to update the state if action is requestSet
            if (action) {
                if (!stateeffecter_setPresentState(&effecter2EffecterInst,req_state)) { 
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
    // entitySimple1_setStateEfffecterEnables()
    //
    // set the value of a state effecter enable if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entitySimple1_setStateEffecterEnables(PldmRequestHeader* rxHeader) {
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
        #ifdef ENTITY_SIMPLE1_TRIGGEREFFECTER_EFFECTERID
            case ENTITY_SIMPLE1_TRIGGEREFFECTER_EFFECTERID:
                if (!stateeffecter_setOperationalState(&triggerEffecterInst, effecter_op_state)) { 
                    response = RESPONSE_UNSUPPORTED_EFFECTERSTATE;
                }
                break;
        #endif
        #ifdef ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_EFFECTERID
            case ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_EFFECTERID:
                if (!stateeffecter_setOperationalState(&globalInterlockEffecterInst, effecter_op_state)) { 
                    response = RESPONSE_UNSUPPORTED_EFFECTERSTATE;
                }
                break;
        #endif
        #ifdef ENTITY_SIMPLE1_EFFECTER2_EFFECTERID
            case ENTITY_SIMPLE1_EFFECTER2_EFFECTERID:
                if (!stateeffecter_setOperationalState(&effecter2EffecterInst, effecter_op_state)) { 
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
    // entitySimple1_getStateSensorReading()
    //
    // get the value of a state sensor state if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entitySimple1_getStateSensorReading(PldmRequestHeader* rxHeader, unsigned char *responseBody, unsigned char *size) {
        // extract the information from the body
        unsigned int  sensor_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));

        // set a few default values
        unsigned char response = RESPONSE_SUCCESS; 
        responseBody[0] = 1;    // the number of sensor states
        *size = 4;              // the size of the body (not including the response code)

        switch (sensor_id) {
        #ifdef ENTITY_SIMPLE1_SENSOR2_SENSORID
            case ENTITY_SIMPLE1_SENSOR2_SENSORID:
                responseBody[1] = statesensor_getOperationalState(&sensor2SensorInst);   
                responseBody[2] = statesensor_getPresentState(&sensor2SensorInst);
                responseBody[3] = statesensor_getPresentState(&sensor2SensorInst);
                break;
        #endif
        #ifdef ENTITY_SIMPLE1_TRIGGERSENSOR_SENSORID
            case ENTITY_SIMPLE1_TRIGGERSENSOR_SENSORID:
                responseBody[1] = statesensor_getOperationalState(&triggerSensorInst);   
                responseBody[2] = statesensor_getPresentState(&triggerSensorInst);
                responseBody[3] = statesensor_getPresentState(&triggerSensorInst);
                break;
        #endif
        #ifdef ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_SENSORID
            case ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_SENSORID:
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
    // entitySimple1_setStateEfffecterEnables()
    //
    // set the value of a state effecter enable if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entitySimple1_setStateSensorEnables(PldmRequestHeader* rxHeader) {
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
        #ifdef ENTITY_SIMPLE1_SENSOR2_SENSORID
            case ENTITY_SIMPLE1_SENSOR2_SENSORID:
                response = statesensor_setOperationalState(&sensor2SensorInst,sensor_op_state, sensor_event_enable);
                break;
        #endif
        #ifdef ENTITY_SIMPLE1_TRIGGERSENSOR_SENSORID
            case ENTITY_SIMPLE1_TRIGGERSENSOR_SENSORID:
                response = statesensor_setOperationalState(&triggerSensorInst,sensor_op_state, sensor_event_enable);
                break;
        #endif
        #ifdef ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_SENSORID
            case ENTITY_SIMPLE1_GLOBALINTERLOCKSENSOR_SENSORID:
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
    // entitySimple1_getStateEfffecterStates()
    //
    // get the value of a numeric state effecter state if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entitySimple1_getStateEffecterStates(PldmRequestHeader* rxHeader, unsigned char *responseBody, unsigned char *size) {
        // extract the information from the body
        unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));

        // set a few default values;
        unsigned char response = RESPONSE_SUCCESS;
        *size = 4;              // size of the body (not including the response code)
        responseBody[0] = 1;    

        switch (effecter_id) {
        #ifdef ENTITY_SIMPLE1_EFFECTER2_EFFECTERID
            case ENTITY_SIMPLE1_EFFECTER2_EFFECTERID:
                responseBody[1] = stateeffecter_getOperationalState(&effecter2EffecterInst);   
                responseBody[2] = stateeffecter_getPresentState(&effecter2EffecterInst);
                responseBody[3] = stateeffecter_getPresentState(&effecter2EffecterInst);
                break;
        #endif
        #ifdef ENTITY_SIMPLE1_TRIGGEREFFECTER_EFFECTERID
            case ENTITY_SIMPLE1_TRIGGEREFFECTER_EFFECTERID:
                responseBody[1] = stateeffecter_getOperationalState(&triggerEffecterInst);   
                responseBody[2] = stateeffecter_getPresentState(&triggerEffecterInst);
                responseBody[3] = stateeffecter_getPresentState(&triggerEffecterInst);
                break;
        #endif
        #ifdef ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_EFFECTERID
            case ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_EFFECTERID:
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
    // entitySimple1_setNumericEfffecterValue()
    //
    // set the value of a numeric state effecter if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entitySimple1_setNumericEffecterValue(PldmRequestHeader* rxHeader) {
        // extract the information from the body
        unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
        unsigned char effecter_numtype = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2);

        // set default valus
        unsigned char response = RESPONSE_SUCCESS; 

        if (effecter_numtype != SINT32_TYPE) return RESPONSE_ERROR_INVALID_DATA;
        FIXEDPOINT_24_8 newvalue = *((long*)(((char*)rxHeader) + sizeof(PldmRequestHeader)+2+1));

        switch (effecter_id) {
        #ifdef ENTITY_SIMPLE1_EFFECTER1_EFFECTERID
            case ENTITY_SIMPLE1_EFFECTER1_EFFECTERID:
                response = numericeffecter_setValue(&effecter1EffecterInst,newvalue);
                break;
        #endif
        default:
            response = RESPONSE_INVALID_EFFECTER_ID;
            break;
        }
        return response;
    }

    //*******************************************************************
    // entitySimple1_getNumericEfffecterValue()
    //
    // set the value of a numeric state effecter if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entitySimple1_getNumericEffecterValue(PldmRequestHeader* rxHeader, unsigned char *responseBody, unsigned char *size) {
        // extract the information from the body
        unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));

        // set some default values;
        unsigned char response = RESPONSE_SUCCESS;
        responseBody[0] = SINT32_TYPE;
        *size = 10;
    
        switch (effecter_id) {
        #ifdef ENTITY_SIMPLE1_EFFECTER1_EFFECTERID
            case ENTITY_SIMPLE1_EFFECTER1_EFFECTERID:
                responseBody[1] = numericeffecter_getOperationalState(&effecter1EffecterInst);
                *((FIXEDPOINT_24_8 *)&(responseBody[2])) = numericeffecter_getValue(&effecter1EffecterInst);
                *((FIXEDPOINT_24_8 *)&(responseBody[6])) = numericeffecter_getValue(&effecter1EffecterInst);
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
    // entitySimple1_getSensorReading()
    //
    // return the value of a numeric sensor.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entitySimple1_getSensorReading(PldmRequestHeader* rxHeader, unsigned char *responseBody, unsigned char *size) {
        // extract the information from the body
        unsigned int  sensor_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
        unsigned char rearm = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)+2));

        // set some default values
        unsigned char response = RESPONSE_SUCCESS;
        *size = 10;
        responseBody[0] = SINT32_TYPE;

        switch (sensor_id) {
        #ifdef ENTITY_SIMPLE1_SENSOR1_SENSORID
            case ENTITY_SIMPLE1_SENSOR1_SENSORID:
                responseBody[1] = numericsensor_getOperationalState(&sensor1SensorInst);
                if (eventgenerator_isEnabled(&(sensor1SensorInst.eventGen))) responseBody[2] = 2;
                else responseBody[2] = 1;
                responseBody[3] = numericsensor_getPresentState(&sensor1SensorInst);
                responseBody[4] = numericsensor_getSensorPreviousState(&sensor1SensorInst);
                responseBody[5] = numericsensor_getEventState(&sensor1SensorInst);
                *((FIXEDPOINT_24_8*)&(responseBody[6])) = numericsensor_getValue(&sensor1SensorInst);
                
                // rearm the sensor if requested
                if (rearm) numericsensor_sensorRearm(&sensor1SensorInst);
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
    // entitySimple1_setNumericEffecterEnable()
    //
    // set the enable for a numeric effecter.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entitySimple1_setNumericEffecterEnable(PldmRequestHeader* rxHeader) {
        // extract the information from the body
        unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
        unsigned int enable_state = *((char*)(((char*)rxHeader)+sizeof(PldmRequestHeader)+2));
        
        if (enable_state>2) return RESPONSE_INVALID_STATE_VALUE; 

        // set some default values
        unsigned char response = RESPONSE_SUCCESS;

        switch (effecter_id) {
        #ifdef ENTITY_SIMPLE1_EFFECTER1_EFFECTERID
            case ENTITY_SIMPLE1_EFFECTER1_EFFECTERID:
                numericeffecter_setOperationalState(&effecter1EffecterInst,enable_state);
                break;
        #endif
        default:
            response = RESPONSE_INVALID_EFFECTER_ID;   // completion code
            break;
        }
        return response;
    }

    //*******************************************************************
    // entitySimple1_setNumericEfffecterEnables()
    //
    // set the value of a state effecter enable if it exists.
    //
    // parameters:
    //    rxHeader - a pointer to the request header
    // returns:
    //    void
    // changes:
    //    the contents of the transmit buffer
    unsigned char entitySimple1_setNumericSensorEnable(PldmRequestHeader* rxHeader) {
        // extract the information from the body
        unsigned int  sensor_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
        unsigned char sensor_op_state     = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2);
        unsigned char sensor_event_enable = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+3);
        unsigned char response = RESPONSE_SUCCESS; 
        
        if (sensor_op_state>1) return RESPONSE_INVALID_STATE_VALUE; 
        if ((sensor_event_enable!=0)&&(sensor_event_enable!=1)&&(sensor_event_enable!=4)) return RESPONSE_EVENT_GENERATION_NOT_SUPPORTED; 
        
        switch (sensor_id) {
        #ifdef ENTITY_SIMPLE1_SENSOR1_SENSORID
            case ENTITY_SIMPLE1_SENSOR1_SENSORID:
                response = numericsensor_setOperationalState(&sensor1SensorInst,sensor_op_state, sensor_event_enable);
                break;
        #endif
        default:
            response = RESPONSE_INVALID_EFFECTER_ID; 
            break;
        }
        return response;
    } 

//**********************************************************************************************
//**********************************************************************************************
// CODE BELOW THIS POINT IS ON THE HIGH-PRIORITY LOOP - it needs to be optimized for speed
//**********************************************************************************************
//**********************************************************************************************

#pragma GCC push_options
#pragma GCC optimize "-O3"
//****************************************************************
// this is the high-priority update loop for the simple sensor/
// effecter. it runs in priority mode and should not
// rely on interrupt-updates to change variable states as interrupts
// are disabled when this function runs.
//
void entitySimple1_updateControl() {
    // update the global Iterlock Effecter output
    if (stateeffecter_isEnabled(&globalInterlockEffecterInst))
        CALL_CHANNEL_FUNCTION(ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_BOUNDCHANNEL,_enable());
    else
        CALL_CHANNEL_FUNCTION(ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_BOUNDCHANNEL,_disable());
    CALL_CHANNEL_FUNCTION(ENTITY_SIMPLE1_GLOBALINTERLOCKEFFECTER_BOUNDCHANNEL,_setOutput(stateeffecter_getOutput(&globalInterlockEffecterInst)));
    
    // Update the trigger Effecter output
    if (stateeffecter_isEnabled(&triggerEffecterInst))
        CALL_CHANNEL_FUNCTION(ENTITY_SIMPLE1_TRIGGEREFFECTER_BOUNDCHANNEL,_enable());
    else
        CALL_CHANNEL_FUNCTION(ENTITY_SIMPLE1_TRIGGEREFFECTER_BOUNDCHANNEL,_disable());
    CALL_CHANNEL_FUNCTION(ENTITY_SIMPLE1_TRIGGEREFFECTER_BOUNDCHANNEL,_setOutput(stateeffecter_getOutput(&triggerEffecterInst)));
    
    // read new values for all the sensors
    entitySimple1_readChannels();
}
#pragma GCC pop_options

#endif // ENTITY_SIMPLE1
