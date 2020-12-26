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
static void delay_ms(float ms) {
    time_t start = time(NULL);
    while (difftime(time(NULL), start) < ms / 1000.0) {};
}

//*******************************************************************
// uart_init()
//
// This function initializes the UART with the provided linux USB pin.
//
// parameters:
//	  handle - a data struct used for all uart functions
//    name - the name of the linux USB pin, usually "/dev/ttyUSB0"
// returns:
//    returns the handle for the serial port device, or -1 on error
int uart_init(const char* name)
{
    struct termios tty;
    int handle;

    memset((void*)(&tty), 0, sizeof(struct termios));

    // attempt to open the USB connection
    printf("opening ");
    printf("%s",name);
    printf("\n");
    handle = open(name, O_RDWR | O_NOCTTY);
    printf("%d",(handle));
    printf("\n");
    if(handle<=0)
    {
        printf("invalid usb port\n");
        return -1;
    }

    /* get the current attributes for the usb connection */
    if (tcgetattr(handle, &tty) != 0) return 0;

    /* Set Baud Rate */
    cfsetospeed(&tty, (speed_t)B9600);
    cfsetispeed(&tty, (speed_t)B9600);

    /* Setting other Port Stuff */
    tty.c_cflag &= ~PARENB;            // Make 8n1
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag &= ~CRTSCTS;           // no flow control
    tty.c_cc[VMIN] = 0;                  // read doesn't block
    tty.c_cc[VTIME] = 5;                  // 0.5 seconds read timeout
    tty.c_cflag |= CREAD | CLOCAL;     // turn on READ & ignore ctrl lines
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);// turn off s/w flow ctrl
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    tty.c_oflag &= ~OPOST;              // make raw

    /* Make raw */
    cfmakeraw(&tty);

    /* Flush Port, then applies attributes */
    printf("flushing port\n");
    tcflush(handle, TCIFLUSH);

    if (tcsetattr(handle, TCSANOW, &tty) != 0)
    {
        return -1;
    }

    // delay a bit to allow the endpoint to wake up.
    delay_ms(5000);

    return handle;
}

//*******************************************************************
// uart_flush()
//
// This function flushes the buffer.
//
// parameters:
//	  handle - a file descriptor to the open uart port
// returns:
//    unsigned char - whether the buffer was successfuly flushed
unsigned char uart_flush(int handle)
// flush all characters from the buffer
{
    char ch;
    while (uart_readCh(handle,&ch)) {};
    return 1;
}

//*******************************************************************
// uart_readCh()
//
// This function reads a char from the buffer.
//
// parameters:
//	  handle - a data struct used for all uart functions
//    ch - a pointer to where the read-in char can be stored
// returns:
//    unsigned char - whether the read was successful
unsigned char uart_readCh(int handle,char* ch)
{
    fd_set rfds;
    struct timeval tv = { .tv_sec = 0, .tv_usec = 0 };
    FD_ZERO(&rfds);
    FD_SET(handle, &rfds);
    
    // non-blocking call to wait until we have data
    int ready = select(handle + 1, &rfds, NULL, NULL, &tv);

    if (ready && FD_ISSET(handle, &rfds)) {
        size_t len = 0;
        ioctl(handle, FIONREAD, &len);

        if (len == 0) {
            return 0;
        }

        // if we have data, collect it
        if (read(handle, ch, 1) > 0) {
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
//	  handle - a data struct used for all uart functions
//    ch - the char being sent out to the serial port
// returns:
//    unsigned char - whether the write was successful
unsigned char uart_writeCh(int handle,char ch)
{
    if (write(handle, &ch, 1) == 0) return 0;
    return 1;
}

//*******************************************************************
// uart_writeBuffer()
//
// This function writes a buffer of chars (char*, str, char[])
//
// parameters:
//	  handle - a data struct used for all uart functions
//    buffer - the chars being written out
//    len - the length of the buffer
// returns:
//    unsigned char - whether the write was successful
unsigned char uart_writeBuffer(int handle, const void* buffer, unsigned int len)
{
    if (write(handle, buffer, len) != len) return 0;
    return 1;
}

//*******************************************************************
// uart_readCh()
//
// This function frees the memory taken by the serial binding
//
// parameters:
//	  handle - a data struct used for all uart functions
// returns:
//    unsigned char - whether the close was successful
unsigned char uart_close(int handle)
{
    close(handle);
    return 1;
}