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

int main(void)
{
  mctp_struct mctp1;
  // enable global interrupts
  SREG |= (1<<SREG_I);

  // initialize the velocity profiler
	vprofiler_setParameters(1000000L, TO_FP16(511), TO_FP16(1), 1);
  
  // initialize the global tick timer
  timer1_init();

  // initilaize the uart 
  uart_init("");

  // initialize mctp socket
  mctp_init(0, &mctp1);
  node_init(&mctp1);
          
  while (1) {
     node_getResponse();
  }
  return 0;
}

