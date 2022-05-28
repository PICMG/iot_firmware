//    stepdir_out.c
//
//    This file defines functions related to the stepdir_out 
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
#include "channels.h"
#include "stepdir_out.h"
#include "config.h"

#ifndef F_CPU
    #define F_CPU 16000000
#endif

// #define VELOCITYDEBUG
#ifndef VELOCITYDEBUG
/********************************************************************
* stepdir_out_init()
*
* initialize the step/dir channel interface
*
* parameters:
*    nothing
*
* returns:
*    void
*
* changes:
*    updates the timer registers for Timer1 (used for the step/dir 
*    rate generator)
*
* NOTE: This function assumes that the cpu clock is running at 16MHz
*/
static unsigned char direction;
static unsigned int timer_divisor;
static long residual = 0;
void step_dir_out1_init()
{
    // stop the timer
    TCCR1B = 0x18;

    // set the timer for fast PWM mode 15
    TCCR1A = 0x33;
    
    // set the top value to an arbitrary value
    OCR1A = 40;
    timer_divisor = 40;

    // set OCR1B above OCR1A so that it will not toggle
    OCR1B = 20;

    // clear the timer count
    TCNT1 = 0x0000;

    /* enable the output compare match B of the timer */
    DDRB |= (1<<DDB2);
    PORTB &= (~(1<<PB2));

    // start the timer with a prescaler of 1
    TCCR1B = 0x19;

    // set the direction pin data direction to output
    DDRB |= (1<<DDB4);
}

/********************************************************************
* stepdir_out_setOutput()
*
* program the step/dir to output a certain number of pulses within
* the sample frame.  This function should be called from the high-frequency
* loop.
*
* Due to rounding errors, this function should not be used to achieve
* more than about 40 steps/microsteps per 4KHz sample frame.  If higher
* rates are desired, a more sophisticated algorithm will be required.
*
* parameters:
*    the number of pulses to output during the frame time
*
* returns:
*    the integer number of pulses that will be sent during the 
*    next sample frame. The sign of the return value signifies the
*    direction of motion.
*
* changes:
*    updates the timer registers for Timer1 (used for the step/dir 
*    rate generator)
*
* NOTE: This function assumes that the cpu clock is running at 16MHz
*/
int step_dir_out1_setOutput(long requested_pulses)
{    
    // stop the timer - this should be done before clearing the count
    TCCR1B = 0x18;

    // set the timer to CTC mode 12 - this allows the ocr register
    // values to be changed immediately (no double buffering) 
    TCCR1A = 0x20;

    // reset the timer count to zero
    TCNT1 = 0x0000;

    // set the new top value to the timer divisor
    OCR1A = timer_divisor-1;
    
    // set OCR1B below OCR1A so that it will not toggle approximately
    // half way through each cycle
    OCR1B = (timer_divisor-1)>>1;

    // if the value of the OCR1B pin is high, clear it
    if (PINB|(1<<PB2)) TCCR1C = (1<<FOC1B);

    // output the direction
    if (direction) PORTB |= (1<<PB4);
    else PORTB &= (~(1<<PB4));

    // set the mode back to FAST PWM 15
    TCCR1A = 0x33;
    
    // enable the timer
    TCCR1B = 0x19;
     
    // calculate the divisor to get the right number of pulses
    // since:
    //    pulses / second = requested_pulses * SAMPLE_RATE;
    // and: 
    //    pulses / second = F_CPU / timer_divisor
    // timer_divisor = (F_CPU/SAMPLE_RATE)/requested_pulses
    residual += requested_pulses; 
    unsigned int pulses;
    if (residual<0) {
        direction = 1;
        pulses = -(residual/65536);
        residual += (pulses*65536);
    } else {
        direction = 0;
        pulses = (residual/65536);
        residual -= (pulses*65536);
    }
    if (pulses == 0) {
        timer_divisor = 0xFFFF;  // this is so high, it will not be reached in a frame time
    } else {
        timer_divisor = (F_CPU/SAMPLE_RATE) / pulses;
    }
    if (direction) return -pulses;
    return pulses;
}    
#else
static unsigned char direction;
static unsigned int timer_divisor;
static long residual = 0;
void step_dir_out1_init()
{
    /* set to fast pwm mode, OCR1A = top, clock divisor = 1*/
    TCCR1A = 0x82;
    TCCR1B = 0x19;

    /* set the top value to 3999 (divisor for 4khz) */
    /* duty cycle set to 50% */
    OCR1A = 2000;
    ICR1 = 3999;

    /* enable the output of the timer on OCR1A*/
    DDRB |= 0x02;
}

/********************************************************************
* stepdir_out_setOutput()
*
* program the step/dir to output a certain number of pulses within
* the sample frame.  This function should be called from the high-frequency
* loop.
*
* Due to rounding errors, this function should not be used to achieve
* more than about 40 steps/microsteps per 4KHz sample frame.  If higher
* rates are desired, a more sophisticated algorithm will be required.
*
* parameters:
*    the number of pulses to output during the frame time expressed as a
*    fixed point number with 16 bits of fractional precision.
*
* returns:
*    void
*
* changes:
*    updates the timer registers for Timer1 (used for the step/dir 
*    rate generator)
*
* NOTE: This function assumes that the cpu clock is running at 16MHz
*/
void step_dir_out1_setOutput(long requested_pulses)
{    
    // set the new top value to the timer divisor
    OCR1A = 2000+(requested_pulses/50);
  
}    
#endif   
