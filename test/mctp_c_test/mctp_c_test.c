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

int main()
{
    unsigned char* ch = (unsigned char*)("ABCDEFGHIJKLMNOPQRSTUVWXYq");

    unsigned int chSize = 26;

    mctp_struct mctp1;
    mctp_init(&mctp1);
    mctp_transmitFrameStart(&mctp1,chSize+4);
    mctp_transmitFrameData(&mctp1,ch,chSize);
    mctp_transmitFrameEnd(&mctp1);
    
    while(mctp_isPacketAvailable(&mctp1)==0){
        mctp_updateRxFSM(&mctp1);
    }
    printf("%s", mctp_getPacket(&mctp1));
    printf("\n");

    return 0;
}
