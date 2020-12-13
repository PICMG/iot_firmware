//*******************************************************************
//    mctp.cpp
//
//    This file contains implementation of a MCTP serial driver 
//    class which is intended to be used as part of 
//    the PICMG PLDM library reference code. This class contains
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
extern bool          uart_haschar();
extern unsigned char uart_getchar();
extern void          uart_writechar(unsigned char);

using namespace std;

//#defines for MCTP data transmission
#define MCTPSER_WAITING_FOR_SYNC 0
#define MCTPSER_GETTING_REV      1
#define MCTPSER_BYTECOUNT        2
#define MCTPSER_VERSION          4
#define MCTPSER_DESTID           5
#define MCTPSER_SOURCEID         6
#define MCTPSER_FLAGS			 7
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

//*******************************************************************
// MctpSerialDriver()
//
// Constructor for the MctpSerialDriver Class - implements and
// initializes UART with default linux USB pin.
MctpSerialDriver::MctpSerialDriver() {
    uart.initialize("/dev/ttyUSB0");
}

//*******************************************************************
// MctpSerialDriver()
//
// Destructor for the MctpSerialDriver Class
MctpSerialDriver::~MctpSerialDriver() {

}

//*******************************************************************
// isPacketAvailable()
//
// This is a helper function that checks whether there is
// a packet available in the buffer
//
// parameters:
//	  none
// returns:
//    boolean - whether there is a packet available
bool  MctpSerialDriver::isPacketAvailable() {
	return mctp_packet_ready;
}

//*******************************************************************
// getPacket()
//
// returns a packet from the buffer
//
// parameters:
//	  none
// returns:
//    rxBuffer - the buffer's contents, usually a MCTP packet
unsigned char* MctpSerialDriver::getPacket() {
	return rxBuffer;
}

//*******************************************************************
// isPacketAvailable()
//
// This is a finite state machine takes in chars from the serial port
// and builds them into a MCTP packet and validates the packet.
//
// parameters:
//	  none
// returns:
//    void
void  MctpSerialDriver::updateRxFSM() {
	static unsigned char mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
	static unsigned int byte_count = 0;
	unsigned int fcs_msg;

    // reading in char from serial port
	unsigned char ch;
	if (!uart.readCh((char*)&ch)) return;

    //building packet FSM
	switch (mctp_serial_state) {
	case MCTPSER_WAITING_FOR_SYNC:
    // checking sync char for start of packet
		if (ch == SYNC_CHAR) {
			mctp_serial_state = MCTPSER_GETTING_REV;
			fcs = frameCheckSequence.calcFcs(frameCheckSequence.INITFCS, &ch, 1);
		}
		break;
	case MCTPSER_GETTING_REV:
    // checking revision number. This is currently set to 0x01
		if (ch == MCTP_SERIAL_REV) {
			mctp_serial_state = MCTPSER_BYTECOUNT;
			fcs = frameCheckSequence.calcFcs(fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_BYTECOUNT:
    // checking bytecount. This number should be the data 
	// payload size plus 4 bytes
		if (ch > 0x4) {
			byte_count = ch - 4;
			mctp_serial_state = MCTPSER_VERSION;
			rxInsertionIdx = 0;
			fcs = frameCheckSequence.calcFcs(fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_VERSION:
    // checking version. This is currently set to 0x01
		if (ch == 0x1) {
			mctp_serial_state = MCTPSER_DESTID;
			fcs = frameCheckSequence.calcFcs(fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_DESTID:
    // checking destination number. This is by default set to 0x00
		if (ch == 0) {
			mctp_serial_state = MCTPSER_SOURCEID;
			fcs = frameCheckSequence.calcFcs(fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_SOURCEID:
    // checking source number. This is by default set to 0x00
		if (ch == 0) {
			mctp_serial_state = MCTPSER_FLAGS;
			fcs = frameCheckSequence.calcFcs(fcs, &ch, 1);
		}
		else mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	case MCTPSER_FLAGS:
    // Checking flags. Refer to base specification for details. 
	// These flags should be set to 0xC8
		if (ch == 0xC8) {
			mctp_serial_state = MCTPSER_BODY;
			fcs = frameCheckSequence.calcFcs(fcs, &ch, 1);
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
			rxBuffer[rxInsertionIdx] = ch;
			rxInsertionIdx = rxInsertionIdx + 1;
			fcs = frameCheckSequence.calcFcs(fcs, &ch, 1);

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
			rxBuffer[rxInsertionIdx] = ch + 0x20;
			rxInsertionIdx = rxInsertionIdx + 1;
			fcs = frameCheckSequence.calcFcs(fcs, &ch, 1);

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
			if (fcs==fcs_msg) mctp_packet_ready = true;
		}
		mctp_serial_state = MCTPSER_WAITING_FOR_SYNC;
		break;
	}
}

//*******************************************************************
// transmitFrameStart()
//
// This function builds the MCTP packet header and puts it into the buffer.
//
// parameters:
//	  totallength - the length of the message being transmitted plus 4 bytes
// returns:
//    void
void  MctpSerialDriver::transmitFrameStart(unsigned char totallength) {
	txfcs = frameCheckSequence.INITFCS;
	unsigned char ch = SYNC_CHAR;
	uart.writeCh(ch);  // mctp synchronization character;
	txfcs = frameCheckSequence.calcFcs(txfcs, &ch, 1);
    ch = 0x01;
	uart.writeCh(ch);  // mctp version
	txfcs = frameCheckSequence.calcFcs(txfcs, &ch, 1);

	// transmit the message length
	uart.writeCh(totallength);
	txfcs = frameCheckSequence.calcFcs(txfcs, &totallength, 1);

	// transmit the MCTP media-independent header
	unsigned char hdr[] = { 0x01, 0x00, 0x00, 0xC8 };
	uart.writeBuffer(hdr,4);
	txfcs = frameCheckSequence.calcFcs(txfcs, &hdr[0], 4);
}

//*******************************************************************
// transmitFrameData()
//
// This function builds the MCTP packet body and puts it into the buffer.
//
// parameters:
//	  data -  the message being transmitted
//    length - the length of the data in bytes
// returns:
//    void
void  MctpSerialDriver::transmitFrameData(unsigned char *data, unsigned int size) {
	for (unsigned int i = 0; i < size; i++) {
		// write the character
		uart.writeCh(data[i]);
		// if the character is a sync or an escape, write the escape sequence
		if ((data[i] == SYNC_CHAR) || (data[i] == ESCAPE_CHAR)) {
			uart.writeCh(ESCAPE_CHAR);
			uart.writeCh(data[i]-0x20);
		}
	}
	// update the fcs
	txfcs = frameCheckSequence.calcFcs(txfcs, data, size);
}

//*******************************************************************
// transmitFrameEnd()
//
// This function builds the MCTP packet footer and puts it into the buffer.
//
// parameters:
//	  none
// returns:
//    void
void  MctpSerialDriver::transmitFrameEnd() {
	// transmit the message length in big-endian format
	uart.writeCh(txfcs >> 8);
	uart.writeCh(txfcs & 0xff);
	// mctp synchronization character
	uart.writeCh(SYNC_CHAR);
}
