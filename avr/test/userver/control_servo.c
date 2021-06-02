//    control_servo.c
//
//    This file defines functions related to the the servo 
//    controller mode of operation for a PICMG IoT device node.  This
//    code is intended to be used as part of the PICMG reference code for IoT.
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
#ifndef __AVR_ATmega328P__ 
#define __AVR_ATmega328P__
#endif
#include <avr/io.h>
#include "vprofiler.h"
#include "control_servo.h"

#define STATE_IDLE       1
#define STATE_COND       2
#define STATE_ERROR      3
#define STATE_RUNNINGV   4
#define STATE_RUNNING    5
#define STATE_WAITING    6
#define STATE_DONE       7

unsigned char servo_cmd   = SERVO_CMD_NONE;
unsigned char servo_mode  = SERVO_MODE_NOWAIT;
unsigned char servo_flags = 0x00;

// buffered values for the requested position, velocity and acceleration
long current_position       = 0;
long requested_position     = 1000000L;
FP16 requested_velocity     = TO_FP16(511);
FP16 requested_acceleration = TO_FP16(1);
FP16 requested_kffa;
static char mode_scurve    = 1;
static unsigned char state = STATE_IDLE;

static unsigned char start_effecter_value = 2;  // initialize to STOP state;

//****************************************************************
// control_setState()
// request a state change.  This function is called from the 
// low-priority loop.
unsigned char control_setState(unsigned char reqState) {
    if (reqState == 1) {  // run requested
        if (state == STATE_IDLE) {
            // run command is only valid from the idle state
            servo_cmd = SERVO_CMD_RUN;
            start_effecter_value = reqState;
            return 1;
        }
        return 0;
    } 
    servo_cmd = SERVO_CMD_STOP; 
    start_effecter_value = reqState;
    return 1;
}

//****************************************************************
// this is the high-priority update loop for the servo motor
// control function - it runs in priority mode and should not
// rely on interrupt-updates to change variable states as interrupts
// are disabled when this function runs.
//
void control_update() {

    switch (state) {
    case STATE_IDLE:
        if (servo_flags & SERVO_FLAGS_ERROR) {
            // perform actions for entry to error state
            // error condition has priority over any other state
            // transistion

            // TODO: turn on the brake if required
            // TODO: disable the current loop if required

            state = STATE_ERROR;
        }
        else if ((servo_cmd == SERVO_CMD_RUN)&&(servo_mode != SERVO_MODE_NOWAIT)) {
            // set the motion parameters to the most recently requested
            long requested_deltax = requested_position - current_position;
            vprofiler_setParameters(requested_deltax, requested_velocity, requested_acceleration, mode_scurve);
            
            // transition to the waiting state
            state = STATE_WAITING;
        } 
        else if ((servo_cmd == SERVO_CMD_RUN) && (servo_mode == SERVO_MODE_NOWAIT)) {
            // set the motion parameters to the most recently requested and
            // start the velocity profiler
            // TODO: disengage the brake if required
            // TODO: enable the current loop if required
            long requested_deltax = requested_position - current_position;
            vprofiler_setParameters(requested_deltax, requested_velocity, requested_acceleration, mode_scurve);
            
            // send a trigger pulse to the oscope and start the profiler
            PORTB|=0x01;
            vprofiler_start();
            PORTB&=0xFE;            vprofiler_start();

            // transition to the running state
            state = STATE_RUNNING;
        }
        break;
    case STATE_RUNNING:
        // update the velocity profiler position - running is the only mode
        // in which this happens
        vprofiler_update();
        if (servo_flags & SERVO_FLAGS_ERROR) {
            // perform actions for entry to error state
            // error condition has priority over any other state
            // transistion

            // TODO: turn on the brake if required
            // TODO: disable the current loop if required

            state = STATE_ERROR;
        }
        else if (servo_flags & SERVO_FLAGS_TRIGGER ) {
            // perform actions for entry to warn state
            // warning conition has priority over all but error
            // transition

            // TODO: turn on the brake if required
            // TODO: disable the current loop if required

            state = STATE_COND;
        }
        else if (servo_cmd == SERVO_CMD_STOP) {
            // set the motion parameters to idle settings (follow/coast/brake)

            // TODO: turn on the brake if required
            // TODO: disable the current loop if required

            // transition to the idle state
            state = STATE_IDLE;
        } 
        else if (vprofiler_isDone()) {
            // set the motion parameters to the done settings (follow/coast/brake)

            // TODO: turn on the brake if required
            // TODO: disable the current loop if required

            // transition to the done state
            state = STATE_DONE;
        }
        break;
    case STATE_WAITING:
        if (servo_flags & SERVO_FLAGS_ERROR) {
            // perform actions for entry to error state
            // error condition has priority over any other state
            // transistion
            // TODO: engage the brake if required
            // TODO: disable the current loop if required
            state = STATE_ERROR;
        }
        else if (!(servo_flags & SERVO_FLAGS_TRIGGER )) {
            // when waiting, transition to running mode on negative
            // edge of global trigger 
            // TODO: disengage the brake if required
            // TODO: enable the current loop if required
            vprofiler_start();
            state = STATE_RUNNING;
        }
        else if (servo_cmd == SERVO_CMD_STOP) {
            // set the motion parameters to idle settings (follow/coast/brake)
            // TODO: engage the brake if required
            // TODO: disable the current loop if required
            
            // transition to the idle state
            state = STATE_IDLE;
        } 
        break;
    case STATE_DONE:
        if (servo_flags & SERVO_FLAGS_ERROR) {
            // perform actions for entry to error state
            // error condition has priority over any other state
            // transistion
            // TODO: engage the brake if required
            // TODO: disable the current loop if required
            state = STATE_ERROR;
        }
        else if (servo_flags & SERVO_FLAGS_TRIGGER ) {
            // perform actions for entry to warnding state
            // warning conition has priority over all but error
            // transition
            // TODO: engage the brake if required
            // TODO: disable the current loop if required
            state = STATE_COND;
        }
        else if (servo_cmd == SERVO_CMD_STOP) {
            // set the motion parameters to idle settings (follow/coast/brake)
            // TODO: engage the brake if required
            // TODO: disable the current loop if required
            
            // transition to the idle state
            state = STATE_IDLE;
        } 
    case STATE_ERROR:
        if (servo_cmd == SERVO_CMD_STOP) {
            // set the motion parameters to idle settings (follow/coast/brake)
            // TODO: disengage the brake if required
            // TODO: enable the current loop if required
            
            // transition to the idle state
            state = STATE_IDLE;
        } 
        break;
    case STATE_COND:
        if (servo_flags & SERVO_FLAGS_ERROR) {
            // perform actions for entry to error state
            // error condition has priority over any other state
            // transistion
            // TODO: engage the brake if required
            // TODO: disable the current loop if required
            state = STATE_ERROR;
        }
        else if (servo_cmd == SERVO_CMD_STOP) {
            // set the motion parameters to idle settings (follow/coast/brake)
            // TODO: disengage the brake if required
            // TODO: enable the current loop if required
            
            // transition to the idle state
            state = STATE_IDLE;
        } 
        break;
    default:
        // perform actions for entry to error state
        // error condition has priority over any other state
        // transistion
        state = STATE_IDLE;
    }
    servo_cmd = SERVO_CMD_NONE;       
}

//****************************************************************
// getState()
// this is a low-priority task that returns the current state of
// the motor controller.
//
unsigned char control_getState() { 
    return state;
}