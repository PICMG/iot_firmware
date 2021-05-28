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
#include "pdrdata.h"

#ifdef ENTITY_STEPPER1
    #include "StateSensor.h"
    #include "NumericSensor.h"
    #include "StateEffecter.h"
    #include "NumericEffecter.h"
    #include "node.h"

    //===============================================================
    // Sensor-Specific Code
    //===============================================================
    static StateSensorInstance globalInterlockSensorInst; 
    static void globalInterlockSensor_sendEvent()
    {
        node_sendStateSensorEvent(&(globalInterlockSensorInst.eventGen),
            ENTITY_STEPPER1_GLOBALINTERLOCKSENSOR_SENSORID,
            statesensor_getSensorPreviousState(&globalInterlockSensorInst));
    }

    static StateSensorInstance triggerSensorInst; 
    static void triggerSensor_sendEvent()
    {
        node_sendStateSensorEvent(&(triggerSensorInst.eventGen),
            ENTITY_STEPPER1_TRIGGERSENSOR_SENSORID,
            statesensor_getSensorPreviousState(&triggerSensorInst));
    }

    static StateSensorInstance motionStateSensorInst; 
    static void motionStateSensor_sendEvent()
    {
        node_sendStateSensorEvent(&(motionStateSensorInst.eventGen),
            ENTITY_STEPPER1_MOTIONSTATE_SENSORID,
            statesensor_getSensorPreviousState(&motionStateSensorInst));
    }

    #ifdef ENTITY_STEPPER1_POSITIVELIMIT
        static StateSensorInstance positiveLimitSensorInst; 
        static void positiveLimitSensor_sendEvent()
        {
            node_sendStateSensorEvent(&(positiveLimitSensorInst.eventGen),
                ENTITY_STEPPER1_POSITIVELIMIT_SENSORID,
                statesensor_getSensorPreviousState(&positiveLimitSensorInst));
        }
    #endif

    #ifdef ENTITY_STEPPER1_NEGATIVELIMIT
        static StateSensorInstance negativeLimitSensorInst; 
        static void negativeLimitSensor_sendEvent()
        {
            node_sendStateSensorEvent(&(negativeLimitSensorInst.eventGen),
                ENTITY_STEPPER1_NEGATIVELIMIT_SENSORID,
                statesensor_getSensorPreviousState(&negativeLimitSensorInst)
            );
        } 
    #endif

    #ifdef ENTITY_STEPPER1_POSITION
        static NumericSensorInstance positionSensorInst;
        static void positionSensor_sendEvent()
        {
            node_sendNumericSensorEvent(&(positionSensorInst.eventGen),
                ENTITY_STEPPER1_POSITION_SENSORID,
                numericsensor_getSensorPreviousState(&positionSensorInst),
                positionSensorInst.value
            );
        } 
    #endif 

    //===============================================================
    // Effecter-Specific Code
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

        #ifdef ENTITY_STEPPER1_POSITION
            numericsensor_init(&positionSensorInst);
            positionSensorInst.thresholdEnables = ENTITY_STEPPER1_POSITION_THRESHOLDENABLES; 
            numericsensor_setThresholds(&positionSensorInst,
                ENTITY_STEPPER1_POSITION_DEFAULT_THRESHOLDUPPERFATAL,
                ENTITY_STEPPER1_POSITION_DEFAULT_THRESHOLDUPPERCRITICAL,
                ENTITY_STEPPER1_POSITION_DEFAULT_THRESHOLDUPPERWARNING,
                ENTITY_STEPPER1_POSITION_DEFAULT_THRESHOLDLOWERWARNING,
                ENTITY_STEPPER1_POSITION_DEFAULT_THRESHOLDLOWERCRITICAL,
                ENTITY_STEPPER1_POSITION_DEFAULT_THRESHOLDLOWERFATAL);
            positionSensorInst.eventGen.sendEvent = positionSensor_sendEvent;
        #endif 

        // initialize the global interlock effecter
        stateeffecter_init(&globalInterlockEffecterInst); 

        // initialize the trigger effecter
        stateeffecter_init(&triggerEffecterInst);

        // initialize the output effecter
        numericeffecter_init(&outputEffecterInst);

        // initialize the command effecter
        stateeffecter_init(&commandEffecterInst);

        // initialize the pfinal effecter
        numericeffecter_init(&pfinalEffecterInst);

        // initialize the vprofile effecter
        numericeffecter_init(&vprofileEffecterInst);

        // initialize the aprofile effecter
        numericeffecter_init(&aprofileEffecterInst);

        // initialize the brake effecter
        #ifdef ENTITY_STEPPER1_OUTPUTENABLE
            stateeffecter_init(&outputEnableEffecterInst);
        #endif 

        // initialize the output enable effecter
        #ifdef ENTITY_STEPPER1_BRAKEEFFECTER
            stateeffecter_init(&brakeEffecterInst);
        #endif 
    }


#endif // ENTITY_STEPPER1
