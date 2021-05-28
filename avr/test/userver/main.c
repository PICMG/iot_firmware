//*******************************************************************
//    main.c
//
//    This creates a simple uart loopback program for the atmega328p
//
//    Copyright (C) 2020,  PICMG
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

#include "avr/io.h"
#include "avr/pgmspace.h"
#include "pldm.h"
#include "pdrdata.h"
#include "uart.h"
#include "mctp.h"
#include "node.h"
#include "vprofiler.h"
#include "timer1.h"
#include "entityStepper1.h"

// temporary stubs
void digital_in1_init() {}
void digital_in2_init() {}
void interlock_in_init() {}
void interlock_out_init() {}
void step_dir_out1_init() {}
void trigger_in_init() {}
void trigger_out_init() {}

int main(void)
{
  unsigned char mctp_discovery_msg[] = {0,CMD_DISCOVERY_NOTIFY};

  // enable global interrupts
  SREG |= (1<<SREG_I);

  // initialize the velocity profiler
	vprofiler_setParameters(1000000L, TO_FP16(511), TO_FP16(1), 1);
  
  // initialize the global tick timer for 4000Khz rate timeout
  timer1_init();

  // initilaize the uart
  uart_init("");

  // initialize mctp socket
  mctp_init();
  node_init();

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

  #ifdef ENTITY_STEPPER1
    entityStepper1_init();
  #endif
  #ifdef ENTITY_SERVO1
    entityServo1_init();
  #endif
  #ifdef ENTITY_PID1
    entityPid1_init();
  #endif
  #ifdef ENTITY_SIMPLE1
    entitySimple1_init();
  #endif

  // send an MCTP discovery notifiy command
  delay_set(0,1000);
  mctp_sendNoWait(1,mctp_discovery_msg,0);
  while (1) {
    // if the discovery notify has timed out and no response has been received, 
    // send another discovery notify message
    if ((!mctp_context.discovered)&&(delay_isDone(0))) {
        mctp_sendNoWait(2,mctp_discovery_msg,0);
        delay_set(0,1000);
    } else {
      // otherwise process messages
      //if (mctp1.discovered) PORTB &= 0xDF;
      node_getResponse();
    }
  }
  return 0;
}

