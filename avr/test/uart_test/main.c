/*
 */

#include "avr/io.h"
#include "uart.h"

/**********************************************************
* writestr()
*
* write an ascii null-terminated string to the uart.
*
* parameters:
*   str - a pointer to the ascii null-terminated string
* returns:
*   void
* changes:
*   the state of the USART will change due to sending
*   characters to the transmit buffer.
*/
void writestr(char* str)
{
    unsigned i = 0;
    while (str[i]!=0) {
        uart_writeCh(0,str[i]);
        i++;
    }
}

/**********************************************************
* writesize
*
* write the size of a data type to the console.  This is
* a very simple routine that is only intended for HW2.
*
* parameters:
*    size - the size of the data type
* returns:
*    void
* changes:
*    the state of the USART will change due to writing
*    characters to the transmit buffer.
*/
void writesize(int size)
{
  switch (size) {
  case 1:
    writestr("8 bits\n");
    break;
  case 2:
    writestr("16 bits\n");
    break;
  case 4:
    writestr("32 bits\n");
    break;
  default:
    writestr("unknown\n");
  }
}

int main(void)
{
    // enable global interrupts
    SREG |= (1<<SREG_I);
    uart_init(0,"");

    writestr("Hello World from Atmega328\n");
    writestr("SER486 - Homework Assignment 2\n");
    writestr("Doug Sandy\n");
    writestr("char size (bits) = ");writesize(sizeof(char));
    writestr("int  size (bits) = ");writesize(sizeof(int));
    writestr("long size (bits) = ");writesize(sizeof(long));

    while (1) {
        char ch;
        if (uart_readCh(0,&ch)) {
          uart_writeCh(0,ch);
        }
    }
    return 0;
}

