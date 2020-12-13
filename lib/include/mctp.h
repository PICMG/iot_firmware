//*******************************************************************
//    mctp.h
//
//    This file provides definitions for MCTP data transfer 
//    protocol. This header is intended to be used as part of 
//    the PICMG PLDM library reference code. 
//    
//    Portions of this code are based on the Management Component Transport
//    Protocol (MCTP) specifications from the Distributed Management Task Force 
//    (DMTF).  More information about MCTP can be found on the DMTF
//    web site (www.dmtf.org).
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
#pragma once

#include <iostream>

#include "uartconnection.h"
#include "fcs.h"

// endpoint command codes
#define CMD_RESERVED                       0x00
#define CMD_GET_MCTP_VERSION_SUPPORT       0x04
#define CMD_GET_MESSAGE_TYPE_SUPPORT       0x05

#define MCTP_BUFFER_SIZE 128

/******************************************************************
* Serial Driver Class structure
*/
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
