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
#include "systemtimer.h"
#include "channels.h"
#include "stepdir_out.h"
#include "entityStepper1.h"
#include "entitySimple1.h"
#include "adc.h"

int main(void)
{
  unsigned char mctp_discovery_msg[] = {0,CMD_DISCOVERY_NOTIFY};

  // enable global interrupts
  SREG |= (1<<SREG_I);

  // initialize the velocity profiler
	vprofiler_setParameters(1000000L, TO_FP16(511), TO_FP16(1), 1);
  
  // initialize the global tick timer for 4000Khz rate timeout
  systemtimer_init();

  // initilaize the uart
  uart_init();

  // initialize mctp socket
  mctp_init();
  node_init();

  // initialize all channels based on configuration paramters
  channels_init();
  adc_init();

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
  //mctp_sendNoWait(1,mctp_discovery_msg,0);
  while (1) {
    // if the discovery notify has timed out and no response has been received, 
    // send another discovery notify message
    if ((!mctp_context.discovered)&&(delay_isDone(0))) {
        //mctp_sendNoWait(2,mctp_discovery_msg,0);
        delay_set(0,1000);
    } else {
      // otherwise process messages
      node_getResponse();

      // update sensor event states
      node_updateEvents();
    }
  }
  return 0;
}

