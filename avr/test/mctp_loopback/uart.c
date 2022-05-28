//*******************************************************************
//    uart.c
//
//    This file provides definitions for UART serial port
//    binding for avr. This header is intended to be used as part of 
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
#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"

// Baud rate.
#define BAUD 38400
#define BUFFERSIZE 64    // this must be an 8-bit power of 2
#define BIT2NUM(bit) (1<<bit)

// rx buffer implementation
static volatile unsigned char uart_rxhead = 0;  // insertion point
static volatile unsigned char uart_rxtail = 0;  // extraction point
static volatile unsigned char uart_rxbuf[BUFFERSIZE];

// tx buffer implementation
static volatile unsigned char uart_txhead = 0;  // insertion point
static volatile unsigned char uart_txtail = 0;  // extraction point
static volatile unsigned char uart_txbuf[BUFFERSIZE];

//===================================================================
// uart receive interrupt service routine
//
// this interrupt service routine grabs a character from the uart
// and places it in the receive buffer.  If the buffer is full, the 
// new charcter is thrown away.
ISR(USART_RX_vect) {
    // get the character from the uart data register
    unsigned char ch = UDR0;

    // if there is space in the buffer place the new character in the buffer
    if ((uart_rxtail - uart_rxhead - 1) & (BUFFERSIZE - 1)) {
        uart_rxbuf[uart_rxhead] = ch;
        uart_rxhead = (uart_rxhead + 1)&(BUFFERSIZE-1);
    }
}

//===================================================================
// uart transmit ready interrupt service routine
//
// this interrupt service routine grabs a character from the transmit
// buffer and places it in the uart transmit register.  If the buffer  
// empty, further interrupts are disabled
ISR(USART_UDRE_vect) {
    // if there is a character, place it in the transmit buffer
    if ((uart_txhead - uart_txtail) & (BUFFERSIZE - 1)) {
        UDR0 = uart_txbuf[uart_txtail]; 
        uart_txtail = (uart_txtail + 1) & (BUFFERSIZE - 1);
    }
    // otherwise, disable further interrupts 
    else {
        UCSR0B &= ~BIT2NUM(UDRIE0);
    }
}

//*******************************************************************
// uart_init()
//
// This function initializes the UART with the provided linux USB pin.
//
// parameters:
//    name - the name of the linux device to connect to (e.g. "/dev/ttyUSB0")
// returns:
//    true if the connection was successful
void uart_init() {
    // set the buad rate based on the cpu frequency
    #if BAUD > 38401
        UCSR0A = BIT2NUM(U2X0);
        UBRR0 = (((F_CPU / 8) / BAUD) - 0.5);
    #else
        UCSR0A = 0;
        UBRR0 = (((F_CPU / 16) / BAUD) - 0.5);
    #endif

    // configure the UCSR0B and UCSR0C for uart operation
    UCSR0B = BIT2NUM(RXEN0) | BIT2NUM(TXEN0);
    UCSR0C = BIT2NUM(UCSZ01) | BIT2NUM(UCSZ00);

    UCSR0B |= BIT2NUM(RXCIE0) | BIT2NUM(UDRIE0);
}

//*******************************************************************
// uart_isConnected()
//
// A helper function that returns if the UART is connected
//
// returns:
//    true if connected
unsigned char uart_isConnected() {
    return 1;
}

//*******************************************************************
// uart_flush()
//
// This function flushes the buffer.
//
// returns:
//    true if the buffer was successfuly flushed
unsigned char uart_flush() {
    uart_txtail = uart_txhead;
    return 1;
}

//*******************************************************************
// uart_readCh()
//
// This function reads a char from the buffer.
//
// parameters:
//    ch - a pointer to where the read-in char can be stored
// returns:
//    unsigned char - whether the read was successful
unsigned char uart_readCh(char*ch) {
    // if there is a character in the buffer, set the value in ch
    // and return true
    if ((uart_rxhead - uart_rxtail) & (BUFFERSIZE - 1)) {
        *ch = uart_rxbuf[uart_rxtail]; 
        uart_rxtail = (uart_rxtail + 1) & (BUFFERSIZE - 1);
        return 1;
    }
    // otherwise, return false
    return 0;
}

//*******************************************************************
// uart_writeCh()
//
// This function writes a char out to the buffer.
//
// parameters:
//    ch - the char being sent out to the serial port
// returns:
//    true if the write was successful
unsigned char uart_writeCh(char ch) {
    // if interrupts are enabled, wait for space to exist
    // in the buffer and write the result.
    if (SREG & BIT2NUM(SREG_I)) {
        while (!((uart_txtail - uart_txhead - 1) & (BUFFERSIZE - 1))) {}
        uart_txbuf[uart_txhead] = ch;
        uart_txhead = (uart_txhead + 1)&(BUFFERSIZE-1);
    }
    else {
        // otherwise, only write the character in the transmit
        // buffer if there is room.
        if ((uart_txtail - uart_txhead - 1) & (BUFFERSIZE - 1)) {
            uart_txbuf[uart_txhead] = ch;
            uart_txhead = (uart_txhead + 1)&(BUFFERSIZE-1);
        }
        else {
            // character not sent
            return 0;
        }
    }
    // enable uart tx interrupt so that the character will be sent
    UCSR0B |= BIT2NUM(UDRIE0);
    return 1;
}

//*******************************************************************
// uart_writeBuffer()
//
// This function writes a buffer of chars (char*, str, char[])
//
// parameters:
//    buffer - the chars being written out
//    len - the length of the buffer
// returns:
//    true if the write was successful
unsigned char uart_writeBuffer(const void* buf, unsigned int size) {
    for (int i=0; i<size; i++) {
        if (!uart_writeCh(((unsigned char *)buf)[i])) {
            return 0;
        }
    }
    return 1;
}

//*******************************************************************
// uart_rx_isempty()
//
// This function returns true if the receive buffer is empty, otherwise,
// it returns false.
//
// returns:
//    true if the buffer is empty, otherwise false
unsigned char uart_rx_isempty() {
    return ((uart_rxhead - uart_rxtail) & (BUFFERSIZE - 1));
}

//*******************************************************************
// uart_close()
//
// This function frees the memory taken by the serial binding
//
// returns:
//    true if the close was successful
unsigned char uart_close() {
    return 1;
}

