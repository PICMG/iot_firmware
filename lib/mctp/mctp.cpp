//Description: cpp file for MCTP data transfer protocol
//Authors: Douglas Sandy, David Sandy
//Copyright 2020 PICMG all rights reserved

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
//Function name: MctpSerialDriver
//Description: default constructor that initializes UART with default USB pin
//Parameters:
//Returns:
//Changes:
MctpSerialDriver::MctpSerialDriver() {
    uart.initialize("/dev/ttyUSB0");
}

//*******************************************************************
//Function name: ~MctpSerialDriver
//Description: default destructor
//Parameters:
//Returns:
//Changes:
MctpSerialDriver::~MctpSerialDriver() {

}

//*******************************************************************
//Function name: isPacketAvailable
//Description: checks for whether there is an MCTP packet available and
//             returns whether there is or not as a bool
//Parameters:
//Returns: a boolean statement of whether there is a packet
//Changes:
bool  MctpSerialDriver::isPacketAvailable() {
	return mctp_packet_ready;
}

//*******************************************************************
//Function name: getPacket
//Description: a getter that returns a MCTP packet from the buffer
//Parameters:
//Returns: an MCTP packet as a char*
//Changes:
unsigned char* MctpSerialDriver::getPacket() {
	return rxBuffer;
}

//*******************************************************************
//Function name: updateRxFSM
//Description: a finite state machine that builds a MCTP packet from
//             the serial buffer and checks for errors in the packet
//Parameters:
//Returns:
//Changes:
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
    // checking bytecount. This number should be the data payload size plus 4 bytes
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
    // checking flags. Refer to base specification for details. These flags should be set to 0xC8
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
    //this either replaces it or drops the packet
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
//Function name: transmitFrameStart
//Description: transmits the MCTP packet header to the serial port
//Parameters: totallength- the length of the data in bytes + 4
//Returns:
//Changes:
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
//Function name: transmitFrameData
//Description: transmits the message payload to the serial port
//Parameters: data- the message being transmitted: size- the length of the data in bytes
//Returns:
//Changes:
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
//Function name: transmitFrameEnd
//Description: transmits the MCTP packet footer to the serial port
//Parameters:
//Returns:
//Changes:
void  MctpSerialDriver::transmitFrameEnd() {
	// transmit the message length in big-endian format
	uart.writeCh(txfcs >> 8);
	uart.writeCh(txfcs & 0xff);
	// mctp synchronization character
	uart.writeCh(SYNC_CHAR);
}
