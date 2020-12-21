//*******************************************************************
//    uart.c
//
//    This file contains implementation of a UART serial binding 
//    which is intended to be used as part of 
//    the PICMG PLDM library reference code. This file contains
//	  functions that allow char transfer through a serial port.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
#include <sys/ioctl.h>
#include <sys/select.h>
#include "uart.h"
#include "time.h"

//*******************************************************************
// delay_ms()
//
// This is a helper function that causes a delay
//
// parameters:
//	  ms - the amount of delay in milliseconds
// returns:
//    void
void delay_ms(float ms) {
    time_t start = time(NULL);
    while (difftime(time(NULL), start) < ms / 1000.0) {};
}

//*******************************************************************
// uart_init()
//
// This function initializes the UART with the provided linux USB pin.
//
// parameters:
//	  vars - a data struct used for all uart functions
//    name - the name of the linux USB pin, usually "/dev/ttyUSB0"
// returns:
//    cbool - whether the connection was successful
cbool uart_init(uart_struct * vars, const char* name)
{
    // dont reinitialize if the port is still connected
    if ((vars->is_connected)==1) return 1;

    if (vars->descriptor) close(vars->descriptor);

    // allocate memory for the termios structure
    vars->tty = (struct termios *)malloc(sizeof(struct termios));
    if (!(vars->tty)) return 0;

    memset((void*)(vars->tty), 0, sizeof(*(vars->tty)));

    // attempt to open the USB connection
    printf("opening ");
    printf("%s",name);
    printf("\n");
    vars->descriptor = open(name, O_RDWR | O_NOCTTY);
    printf("%d",(vars->descriptor));
    printf("\n");
    if(vars->descriptor<=0)
    {
        printf("invalid usb port\n");
        return 0;
    }

    /* get the current attributes for the usb connection */
    if (tcgetattr(vars->descriptor, vars->tty) != 0) return 0;

    /* Set Baud Rate */
    cfsetospeed(vars->tty, (speed_t)B115200);
    cfsetispeed(vars->tty, (speed_t)B115200);

    /* Setting other Port Stuff */
    vars->tty->c_cflag &= ~PARENB;            // Make 8n1
    vars->tty->c_cflag &= ~CSTOPB;
    vars->tty->c_cflag &= ~CSIZE;
    vars->tty->c_cflag |= CS8;

    vars->tty->c_cflag &= ~CRTSCTS;           // no flow control
    vars->tty->c_cc[VMIN] = 0;                  // read doesn't block
    vars->tty->c_cc[VTIME] = 5;                  // 0.5 seconds read timeout
    vars->tty->c_cflag |= CREAD | CLOCAL;     // turn on READ & ignore ctrl lines
    vars->tty->c_iflag &= ~(IXON | IXOFF | IXANY);// turn off s/w flow ctrl
    vars->tty->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    vars->tty->c_oflag &= ~OPOST;              // make raw

    /* Make raw */
    cfmakeraw(vars->tty);

    /* Flush Port, then applies attributes */
    printf("flushing port\n");
    tcflush(vars->descriptor, TCIFLUSH);

    if (tcsetattr(vars->descriptor, TCSANOW, vars->tty) != 0)
    {
        return 0;
    }

    vars->is_connected = 1;

    // delay a bit to allow the endpoint to wake up.
    delay_ms(100);

    return 1;
}

//*******************************************************************
// uart_isConnected()
//
// A helper function that returns if the UART is connected
//
// parameters:
//	  vars - a data struct used for all uart functions
// returns:
//    cbool - whether the connection was successful
cbool uart_isConnected(uart_struct* vars)
{
    return vars->is_connected;
}

//*******************************************************************
// uart_flush()
//
// This function flushes the buffer.
//
// parameters:
//	  vars - a data struct used for all uart functions
// returns:
//    cbool - whether the buffer was successfuly flushed
cbool uart_flush(uart_struct* vars)
// flush all characters from the buffer
{
    if (vars->is_connected==0) return 0;
    char ch;
    while (uart_readCh(vars,&ch)) {};
    return 1;
}

//*******************************************************************
// uart_readCh()
//
// This function reads a char from the buffer.
//
// parameters:
//	  vars - a data struct used for all uart functions
//    ch - a pointer to where the read-in char can be stored
// returns:
//    cbool - whether the read was successful
cbool uart_readCh(uart_struct* vars,char* ch)
{
    if (vars->is_connected==0) return 0;
    fd_set rfds;
    struct timeval tv = { .tv_sec = 0, .tv_usec = 0 };
    FD_ZERO(&rfds);
    FD_SET(vars->descriptor, &rfds);
    
    // non-blocking call to wait until we have data
    int ready = select(vars->descriptor + 1, &rfds, NULL, NULL, &tv);

    if (ready && FD_ISSET(vars->descriptor, &rfds)) {
        size_t len = 0;
        ioctl(vars->descriptor, FIONREAD, &len);

        if (len == 0) {
            vars->is_connected = 0;
            return 0;
        }

        // While we have data, collect it
        if (read(vars->descriptor, ch, 1) > 0) {
            return 1;
        }

    }
    return 0;
}

//*******************************************************************
// uart_writeCh()
//
// This function writes a char out to the buffer.
//
// parameters:
//	  vars - a data struct used for all uart functions
//    ch - the char being sent out to the serial port
// returns:
//    cbool - whether the write was successful
cbool uart_writeCh(uart_struct* vars,char ch)
{
    if (vars->is_connected==0) return 0;
    if (write(vars->descriptor, &ch, 1) == 0) return 0;
    return 1;
}

//*******************************************************************
// uart_writeBuffer()
//
// This function writes a buffer of chars (char*, str, char[])
//
// parameters:
//	  vars - a data struct used for all uart functions
//    buffer - the chars being written out
//    len - the length of the buffer
// returns:
//    cbool - whether the write was successful
cbool uart_writeBuffer(uart_struct* vars, const void* buffer, unsigned int len)
{
    if (vars->is_connected==0) return 0;
    if (write(vars->descriptor, buffer, len) != len) return 0;
    return 1;
}

//*******************************************************************
// uart_readCh()
//
// This function frees the memory taken by the serial binding
//
// parameters:
//	  vars - a data struct used for all uart functions
// returns:
//    cbool - whether the close was successful
cbool uart_close(uart_struct* vars)
{
    if (vars->is_connected==0) return 0;
    close(vars->descriptor);
    if (vars->tty) free(vars->tty);
    return 1;
}