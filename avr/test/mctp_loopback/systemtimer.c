
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
ISR(TIMER2_COMPA_vect) {
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
*/
void systemtimer_init()
{
    DDRD |= 0x04;

    /* clear all the delay counters */
    for (unsigned char i = 0; i<DELAY_INSTANCES; i++) {
        delay_count[i] = 0;
        delay_limit[i] = 0;
        delay_newlimit[i] = 0;
        delay_limit_changed[i] = 0;
    }

    // TCCR2A
    // Normal Port operation, OC2A disconnected
    // Normal Port operation, OC2B disconnected
    // WGM = 010 = CTC  - WGM21:0 = 10'b
    TCCR2A = 0x02;

    // TCCR2B
    // No force compare for OCA/OCB
    // WGM22 = 0
    // CS2[2:0] = 011'b (divide by 32)
    TCCR2B = 0x03;

    // Set the divisor for 4Khz
    // 16Mhz/32(prescaler) = 500khz(prescaled clock rate)
    // 500Khz / 125 = 4KHz 
    OCR2A = (F_CPU/32) / 4000;

    // enable global interrupts if they are not already enabled 
    __builtin_avr_sei();

    // enable interrupts on compare match A
    TIMSK2 = 0x02;
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