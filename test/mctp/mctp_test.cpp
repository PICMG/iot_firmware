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

class TestDriver{
    private:

    public:
    unsigned char* transmitPacket(unsigned char* ch, unsigned int length){
            static MctpSerialDriver mctpSerial;
            mctpSerial.transmitFrameStart(length+4);
            mctpSerial.transmitFrameData(ch,length);
            mctpSerial.transmitFrameEnd();

            while(!mctpSerial.isPacketAvailable()){
                mctpSerial.updateRxFSM();
            }
            return mctpSerial.getPacket();
    }
};

int main()
{
	static TestDriver tester;

    unsigned char* ch = (unsigned char*)("ABCDEFGHIJKLMNOPQRSTUVWXYq");

    unsigned int chSize = 26;

    std::cout<<tester.transmitPacket(ch,chSize)<<std::endl;

    return 0;
}
