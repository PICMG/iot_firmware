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

int main(void)
{
  mctp_struct mctp1;

  // enable global interrupts
  SREG |= (1<<SREG_I);

  // initilaize the uart 
  uart_init("");

  // initialize mctp socket
  mctp_init(0, &mctp1);
          
  while (1) {
    if (!mctp_isPacketAvailable(&mctp1)) {
      mctp_updateRxFSM(&mctp1);
    } else {
      // packet has arrived - process as a loopback
      mctp_transmitFrameStart(&mctp1,10+5);
      mctp_transmitFrameData(&mctp1,mctp_getPacket(&mctp1),10);
      mctp_transmitFrameEnd(&mctp1);
    }
  }
  return 0;
}

