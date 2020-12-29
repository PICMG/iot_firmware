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
void mctp_init(int handle, mctp_struct* vars){
	vars->uart_handle = handle;
	vars->mctp_packet_ready=0;
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
	return vars->mctp_packet_ready;
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
	return vars->rxBuffer;
}

//*******************************************************************
// mctp_isPacketAvailable()
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
	if (uart_readCh(vars->uart_handle,(char*)&ch)==0) return;

    //building packet FSM
	switch (mctp_serial_state) {
	case MCTPSER_WAITING_FOR_SYNC:
    // checking sync char for start of packet
		if (ch == SYNC_CHAR) {
			mctp_serial_state = MCTPSER_GETTING_REV;
			vars->fcs = fcs_calcFcs(INITFCS, &ch, 1);
		}
		break;
	case MCTPSER_GETTING_REV:
    // checking revision number. This is currently set to 0x01
		if (ch == MCTP_SERIAL_REV) {
			mctp_serial_state = MCTPSER_BYTECOUNT;
			vars->fcs = fcs_calcFcs(vars->fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_BYTECOUNT:
    // checking bytecount. This number should be the data 
	// payload size plus 4 bytes
		if (ch > 0x4) {
			byte_count = ch - 4;
			mctp_serial_state = MCTPSER_VERSION;
			vars->rxInsertionIdx = 0;
			vars->fcs = fcs_calcFcs(vars->fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_VERSION:
    // checking version. This is currently set to 0x01
		if (ch == 0x1) {
			mctp_serial_state = MCTPSER_DESTID;
			vars->fcs = fcs_calcFcs(vars->fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_DESTID:
    // checking destination number. This is by default set to 0x00
		if (ch == 0) {
			mctp_serial_state = MCTPSER_SOURCEID;
			vars->fcs = fcs_calcFcs(vars->fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_SOURCEID:
    // checking source number. This is by default set to 0x00
		if (ch == 0) {
			mctp_serial_state = MCTPSER_FLAGS;
			vars->fcs = fcs_calcFcs(vars->fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_FLAGS:
    // Checking flags. Refer to base specification for details. 
	// These flags should be set to 0xC8
		if (ch == 0xC8) {
			mctp_serial_state = MCTPSER_BODY;
			vars->fcs = fcs_calcFcs(vars->fcs, &ch, 1);
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
			vars->rxBuffer[vars->rxInsertionIdx] = ch;
			vars->rxInsertionIdx = vars->rxInsertionIdx + 1;
			vars->fcs = fcs_calcFcs(vars->fcs, &ch, 1);

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
			vars->rxBuffer[vars->rxInsertionIdx] = ch + 0x20;
			vars->rxInsertionIdx = vars->rxInsertionIdx + 1;
			vars->fcs = fcs_calcFcs(vars->fcs, &ch, 1);

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
			if (vars->fcs==fcs_msg) vars->mctp_packet_ready = 1;
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
//	  totallength - the length of the message being transmitted plus 4 bytes
// returns:
//    void
void  mctp_transmitFrameStart(mctp_struct* vars, unsigned char totallength) {
	vars->txfcs = INITFCS;
	unsigned char ch = SYNC_CHAR;
	uart_writeCh(vars->uart_handle,ch);  // mctp synchronization character;
	vars->txfcs = fcs_calcFcs(vars->txfcs, &ch, 1);
    ch = 0x01;
	uart_writeCh(vars->uart_handle,ch);  // mctp version
	vars->txfcs = fcs_calcFcs(vars->txfcs, &ch, 1);

	// transmit the message length
	uart_writeCh(vars->uart_handle,totallength);
	vars->txfcs = fcs_calcFcs(vars->txfcs, &totallength, 1);

	// transmit the MCTP media-independent header
	unsigned char hdr[] = { 0x01, 0x00, 0x00, 0xC8 };
	uart_writeBuffer(vars->uart_handle,hdr,4);
	vars->txfcs = fcs_calcFcs(vars->txfcs, &hdr[0], 4);
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
void  mctp_transmitFrameData(mctp_struct* vars, unsigned char *data, unsigned int size) {
	for (unsigned int i = 0; i < size; i++) {
		// write the character
		uart_writeCh(vars->uart_handle,data[i]);
		// if the character is a sync or an escape, write the escape sequence
		if ((data[i] == SYNC_CHAR) || (data[i] == ESCAPE_CHAR)) {
			uart_writeCh(vars->uart_handle,ESCAPE_CHAR);
			uart_writeCh(vars->uart_handle,data[i]-0x20);
		}
		//TODO: fix handling of sync char and esc char to match documentation.
	}
	// update the fcs
	vars->txfcs = fcs_calcFcs(vars->txfcs, data, size);
}

//*******************************************************************
// mctp_transmitFrameEnd()
//
// This function builds the MCTP packet footer and puts it into the buffer.
//
// parameters:
//	  vars - a data struct used for all mctp functions
// returns:
//    void
void  mctp_transmitFrameEnd(mctp_struct* vars) {
	// transmit the message length in big-endian format
	uart_writeCh(vars->uart_handle,vars->txfcs >> 8);
	uart_writeCh(vars->uart_handle,vars->txfcs & 0xff);
	// mctp synchronization character
	uart_writeCh(vars->uart_handle,SYNC_CHAR);
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
	uart_close(vars->uart_handle);
}