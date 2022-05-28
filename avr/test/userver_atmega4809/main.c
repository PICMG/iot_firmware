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

#ifdef FALSE
#include "avr/io.h"
#include "avr/pgmspace.h"
#include "pldm.h"
#include "config.h"
#include "uart.h"
#include "mctp.h"
#include "node.h"
#include "vprofiler.h"
#include "systemtimer.h"
#include "channels.h"
#include "entityStepper1.h"
#include "entitySimple1.h"
#include "adc.h"
#include "temp.h"


int main(void)
{
  // enable global interrupts
  SREG |= (1<<SREG_I);

  // set led direction and value
  PORTE_DIR |= PIN2_bm;
  PORTE_OUT &= ~PIN2_bm;

  // set peripheral clock to 16 Mhz (no divide)
  CPU_CCP = CCP_IOREG_gc;
  CLKCTRL_MCLKCTRLB = 0;

	//vprofiler_setParameters(1000000L, TO_FP16(511), TO_FP16(1), 1);
  systemtimer_init();
  uart_init();
  mctp_init();
  node_init();
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


  uart_writeBuffer("Atmega4809 Running\r\n",21);
  delay_set(0,1000);
  while (1) {
    char ch;
    if (uart_readCh(&ch)) uart_writeCh(ch);
    if (delay_isDone(0)) {
      delay_set(0,1000);
      adc_sample();
      hex16(adc_getRawData());
      uart_writeBuffer("\r\n",2);
      PORTE_OUTTGL = PIN2_bm;      
    }
  }

  return 0;
}
#else
#include "avr/io.h"
#include "avr/pgmspace.h"
#include "pldm.h"
#include "config.h"
#include "uart.h"
#include "mctp.h"
#include "node.h"
#include "vprofiler.h"
#include "systemtimer.h"
#include "channels.h"
#include "entityStepper1.h"
#include "entitySimple1.h"
#include "adc.h"

int main(void)
{
  unsigned char mctp_discovery_msg[] = {0,CMD_DISCOVERY_NOTIFY};

  // set peripheral clock to 16 Mhz (no divide)
  CPU_CCP = CCP_IOREG_gc;
  CLKCTRL_MCLKCTRLB = 0;

  // enable global interrupts
  SREG |= (1<<SREG_I);

  // set led direction and value
  PORTE_DIR |= PIN2_bm;
  PORTE_OUT &= ~PIN2_bm;

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
        PORTE_OUTTGL = PIN2_bm;      
        mctp_sendNoWait(2,mctp_discovery_msg,0);
        delay_set(0,1000);
    } else {
      PORTE_OUTCLR = PIN2_bm;      

      // otherwise process messages
      node_getResponse();

      // update sensor event states
      node_updateEvents();
    }
  }
}
#endif

