
//    systemtimer.c
//
//    This file defines functions related to the systemtimer 
//    hardware as part of the PICMG reference code for IoT.
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
#include <avr/interrupt.h>
#include "systemtimer.h"
#include "vprofiler.h"
#include "config.h"
#include "pldm.h"
#include "entityStepper1.h"
#include "entitySimple1.h"

#ifndef F_CPU
    #define F_CPU 16000000
#endif

#define DELAY_INSTANCES 4
static unsigned int delay_count[DELAY_INSTANCES];
static unsigned int delay_limit[DELAY_INSTANCES];
static unsigned int delay_newlimit[DELAY_INSTANCES];
static unsigned char delay_limit_changed[DELAY_INSTANCES];

#pragma GCC push_options
#pragma GCC optimize "-O3"
/********************************************************************
* TIMER2_COMPA_VECT
*
* interrupt handler for timer 2 compare match.  This timer is programmed for
* a 4kHz update rate.  All other system ticks are synchronized from 
* this one.
*
* parameters:
*    nothing
* returns:
*    void
* changes:
*    updates the motor controller (if used).
*    updates any delay counters
*/
static unsigned char tick = 0;
ISR(TCA0_OVF_vect) {
    // clear the overvlow condition
    TCA0_SINGLE_INTFLAGS = TCA_SINGLE_OVF_bm;
    #ifdef ENTITY_STEPPER1
        entityStepper1_updateControl();
    #endif
    #ifdef ENTITY_SERVO1
        entityServo1_updateControl();
    #endif
    #ifdef ENTITY_PID1
        entityPID1_updateControl();
    #endif
    #ifdef ENTITY_SIMPLE1
        entitySimple1_updateControl();
    #endif

    // update delay counters every fourth clock
    if (tick == 0) {
        for (unsigned char i = 0; i<DELAY_INSTANCES; i++)
            // if the delay limit has changed, update the limit, and clear the count
            if (delay_limit_changed[i]) {
                delay_limit[i] = delay_newlimit[i];
                delay_count[i] = 0;
                delay_limit_changed[i] = 0;
            } else {
                // otherwise, update the delay count 
                if (delay_count[i]<delay_limit[i]) delay_count[i]++;
            }
    }
    tick = (tick+1)&0x03;
}
#pragma GCC pop_options

/********************************************************************
* systemtimer_init()
*
* initialize the timer for pwm mode with a frame rate of 4kHz
*
* parameters:
*    nothing
*
* returns:
*    void
*
* changes:
*    updates the timer registers for Timer1.
*
* NOTE: This function assumes that the cpu clock is running at 16MHz
*       and that clock_per is running at 1/6th of the cpu clock.
*/
void systemtimer_init()
{
    // clear all the delay counters
    for (unsigned char i = 0; i<DELAY_INSTANCES; i++) {
        delay_count[i] = 0;
        delay_limit[i] = 0;
        delay_newlimit[i] = 0;
        delay_limit_changed[i] = 0;
    }

    // Write a TOP value to the Period (TCAn.PER) register.  This is set to
    // give a 4000Hz overflow rate, assuming a timer clock divide of 16
    TCA0_SINGLE_PER = ((F_CPU/16) / 4000) - 1;
    
    // Enable the peripheral by writing a ‘1’ to the ENABLE bit in the Control A (TCAn.CTRLA) register.
    // The counter will start counting clock ticks according to the prescaler setting in the Clock Select (CLKSEL) bit
    // field in the TCAn.CTRLA register.
    TCA0_SINGLE_CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc + TCA_SINGLE_CLKSEL_gp;

    // enable interrupts on TOP
    TCA0_SINGLE_INTCTRL = TCA_SINGLE_OVF_bm;

    // enable global interrupts if they are not already enabled 
    __builtin_avr_sei();

#ifdef REMOVE_THIS
    // set the data direction for the output
    DDRD |= 0x04;
#endif
}

/********************************************************************
* delay_set()
*
* Set the delay timeout for the specified delay counter.
*
* parameters:
*    delay_instance - the instance of the delay to set
*    limit - the new delay value in milliseconds
*
* returns:
*    void
*
* changes:
*
*/
void delay_set(unsigned char delay_instance, unsigned int limit) {
    // ignore delay instances that are beyond the number supported
    if (delay_instance >= DELAY_INSTANCES) return;

    // set the new value.  On the next 1ms tick, the value will be
    // set.
    delay_newlimit[delay_instance] = limit;
    delay_limit_changed[delay_instance] = 1;
}

/********************************************************************
* delay_isdone()
*
* check to see if the specified delay timer has timed out.
*
* parameters:
*    delay_instance - the instance of the delay to check
*
* returns:
*    void
*
* changes:
*    updates the delay instance for the specified timeout.
*
*/
unsigned char delay_isDone(unsigned char delay_instance) {
    // ignore delay instances that are beyond the number supported
    // delay will never be done because it does not exist
    if (delay_instance >= DELAY_INSTANCES) return 0;

    if (delay_limit_changed[delay_instance]) {
        // a change to the limit has been requested by has not yet been
        // accepted.  Return false, unless the delay value is zero.
        return (delay_newlimit[delay_instance]==0);
    }
    
    // check to see if the delay count has reached the limit.  
    // no need for synchronization here
    return (delay_count[delay_instance] == delay_limit[delay_instance]);
}