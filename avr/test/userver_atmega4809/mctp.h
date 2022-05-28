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

#include "uart.h"
#include "fcs.h"

// endpoint command codes
#define CMD_RESERVED                       0x00
#define CMD_SET_ENDPOINT_ID                0x01
#define CMD_GET_ENDPOINT_ID                0x02
#define CMD_GET_MCTP_VERSION_SUPPORT       0x04
#define CMD_GET_MESSAGE_TYPE_SUPPORT       0x05
#define CMD_DISCOVERY_NOTIFY               0x0d

#define MCTP_BUFFER_SIZE 128

//#defines for MCTP data transmission
#define MCTPSER_WAITING_FOR_SYNC 0
#define MCTPSER_GETTING_REV      1
#define MCTPSER_BYTECOUNT        2
#define MCTPSER_VERSION          4
#define MCTPSER_DESTID           5
#define MCTPSER_SOURCEID         6
#define MCTPSER_FLAGS			 7
#define MCTPSER_CMD             13
#define MCTPSER_BODY             8
#define MCTPSER_ESCAPE           9
#define MCTPSER_FCS_MSB         10
#define MCTPSER_FCS_LSB         11
#define MCTPSER_ENDSYNC         12

#define ESCAPE_CHAR             0x7D
#define SYNC_CHAR               0x7E
#define ESCAPED_ESCAPE          0x5D
#define ESCAPED_SYNC            0x5E
#define MCTP_SERIAL_REV         0x01


// struct for data transfer
typedef struct{
    unsigned char rxBuffer[MCTP_BUFFER_SIZE];
	unsigned char rxInsertionIdx;
	unsigned int  fcs;
	unsigned int  txfcs;
	unsigned char mctp_packet_ready;
	unsigned char discovered;
	unsigned char last_msg_type;
} mctp_struct;

extern mctp_struct mctp_context;

// function definitions
void  mctp_init();
unsigned char mctp_sendAndWait(unsigned int, unsigned char*, unsigned char mctp_message_type);
unsigned char mctp_sendNoWait(unsigned int, unsigned char*, unsigned char mctp_message_type);
unsigned char mctp_isPacketAvailable();
unsigned char* mctp_getPacket();
void  mctp_updateRxFSM();
void  mctp_transmitFrameStart(unsigned char totallength, unsigned char mctp_message_type);
void  mctp_transmitFrameData(unsigned char*, unsigned int);
void  mctp_transmitFrameEnd();
void  mctp_close();
