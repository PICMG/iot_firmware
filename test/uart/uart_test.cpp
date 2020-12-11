#include <iostream>
#include <stdio.h>
#include <string.h>
#include <ctime>
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

#include "uartconnection.h"
#include "mctp.h"
using namespace std;
/*
int main()
{
	static MctpSerialDriver mctpSerial;

    string str = "Jingle bells Jingle bells Jingle all the way oh what fun it is to ride in a one horse open sleigh";

    unsigned int chSize = str.length();
    std::cout<<chSize<<std::endl;
    mctpSerial.transmitFrameStart(chSize+4);

    mctpSerial.transmitFrameData((unsigned char*)(str.c_str()),chSize);
    mctpSerial.transmitFrameEnd();

    while(!mctpSerial.isPacketAvailable()){
        mctpSerial.updateRxFSM();
    }std::cout<<mctpSerial.getPacket()<<std::endl;
	return 0;
}
*/
