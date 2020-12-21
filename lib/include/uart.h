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

// struct for data transfer
typedef unsigned char cbool;
typedef struct{
   int descriptor;        // file descriptor for the usb device
   cbool is_connected;
   struct termios *tty;
} uart_struct;

// function definitions
void  uart_delay_ms(float ms);
cbool uart_init(uart_struct *, const char *);
cbool uart_isConnected(uart_struct*);
cbool uart_flush(uart_struct*);
cbool uart_readCh(uart_struct *,char*);
cbool uart_writeCh(uart_struct*, char);
cbool uart_writeBuffer(uart_struct*,const void* buf, unsigned int size);
cbool uart_rx_isempty();
cbool uart_close(uart_struct *);

#endif // UART_H_INCLUDED