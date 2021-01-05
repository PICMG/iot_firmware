
//    timer1.c
//
//    This file defines functions related to the timer1 
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
#ifndef __AVR_ATmega328P__ 
#define __AVR_ATmega328P__
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer1.h"
#include "vprofiler.h"
#include "control_servo.h"

static int duty = 00;
/********************************************************************
* __vector_11
*
* interrupt handler for timer 1 reset.  This timer is programmed for
* a 4kHz update rate.  All other system ticks are synchronized from 
* this one.
* When the PWM output is used, it is also driven from this timer.
*
* parameters:
*    nothing
* returns:
*    void
* changes:
*    increments the 1 second tick counter.
*/
ISR(TIMER1_COMPA_vect) {
    // the duty cycle is set to the velocity profiler output for now
    OCR1A = duty;
    control_update();
    duty = (unsigned int)(current_velocity>>14)+2048L;
}

/********************************************************************
* timer0_init()
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
void timer1_init()
{
    /* set the top value to 3999 (divisor for 4khz) */
    /* duty cycle set to 50% */
    OCR1A = 2000;
    ICR1 = 3999;

    /* set to fast pwm mode, OCR1A = top, clock divisor = 1*/
    TCCR1A = 0x82;
    TCCR1B = 0x19;

    /* enable global interrupts if they are not already enabled */
    __builtin_avr_sei();

    /* enable interrupts on tov */
    TIMSK1 = 0x01;
}
