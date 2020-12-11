//Description: header file for MCTP data transfer protocol
//Authors: Douglas Sandy, David Sandy
//Copyright 2020 PICMG all rights reserved

#pragma once

#include <iostream>

#include "uartconnection.h"
#include "fcs.h"

// endpoint command codes
#define CMD_RESERVED                       0x00
#define CMD_GET_MCTP_VERSION_SUPPORT       0x04
#define CMD_GET_MESSAGE_TYPE_SUPPORT       0x05

#define MCTP_BUFFER_SIZE 128

class MctpSerialDriver {
private:
    unsigned char rxBuffer[MCTP_BUFFER_SIZE];
	unsigned char rxInsertionIdx = 0;
	unsigned int  fcs;
	unsigned int  txfcs;
	bool mctp_packet_ready;
    uartConnection uart;
    FrameCheckSequence frameCheckSequence;
public:
	MctpSerialDriver();
	~MctpSerialDriver();
	bool  isPacketAvailable();
	unsigned char* getPacket();
	void  updateRxFSM();
	void  transmitFrameStart(unsigned char totallength);
	void  transmitFrameData(unsigned char*, unsigned int);
	void  transmitFrameEnd();
};
