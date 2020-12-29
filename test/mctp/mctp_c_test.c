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

    //testing sending packet
    unsigned char* ch = (unsigned char*)("AB}CD");

    unsigned int chSize = 5;

    mctp_struct mctp1;
    mctp_init(uart_handle, &mctp1);
    mctp_transmitFrameStart(&mctp1,chSize+4);
    mctp_transmitFrameData(&mctp1,ch,chSize);
    mctp_transmitFrameEnd(&mctp1);
    
    while(mctp_isPacketAvailable(&mctp1)==0){
        mctp_updateRxFSM(&mctp1);
    }
    unsigned char*packet = mctp_getPacket(&mctp1);
    packet[10] = 0; 
    printf("%s",mctp_getPacket(&mctp1));
    printf("\n");
    
    mctp_close(&mctp1);
    
    return 0;
}
