//*******************************************************************
//    mctp_c_test.c
//
//    This file contains the test code for mctp data transfer protocol.
//    When run, the program will test MCTP packets sent through a UART
//    serial port and compare them to their expected result. 
//
//    Hardware Requirements:
//    A loopback cable attached to the machine's usb
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
#include <sys/ioctl.h>

#include "uart.h"
#include "mctp.h"
#include "fcs.h"

//*******************************************************************
// testPacket()
//
// A helper function that tests a packet against its expected outcome
//
// parameters:
//    msgSize -  the size of the message in bytes.
//    msg - the message being transmitted through mctp.
//    uart_handle - the descriptor for the uart.
// returns:
//    Whether or not the test was successful.
int testPacket(int msgSize, unsigned char* msg, int uart_handle, mctp_struct* mctp1)
{
    // send the message
    mctp_transmitFrameStart(mctp1,msgSize+4);
    mctp_transmitFrameData(mctp1,msg,msgSize);
    mctp_transmitFrameEnd(mctp1);
    // update the fsm until the packet is ready
    while(mctp_isPacketAvailable(mctp1)==0){
        mctp_updateRxFSM(mctp1);
    }
    // store the packet in a char*
    unsigned char*packet = mctp_getPacket(mctp1);
    packet[msgSize] = 0;
    printf("%s",msg);
    printf("\n");
    printf("%s",packet);
    printf("\n");
    
    // compare the packet to the message and return.
    if(strcmp(((const char*) packet),((const char*) msg))==0){
        printf("packet successful\n");
        return 1;
    }
    printf("packet failed\n");
    return 0;
}

//*******************************************************************
// main()
//
// Main function. Runs executable code.
//
// parameters:
//    argc - the number of arguments supplied
//	  argv - the linux usb port
// returns:
//    0
int main(int argc, char* argv[])
{
    if (argc!=2) {
        printf("invalid number of arguments.\n");
        printf("syntax: mctp_c_test port\n");
    }
    
    // initilaize the uart 
    int uart_handle = uart_init(argv[1]);
    if (uart_handle<=0) {
        printf("unable to connect to uart port\n");
        return 0;
    }

    // initilize mctp
    mctp_struct mctp1;
    mctp_init(uart_handle, &mctp1);
    
    // testing sending a packet with 4 chars
    testPacket(4,(unsigned char*) "ABCD",uart_handle,&mctp1);
    // testing sending another packet with 4 chars
    testPacket(4,(unsigned char*) "EFGH",uart_handle,&mctp1);
    // testing sending a packet with 26 chars
    testPacket(26,(unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZ",uart_handle,&mctp1);
    // testing sending a packet with a sync char in the middle
    testPacket(5,(unsigned char*) "AB~CD",uart_handle,&mctp1);
    // testing sending a packet with an escape char in the middle
    testPacket(5,(unsigned char*) "AB}CD",uart_handle,&mctp1);
    // testing sending a packet with an assortment of different characters
    testPacket(11,(unsigned char*) "Ab&{}jiS~IL",uart_handle,&mctp1);

    mctp_close(&mctp1);
    

    return 0;
}
