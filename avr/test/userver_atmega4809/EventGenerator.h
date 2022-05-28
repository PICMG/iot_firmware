//    EventGenerator.h
//
//    This header file declares functions related to the the EventGenerator 
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
#ifndef EVENTGENERATOR_H_INCLUDED
#define EVENTGENERATOR_H_INCLUDED
#include "pldm.h"

typedef struct {
    unsigned char eventState;   // the current state of the event generator
    char priority;              // fifo priority
    char (*eventOccurred)();    // pointer to child's implmentation eventOccured()
    void (*sendEvent)(PldmRequestHeader *, unsigned char); // pointer to child's implmentation of sendEvent()

} EventGeneratorInstance;

void eventgenerator_init(EventGeneratorInstance *egi);
void eventgenerator_setEventOccurredFn(EventGeneratorInstance *egi,char (*)());
void eventgenerator_setSendEventFn(EventGeneratorInstance *egi,void (*)(PldmRequestHeader *, unsigned char));
char eventgenerator_isEventPending(EventGeneratorInstance* egi);
char eventgenerator_isEventSent(EventGeneratorInstance* egi);
char eventgenerator_isEnabled(EventGeneratorInstance* egi);
void eventgenerator_setEnableEvents(EventGeneratorInstance* egi, unsigned char enable);
void eventgenerator_startSending(EventGeneratorInstance* egi, PldmRequestHeader *rxHeader, unsigned char more);
void eventgenerator_updateEventStateMachine(EventGeneratorInstance* egi);
void eventgenerator_acknowledge(EventGeneratorInstance* egi);

#endif // EVENTGENERATOR_H_INCLUDED