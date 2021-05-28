//    EventGenerator.c
//
//    This header file defines functions related to the the EventGenerator 
//    "base class".  Sensors that can generate events utilize code from
//    this class to send events to the PLDM event listener.
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
#include "EventGenerator.h"
#include "pdrdata.h"
#include "mctp.h"

#define MAX_TIMEOUT_COUNT (1*SAMPLE_RATE)
#define MAX_ATTEMPTS      3

//==============================================
// MACRO DEFINITIONS FOR THE STATE MACHINE STATE
//==============================================
#define DISABLED 0
#define ENABLED  1
#define PENDING  2
#define SENDING  3
#define WAITING  4

//===================================================================
// eventgenerator_init()
//
// set the initial values for the Event generator instance.
//
// parameters:
//    egi - a pointer to the event generator instance that will be
//          evaluated.
void eventgenerator_init(EventGeneratorInstance *egi)
{
    egi->acknowledged  = 0;
    egi->attempts      = 0;
    egi->eventOccurred = 0;
    egi->eventState    = DISABLED;
    egi->timeoutCount  = 0;
    egi->sendEvent     = 0;
}

//===================================================================
// eventgenerator_setEventOccurredFn()
//
// set the function to be used to check if an event occured.
//
// parameters:
//    egi - a pointer to the event generator instance that will be
//          evaluated.
void eventgenerator_setEventOccuredFn(EventGeneratorInstance *egi, char (*fn)())
{
    egi->eventOccurred = fn;
}

//===================================================================
// eventgenerator_setSendEventFn()
//
// set the function to be used to send an event.
//
// parameters:
//    egi - a pointer to the event generator instance that will be
//          evaluated.
void eventgenerator_setSendEventFn(EventGeneratorInstance *egi,void (*fn)())
{
    egi->sendEvent = fn;
}

//===================================================================
// eventgenerator_isEventPending()
//
// this function returns true if the sensor has an event that is 
// pending that has not been sent to the PDLM Event Receiver.  
// This corresponds to the PENDING finite state machine state. 
//
// parameters:
//    egi - a pointer to the event generator instance that will be
//          evaluated.
char eventgenerator_isEventPending(EventGeneratorInstance* egi) 
{
    return (egi->eventState == PENDING);
}

//===================================================================
// eventgenerator_isEventSending()
//
// this function returns true if the sensor has initiated an event 
// message with the PLDM Event Receiver but the event has not yet 
// been acknowledged.
//
// parameters:
//    egi - a pointer to the event generator instance that will be
//          evaluated.
char eventgenerator_isEventSending(EventGeneratorInstance* egi)
{
    return ((egi->eventState == SENDING) || 
            (egi->eventState == WAITING));
}

//===================================================================
// eventgenerator_isEventEnabled()
//
// this function returns true if the event generator is not disabled.
//
// parameters:
//    egi - a pointer to the event generator instance that will be
//          evaluated.
char eventgenerator_isEnabled(EventGeneratorInstance* egi) 
{
    return (egi->eventState != DISABLED);
}

//===================================================================
// eventgenerator_setEnableAsynchronousEvents()
//
// based on the specified parameter, enable or disable events for 
// this sensor.  If the parameter is “true”, asynchronous event 
// generation for the I/O device is enabled.
//
// parameters:
//    egi - a pointer to the event generator instance that will be
//          operated on.
//    enable - if true, enable events, otherwise disable them
void eventgenerator_setEnableAsyncEvents(EventGeneratorInstance* egi, unsigned char enable)
{
    if ((enable)&&(egi->eventState == DISABLED)) {
        egi->eventState = ENABLED;
    } else if (!enable) {
        egi->eventState = DISABLED;
    }
}

//===================================================================
// eventgenerator_startSending()
//
// When the sensors’s state machine is in the PENDING state, this 
// function will cause it to transition to SENDING so that on the 
// next call to updateEventStateMachine(), event transmission will 
// begin.  This function has no effect when the state machine is in 
// any other state.
// 
// parameters:
//    egi - a pointer to the event generator instance that will be
//          operated on.
void eventgenerator_startSending(EventGeneratorInstance* egi)
{
    if (egi->eventState == PENDING) {
        egi->eventState = SENDING;
        egi->attempts = 0;
        egi->acknowledged = 0; 
    }
}

//===================================================================
// eventgenerator_updateEventStateMachine()
//
// this function updates the event state machine for the sensor.  
// This function should be called at regular intervals by the main 
// program code to maintain event generation functionality for the 
// sensor. 
//
// parameters:
//    egi - a pointer to the event generator instance that will be
//          operated on.
void eventgenerator_updateEventStateMachine(EventGeneratorInstance* egi)
{
    switch (egi->eventState) {
        case DISABLED:
            // do nothing - just wait to be enabled
            break;
        case ENABLED:
            if (egi->eventOccurred()) egi->eventState = PENDING;
            break;
        case PENDING:
            // do nothing - just wait for start sending command
            break;
        case SENDING:
            egi->sendEvent();
            egi->timeoutCount = 0;
            egi->attempts++;
            egi->eventState = WAITING;
            break;
        case WAITING:
            // wait for an acknowledge or a timeout
            if (egi->acknowledged) {
                egi->eventState = ENABLED;
            } else {
                egi->timeoutCount ++;
                if (egi->timeoutCount>=MAX_TIMEOUT_COUNT) {
                    if (egi->attempts<=MAX_ATTEMPTS) {
                        egi->attempts++;
                        egi->eventState = SENDING;
                    } else {
                        egi->eventState = ENABLED;
                    }
                }
            }
            break;
    }
}


