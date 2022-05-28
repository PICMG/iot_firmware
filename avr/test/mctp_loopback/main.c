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
#include "avr/io.h"
#include "uart.h"
#include "mctp.h"
#include "systemtimer.h"

char packetbody[] = "This is a test of MCTP";
int main(void)
{
  // enable global interrupts
  SREG |= (1<<SREG_I);
  
  // initialize the global tick timer for 4000Khz rate timeout
  systemtimer_init();

  // initilaize the uart
  uart_init();

  // initialize mctp socket
  mctp_init();
  mctp_sendNoWait(22, packetbody, 0x01);

  while (1) {
    mctp_updateRxFSM();
    if (mctp_isPacketAvailable(&mctp_context)) {
      unsigned int packetSize = mctp_context.rxInsertionIdx;
      unsigned char type = mctp_context.last_msg_type;
      // return the packet
      mctp_sendNoWait(packetSize, mctp_getPacket(), type);
    }
  }
  return 0;
}

