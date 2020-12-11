#include <iostream>
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
#include <sys/ioctl.h>
#include "uartconnection.h"
using namespace std;

#define STATE_WAIT_FOR_START 0
#define STATE_GET_OPERATION  1
#define STATE_GET_ENDPOINT1  2
#define STATE_GET_ENDPOINT2  3
#define STATE_GET_LENGTH1    4
#define STATE_GET_LENGTH2    5
#define STATE_GET_WRDATA     6
#define STATE_SEND_RESPONSE  7

void delay_ms(float ms) {
    time_t start = time(NULL);
    while (difftime(time(NULL), start) < ms / 1000.0) {};
}

uartConnection::uartConnection()
{
    descriptor = 0;
    is_connected = false;
}

uartConnection::~uartConnection()
{
    //dtor
    if (descriptor != 0) close(descriptor);
}

bool uartConnection::initialize(const char* name)
{
    // dont reinitialize if the port is still connected
    if (is_connected) return true;

    if (descriptor) close(descriptor);
    memset(&tty, 0, sizeof tty);

    // attempt to open the USB connection
    cout << "opening " << name << endl;
    descriptor = open(name, O_RDWR | O_NOCTTY);
    cout << descriptor << endl;
    if (descriptor <= 0) {
        cout << "invalid usb port" << endl;
        return false;
    }

    /* get the current attributes for the usb connection */
    if (tcgetattr(descriptor, &tty) != 0) return false;

    /* Set Baud Rate */
    cfsetospeed(&tty, (speed_t)B115200);
    cfsetispeed(&tty, (speed_t)B115200);

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
    cout << "flushing port" << endl;
    tcflush(descriptor, TCIFLUSH);

    if (tcsetattr(descriptor, TCSANOW, &tty) != 0)
    {
        return false;
    }

    is_connected = true;

    // delay a bit to allow the endpoint to wake up.
    delay_ms(100);

    return true;
}

bool uartConnection::isConnected()
{
    return is_connected;
}

bool uartConnection::sendWriteRequest(unsigned endpoint, char* str)
{
    if (!is_connected) return false;
    writeCh(2); writeCh('W');
    writeHex8(endpoint);
    writeHex8(strlen(str));
    if (write(descriptor, str, sizeof(float))) return false;
    return true;
}

bool uartConnection::sendWriteRequest(unsigned endpoint, float* val)
{
    if (!is_connected) return false;
    writeCh(2); writeCh('W');
    writeHex8(endpoint);
    writeHex8(sizeof(float));
    if (write(descriptor, &val, sizeof(float))) return false;
    return true;
}

bool uartConnection::sendReadRequest(unsigned int endpoint)
{
    if (!is_connected) return false;
    writeCh(2); writeCh('R');
    writeHex8(endpoint);
    //delay_ms(1);
    return true;
}

bool uartConnection::flush()
// flush all characters from the buffer
{
    if (!is_connected) return false;
    char ch;
    while (readCh(&ch)) {};
    return true;
}

// attempt to read one character from the device.  If one is not ready, return false.
// Updatet the connection state if it gets lost.
bool uartConnection::readCh(char* ch)
{
    if (!is_connected) return false;
    fd_set rfds;
    struct timeval tv = { .tv_sec = 0, .tv_usec = 0 };
    FD_ZERO(&rfds);
    FD_SET(descriptor, &rfds);

    // non-blocking call to wait until we have data
    int ready = select(descriptor + 1, &rfds, NULL, NULL, &tv);

    if (ready && FD_ISSET(descriptor, &rfds)) {
        size_t len = 0;
        ioctl(descriptor, FIONREAD, &len);

        if (len == 0) {
            is_connected = false;
            return false;
        }

        // While we have data, collect it
        if (read(descriptor, ch, 1) > 0) {
            return true;
        }

    }
    return false;
}

bool uartConnection::writeCh(char ch)
{
    if (!is_connected) return false;
    if (write(descriptor, &ch, 1) == 0) return false;
    return true;
}

bool uartConnection::writeBuffer(const void* buffer, unsigned int len)
{
    if (!is_connected) return false;
    if (write(descriptor, buffer, len) != len) return false;
    return true;
}


bool uartConnection::getResponse(char* buffer)
{
    if (!is_connected) return false;
    unsigned char state = STATE_WAIT_FOR_START;
    unsigned char endpoint = 0;
    unsigned char length = 0;
    unsigned char bytes_read = 0;
    unsigned char ch;

    time_t starttime = time(NULL);

    // wait up to 0.5 seconds for a complete response
    while (difftime(time(NULL), starttime) < 1.5) {
        // return if a connection has been lost
        if (!isConnected()) return false;

        // attempt to read a new character
        if (readCh((char*)&ch)) {
            switch (state) {
            case STATE_WAIT_FOR_START:
                if (ch == 2) state = STATE_GET_OPERATION;
                break;
            case STATE_GET_OPERATION:
                // responses all start with 'Z'
                if (ch == 'Z') {
                    state = STATE_GET_ENDPOINT1;
                }
                else {
                    state = STATE_WAIT_FOR_START;
                }
                break;
            case STATE_GET_ENDPOINT1:
                state = STATE_GET_ENDPOINT2;
                if ((ch >= '0') && (ch <= '9')) {
                    endpoint = ch - '0';
                }
                else if ((ch >= 'a') && (ch <= 'f')) {
                    endpoint = ch - 'a' + 10;
                }
                else if ((ch >= 'A') && (ch <= 'F')) {
                    endpoint = ch - 'A' + 10;
                }
                else {
                    state = STATE_WAIT_FOR_START;
                }
                break;
            case STATE_GET_ENDPOINT2:
                state = STATE_GET_LENGTH1;
                endpoint = endpoint << 4;
                if ((ch >= '0') && (ch <= '9')) {
                    endpoint += ch - '0';
                }
                else if ((ch >= 'a') && (ch <= 'f')) {
                    endpoint += ch - 'a' + 10;
                }
                else if ((ch >= 'A') && (ch <= 'F')) {
                    endpoint += ch - 'A' + 10;
                }
                else {
                    state = STATE_WAIT_FOR_START;
                }
                break;
            case STATE_GET_LENGTH1:
                state = STATE_GET_LENGTH2;
                if ((ch >= '0') && (ch <= '9')) {
                    length = ch - '0';
                }
                else if ((ch >= 'a') && (ch <= 'f')) {
                    length = ch - 'a' + 10;
                }
                else if ((ch >= 'A') && (ch <= 'F')) {
                    length = ch - 'A' + 10;
                }
                else {
                    state = STATE_WAIT_FOR_START;
                }
                break;
            case STATE_GET_LENGTH2:
                state = STATE_GET_WRDATA;
                bytes_read = 0;
                length = length << 4;
                if ((ch >= '0') && (ch <= '9')) {
                    length += ch - '0';
                }
                else if ((ch >= 'a') && (ch <= 'f')) {
                    length += ch - 'a' + 10;
                }
                else if ((ch >= 'A') && (ch <= 'F')) {
                    length += ch - 'A' + 10;
                }
                else {
                    state = STATE_WAIT_FOR_START;
                }
                bytes_read = 0;
                break;
            case STATE_GET_WRDATA:
                buffer[bytes_read] = ch;
                bytes_read++;
                if (bytes_read == length) {
                    buffer[bytes_read] = 0;
                    return true;
                }
                break;
            default:
                state = STATE_WAIT_FOR_START;
            }
        }
    } // while not timeout
    cout << "Timeout error" << endl;
    return false;  // timeout error
}

bool uartConnection::writeHex8(unsigned char num)
{
    if (!is_connected) return false;
    unsigned char ch;
    ch = num >> 4;
    (ch <= 9) ? ch = ch + '0' : ch = ch + 'A' - 10;
    writeCh(ch);
    ch = num & 0xf;
    (ch <= 9) ? ch = ch + '0' : ch = ch + 'A' - 10;
    writeCh(ch);
    return true;
}