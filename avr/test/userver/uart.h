//*******************************************************************
//    uart.h
//
//    This file provides definitions for UART serial port
//    binding. This header is intended to be used as part of 
//    the PICMG PLDM library reference code. 
//
//    More information on the PICMG IoT data model can be found within
//    the PICMG family of IoT specifications.  For more information,
//    please visit the PICMG web site (www.picmg.org)
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
#ifndef UART_H_INCLUDED
#define UART_H_INCLUDED

// function definitions
void uart_init();
unsigned char uart_flush();
unsigned char uart_readCh();
unsigned char uart_writeCh(char);
unsigned char uart_writeBuffer(const void* buf, unsigned int size);
unsigned char uart_rx_isempty();
unsigned char uart_close();

#endif // UART_H_INCLUDED