#ifndef UARTCONNECTION_H
#define UARTCONNECTION_H
#include <termios.h>
#include <string.h>

class uartConnection
{
public:
    uartConnection();
    virtual ~uartConnection();

    bool initialize(const char*);
    bool isConnected();
    bool sendReadRequest(unsigned regnum);
    bool flush();
    bool sendWriteRequest(unsigned regnum, char* str);
    bool sendWriteRequest(unsigned regnum, float* val);
    bool getResponse(char* buffer);  // wait for a response from the uart - return false on timeout
    bool readCh(char* ch); // read one character from the device
    bool writeCh(char ch); // write one character to the device
    bool writeBuffer(const void* buffer, unsigned int len);
    bool writeHex8(unsigned char num);
protected:
private:
    int descriptor;        // file descriptor for the usb device
    bool is_connected;
    struct termios tty;
};

void delay_ms(float ms);

#endif // UARTCONNECTION_H
