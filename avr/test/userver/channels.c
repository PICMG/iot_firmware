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
#ifndef __AVR_ATmega328P__ 
#define __AVR_ATmega328P__
#endif

#include "pdrdata.h"
#include "avr/io.h"
#include "channels.h"

//===================================================================
// This macro conatenates two tokens into one
#define CONCATENATE(x, y) x ## y

//===================================================================
// This macro generates digital input channel functions for the 
// named interface channel on the specified port and bit.
#define DEFINE_DIGITAL_IN_CHANNEL_FUNCTS(channelname, port, bit) \
    static unsigned char CONCATENATE(channelname,_rawdata); \
    void CONCATENATE(channelname, _init()) { \
        /* set the bit direction to input */ \
        CONCATENATE(DDR, port) &= (~(1<<bit)); \
        /* turn on the pull-up resistor */ \
        CONCATENATE(PORT, port) |= (1<<bit); \
    } \
    void CONCATENATE(channelname, _sample()) { \
      CONCATENATE(channelname,_rawdata) = ((CONCATENATE(PIN, port))>>bit)&1; \
    } \
    unsigned char CONCATENATE(channelname, _getRawData()) { \
      return CONCATENATE(channelname, _rawdata); \
    }

//===================================================================
// This macro generates digital output channel functions for the 
// named interface channel on the specified port and bit.
#define DEFINE_DIGITAL_OUT_CHANNEL_FUNCTS(channelname, port, bit) \
    void CONCATENATE(channelname, _init()) { \
        /* set the bit direction to input */ \
        CONCATENATE(DDR, port) &= (~(1<<bit)); \
        /* turn off the pull-up resistor */ \
        CONCATENATE(PORT, port) &= (~(1<<bit)); \
    } \
    void CONCATENATE(channelname, _setOutput(unsigned char output)) { \
        /* set the output value */ \
        CONCATENATE(PORT, port) |= (output<<bit); \
    } \
    void CONCATENATE(channelname, _enable()) { \
        /* set the pin to output */ \
        CONCATENATE(DDR, port) &= (~(1<<bit)); \
    } \
    void CONCATENATE(channelname, _disable()) { \
        /* set the pin to input */ \
        CONCATENATE(DDR, port) |= (~(1<<bit)); \
        /* turn off the pull-up resistor */ \
        CONCATENATE(PORT, port) &= (~(1<<bit)); \
    } \

  // declare the channel-related raw data
  #ifdef CHANNEL_AIN12V
    unsigned int ain12v_rawdata;
  #endif
  #ifdef CHANNEL_AIN24V
    unsigned int ain24v_rawdata;
  #endif
  #ifdef CHANNEL_AIN5V
    unsigned int ain5v_rawdata;
  #endif
  #ifdef CHANNEL_COUNT_IN1
    unsigned int count_in1_rawdata;
  #endif
  #ifdef CHANNEL_COUNT_IN2
    unsigned int count_in2_rawdata;
  #endif
  #ifdef CHANNEL_COUNT_IN3
    unsigned int count_in3_rawdata;
  #endif
  #ifdef CHANNEL_COUNT_IN4
    unsigned int count_in4_rawdata;
  #endif
  #ifdef CHANNEL_COUNT_IN5
    unsigned int count_in5_rawdata;
  #endif
  #ifdef CHANNEL_CURRENT_LOOP_IN
    unsigned int current_loop_in_rawdata;
  #endif
  #ifdef CHANNEL_DIGITAL_IN1
    DEFINE_DIGITAL_IN_CHANNEL_FUNCTS(digital_in1, B, 3)
  #endif
  #ifdef CHANNEL_DIGITAL_IN2
    DEFINE_DIGITAL_IN_CHANNEL_FUNCTS(digital_in2, B, 5)
  #endif
  #ifdef CHANNEL_IN_DIGITAL_IN3
    DEFINE_DIGITAL_IN_CHANNEL_FUNCTS(digital_in3, D, 5)
  #endif
  #ifdef CHANNEL_IN_DIGITAL_IN4
    DEFINE_DIGITAL_IN_CHANNEL_FUNCTS(digital_in4, E, 0)
  #endif
  #ifdef CHANNEL_IN_DIGITAL_IN5
    DEFINE_DIGITAL_IN_CHANNEL_FUNCTS(digital_in5, B, 0)
  #endif
  #ifdef CHANNEL_DIGITAL_OUT1
    DEFINE_DIGITAL_OUT_CHANNEL_FUNCTS(digital_out1, B, 4)
  #endif
  #ifdef CHANNEL_DIGITAL_OUT2
    DEFINE_DIGITAL_OUT_CHANNEL_FUNCTS(digital_out2, D, 5)
  #endif
  #ifdef CHANNEL_DIGITAL_OUT3
    DEFINE_DIGITAL_OUT_CHANNEL_FUNCTS(digital_out3, E, 0)
  #endif
  #ifdef CHANNEL_DIGITAL_OUT4
    DEFINE_DIGITAL_OUT_CHANNEL_FUNCTS(digital_out4, B, 0)
  #endif
  #ifdef CHANNEL_INTERLOCK_IN
    DEFINE_DIGITAL_IN_CHANNEL_FUNCTS(interlock_in, D, 2)
  #endif
  #ifdef CHANNEL_INTERLOCK_OUT
    DEFINE_DIGITAL_OUT_CHANNEL_FUNCTS(interlock_out, D, 6)
  #endif
  #ifdef CHANNEL_PWM_OUT1
    unsigned int pwm_out1_rawdata;
  #endif
  #ifdef CHANNEL_QUADRATURE_IN1
    long quadrature_in1_rawdata;
  #endif
  #ifdef CHANNEL_RATE_IN1
    unsigned int rate_in1_rawdata;
  #endif
  #ifdef CHANNEL_RATE_IN2
    unsigned int rate_in2_rawdata;
  #endif
  #ifdef CHANNEL_RATE_IN3
    unsigned int rate_in3_rawdata;
  #endif
  #ifdef CHANNEL_RATE_IN4
    unsigned int rate_in4_rawdata;
  #endif
  #ifdef CHANNEL_RATE_IN5
    unsigned int rate_in5_rawdata;
  #endif
  #ifdef CHANNEL_RATE_OUT1
    unsigned int rate_out1_rawdata;
  #endif
  #ifdef CHANNEL_RATE_OUT2
    unsigned int rate_out2_rawdata;
  #endif
  #ifdef CHANNEL_RATE_OUT3
    unsigned int rate_out3_rawdata;
  #endif
  #ifdef CHANNEL_RATE_OUT4
    unsigned int rate_out4_rawdata;
  #endif
  #ifdef CHANNEL_STEP_DIR_OUT1
    unsigned int step_dir_out1_step_rawdata;
    unsigned char setp_dir_out1_dir_rawdata;
  #endif
  #ifdef CHANNEL_TRIGGER_IN
    DEFINE_DIGITAL_IN_CHANNEL_FUNCTS(trigger_in, D, 3)
  #endif
  #ifdef CHANNEL_TRIGGER_OUT
    DEFINE_DIGITAL_OUT_CHANNEL_FUNCTS(trigger_out, D, 7)
  #endif

//===================================================================
// channels_init()
//
// initialize all the channels based on configuration switches
//
// parameters: none
// returns: nothing
// changes:
//   The state of all channel hardware will be initialized and set
// to the "disabled" state.
void channels_init() 
{
      // initialize all channels based on configuration paramters
  #ifdef CHANNEL_AIN12V
    ain12v_init();
  #endif
  #ifdef CHANNEL_AIN24V
    ain24v_init();
  #endif
  #ifdef CHANNEL_AIN5V
    ain5v_init();
  #endif
  #ifdef CHANNEL_COUNT_IN1
    count_in1_init();
  #endif
  #ifdef CHANNEL_COUNT_IN2
    count_in2_init();
  #endif
  #ifdef CHANNEL_COUNT_IN3
    count_in3_init();
  #endif
  #ifdef CHANNEL_COUNT_IN4
    count_in4_init();
  #endif
  #ifdef CHANNEL_COUNT_IN5
    count_in5_init();
  #endif
  #ifdef CHANNEL_CURRENT_LOOP_IN
    current_loop_in_init();
  #endif
  #ifdef CHANNEL_DIGITAL_IN1
    digital_in1_init();
  #endif
  #ifdef CHANNEL_DIGITAL_IN2
    digital_in2_init();
  #endif
  #ifdef CHANNEL_DIGITAL_IN3
    digital_int3_init();
  #endif
  #ifdef CHANNEL_DIGITAL_IN4
    digital_in4_init();
  #endif
  #ifdef CHANNEL_DIGITAL_IN5
    digital_in5_init();
  #endif
  #ifdef CHANNEL_DIGITAL_OUT1
    digital_out1_init();
  #endif
  #ifdef CHANNEL_DIGITAL_OUT2
    digital_out2_init();
  #endif
  #ifdef CHANNEL_DIGITAL_OUT3
    digital_out3_init();
  #endif
  #ifdef CHANNEL_DIGITAL_OUT4
    digital_out4_init();
  #endif
  #ifdef CHANNEL_INTERLOCK_IN
    interlock_in_init();
  #endif
  #ifdef CHANNEL_INTERLOCK_OUT
    interlock_out_init();
  #endif
  #ifdef CHANNEL_PWM_OUT1
    pwm_out1_init();
  #endif
  #ifdef CHANNEL_QUADRATURE_IN1
    quadrature_in1_init();
  #endif
  #ifdef CHANNEL_RATE_IN1
    rate_in1_init();
  #endif
  #ifdef CHANNEL_RATE_IN2
    rate_in2_init();
  #endif
  #ifdef CHANNEL_RATE_IN3
    rate_in3_init();
  #endif
  #ifdef CHANNEL_RATE_IN4
    rate_in4_init();
  #endif
  #ifdef CHANNEL_RATE_IN5
    rate_in5_init();
  #endif
  #ifdef CHANNEL_RATE_OUT1
    rate_out1_init();
  #endif
  #ifdef CHANNEL_RATE_OUT2
    rate_out2_init();
  #endif
  #ifdef CHANNEL_RATE_OUT3
    rate_out3_init();
  #endif
  #ifdef CHANNEL_RATE_OUT4
    rate_out4_init();
  #endif
  #ifdef CHANNEL_STEP_DIR_OUT1
    step_dir_out1_init();
  #endif
  #ifdef CHANNEL_TRIGGER_IN
    trigger_in_init();
  #endif
  #ifdef CHANNEL_TRIGGER_OUT
    trigger_out_init();
  #endif
}
