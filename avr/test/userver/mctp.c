//*******************************************************************
//    mctp.c
//
//    This file contains implementation of a MCTP serial driver 
//    which is intended to be used as part of 
//    the PICMG PLDM library reference code. This file contains
//	  a state machine that recieves a char stream and outputs and
//    validates MCTP packets.
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
#include "mctp.h"
#include "fcs.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "avr/io.h"

// context for the MCTP Interface
mctp_struct mctp_context;

//*******************************************************************
// mctp_init()
//
// Initializer for mctp - implements and
// initializes UART with default linux USB pin.
//
// parameters:
//    handle - a handle for the uart to transmit with
//	  vars - a data struct used for all mctp functions
// returns:
//    none
void mctp_init(){
	mctp_context.mctp_packet_ready=0;
	mctp_context.discovered = 0;
	mctp_context.last_msg_type = 0;
}

//*******************************************************************
// mctp_sendNoWait()
//
// This function sends a MCTP packet without waiting for response.
// parameters:
//    length - the length of the message
//    msg - the message being transmitted
//	  vars - a data struct used for all mctp functions
// returns:
//    whether or not the packet was sent successfully
unsigned char mctp_sendNoWait(unsigned int length, unsigned char* msg, unsigned char mctp_message_type){
	// send packet
	mctp_transmitFrameStart(length+5,mctp_message_type);
	mctp_transmitFrameData(msg,length);
	mctp_transmitFrameEnd();
	return 1;
}

//*******************************************************************
// mctp_isPacketAvailable()
//
// This is a helper function that checks whether there is
// a packet available in the buffer
//
// parameters:
//	  vars - a data struct used for all mctp functions
// returns:
//    cbool - whether there is a packet available
unsigned char  mctp_isPacketAvailable(mctp_struct* vars) {
	return mctp_context.mctp_packet_ready;
}

//*******************************************************************
// mctp_getPacket()
//
// returns a packet from the buffer
//
// parameters:
//	  vars - a data struct used for all mctp functions
// returns:
//    rxBuffer - the buffer's contents, usually a MCTP packet
unsigned char* mctp_getPacket(mctp_struct* vars) {
	mctp_context.mctp_packet_ready = 0;
	return mctp_context.rxBuffer;
}

//*******************************************************************
// mctp_processControlMessage()
//
// This is a helper function that processess any MCTP control messages
// that are received.
static void mctp_processControlMessage(mctp_struct* vars)
{
	switch (mctp_context.rxBuffer[1]) {
		case CMD_SET_ENDPOINT_ID:
			break;
		case CMD_GET_ENDPOINT_ID:
			break;
		case CMD_GET_MCTP_VERSION_SUPPORT:
			break;
		case CMD_GET_MESSAGE_TYPE_SUPPORT:
			break;
		case CMD_DISCOVERY_NOTIFY:
			mctp_context.discovered = 1;
			break;
		default:
			break;
	}
}

//*******************************************************************
// mctp_updateRxFSM()
//
// This is a finite state machine takes in chars from the serial port
// and builds them into a MCTP packet and validates the packet.
//
// parameters:
//	  vars - a data struct used for all mctp functions
// returns:
//    void
void  mctp_updateRxFSM(mctp_struct* vars) {
	static unsigned char mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
	static unsigned int byte_count = 0;
	static unsigned int fcs_msg = 0;

    // reading in char from serial port
	unsigned char ch;

	if (uart_readCh((char*)&ch)==0) return;

    //building packet FSM
	switch (mctp_serial_state) {
	case MCTPSER_WAITING_FOR_SYNC:
    // checking sync char for start of packet
		if (ch == SYNC_CHAR) {
			mctp_serial_state = MCTPSER_GETTING_REV;
			mctp_context.fcs = fcs_calcFcs(INITFCS, &ch, 1);
		}
		break;
	case MCTPSER_GETTING_REV:
    // checking revision number. This is currently set to 0x01
		if (ch == MCTP_SERIAL_REV) {
			mctp_serial_state = MCTPSER_BYTECOUNT;
			mctp_context.fcs = fcs_calcFcs(mctp_context.fcs, &ch, 1);
		} 
		else if (ch == SYNC_CHAR) {
			mctp_context.fcs = fcs_calcFcs(INITFCS, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_BYTECOUNT:
    // checking bytecount. This number should be the data 
	// payload size plus 5 bytes
		if (ch > 0x4) {
			byte_count = ch - 5;
			mctp_serial_state = MCTPSER_VERSION;
			mctp_context.rxInsertionIdx = 0;
			mctp_context.fcs = fcs_calcFcs(mctp_context.fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_VERSION:
    // checking version. This is currently set to 0x01
		if (ch == 0x1) {
			mctp_serial_state = MCTPSER_DESTID;
			mctp_context.fcs = fcs_calcFcs(mctp_context.fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_DESTID:
    // checking destination number. This is by default set to 0x00
		if (ch == 0) {
			mctp_serial_state = MCTPSER_SOURCEID;
			mctp_context.fcs = fcs_calcFcs(mctp_context.fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_SOURCEID:
    // checking source number. This is by default set to 0x00
		if (ch == 0) {
			mctp_serial_state = MCTPSER_FLAGS;
			mctp_context.fcs = fcs_calcFcs(mctp_context.fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_FLAGS:
    // Checking flags. Refer to base specification for details. 
	// These flags should be set to 0xC8
		if (ch == 0xC8) {
			mctp_serial_state = MCTPSER_CMD;
			mctp_context.fcs = fcs_calcFcs(mctp_context.fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_CMD:
		if ((ch & 0xFE)==0) {
			mctp_serial_state = MCTPSER_BODY;
			mctp_context.last_msg_type = ch;
			mctp_context.fcs = fcs_calcFcs(mctp_context.fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_BODY:
    // building data payload
		if (ch == ESCAPE_CHAR) {
			mctp_serial_state = MCTPSER_ESCAPE;
		}
		else if (ch == SYNC_CHAR) {
			mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		} else {
			// insert the character in the buffer
			mctp_context.rxBuffer[mctp_context.rxInsertionIdx] = ch;
			mctp_context.rxInsertionIdx = mctp_context.rxInsertionIdx + 1;
			mctp_context.fcs = fcs_calcFcs(mctp_context.fcs, &ch, 1);

			// decrease the byte count
			byte_count--;
			if (byte_count == 0) mctp_serial_state = MCTPSER_FCS_MSB;
		}
		break;
	case MCTPSER_ESCAPE:
    // if escaped sync or escaped escape are detected in data payload,
    // this either replaces it or drops the packet
		if ((ch==ESCAPED_SYNC)||(ch==ESCAPED_ESCAPE)) {
			// insert the character in the buffer
			ch = ch+0x20;
			mctp_context.rxBuffer[mctp_context.rxInsertionIdx] = ch;
			mctp_context.rxInsertionIdx = mctp_context.rxInsertionIdx + 1;
			mctp_context.fcs = fcs_calcFcs(mctp_context.fcs, &ch, 1);

			// decrease the byte count
			byte_count--;
			mctp_serial_state = MCTPSER_BODY;
			if (byte_count == 0) mctp_serial_state = MCTPSER_FCS_MSB;
		} else {
			mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		}
		break;
	case MCTPSER_FCS_MSB:
    // checking FCS most significant bit
		fcs_msg = ((unsigned int)ch) << 8;
		mctp_serial_state = MCTPSER_FCS_LSB;
		break;
	case MCTPSER_FCS_LSB:
    // checking FCS least significant bit
		fcs_msg += ((unsigned int)ch);
		mctp_serial_state = MCTPSER_ENDSYNC;
		break;
	case MCTPSER_ENDSYNC:
    	// checking final sync char. If the FCS matches, then the packet is
    	// declared ready. Otherwise, the packet is dropped.
		if (ch == SYNC_CHAR) {
			if (mctp_context.fcs==fcs_msg) {
				// process MCTP control message (or response) if received
				if (mctp_context.last_msg_type==0) {
					mctp_processControlMessage(vars);
				} 
				else {
					// otherwise, notifiy that a new packet is ready for PLDM
					// interface
					mctp_context.mctp_packet_ready = 1;
				}
			}
		}
		mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	}
}

//*******************************************************************
// mctp_transmitFrameStart()
//
// This function builds the MCTP packet header and puts it into the buffer.
//
// parameters:
//	  vars - a data struct used for all mctp functions
//	  totallength - the length of the message being transmitted (body + mctp serial header)
// returns:
//    void
void  mctp_transmitFrameStart(unsigned char totallength, unsigned char mctp_message_type) {
	mctp_context.txfcs = INITFCS;
	unsigned char ch = SYNC_CHAR;
	uart_writeCh(ch);  // mctp synchronization character;
	mctp_context.txfcs = fcs_calcFcs(mctp_context.txfcs, &ch, 1);
    ch = 0x01;
	uart_writeCh(ch);  // mctp version
	mctp_context.txfcs = fcs_calcFcs(mctp_context.txfcs, &ch, 1);

	// transmit the message length
	uart_writeCh(totallength);
	mctp_context.txfcs = fcs_calcFcs(mctp_context.txfcs, &totallength, 1);

	// transmit the MCTP media-independent header
	// header version = 1,,  destination/source ID = 0, SOM, EOM, TO
	unsigned char hdr[] = { 0x01, 0x00, 0x00, 0xC8 };
	uart_writeBuffer(hdr,4);
	mctp_context.txfcs = fcs_calcFcs(mctp_context.txfcs, &hdr[0], 4);

	// transmit the MCTP message type (0 for command message, 1 for PLDM)
	uart_writeBuffer(&mctp_message_type,1);
	mctp_context.txfcs = fcs_calcFcs(mctp_context.txfcs, &mctp_message_type, 1);
}

//*******************************************************************
// mctp_transmitFrameData()
//
// This function builds the MCTP packet body and puts it into the buffer.
//
// parameters:
//    vars - a data struct used for all mctp functions
//	  data -  the message being transmitted
//    length - the length of the data in bytes
// returns:
//    void
void  mctp_transmitFrameData(unsigned char *data, unsigned int size) {
	for (unsigned int i = 0; i < size; i++) {
		// if the character is a sync or an escape, write the escape sequence
		if ((data[i] == SYNC_CHAR) || (data[i] == ESCAPE_CHAR)) {
			uart_writeCh(ESCAPE_CHAR);
			uart_writeCh(data[i]-0x20);
		} else {
			// write the character
			uart_writeCh(data[i]);
		}
	}
	// update the fcs
	mctp_context.txfcs = fcs_calcFcs(mctp_context.txfcs, data, size);
}

//*******************************************************************
// mctp_transmitFrameEnd()
//
// This function builds the MCTP packet footer and puts it into the buffer.
//
// returns:
//    void
void  mctp_transmitFrameEnd() {
	// transmit the message length in big-endian format
	uart_writeCh(mctp_context.txfcs >> 8);
	uart_writeCh(mctp_context.txfcs & 0xff);
	// mctp synchronization character
	uart_writeCh(SYNC_CHAR);
}

//*******************************************************************
// mctp_close()
//
// This function cleans and frees memory taken by the UART.
//
// parameters:
//	  vars - a data struct used for all mctp functions
// returns:
//    void
void mctp_close(mctp_struct* vars){
	uart_close();
}