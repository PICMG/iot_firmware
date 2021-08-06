//*******************************************************************
//    node.cpp
//
//    This file includes implementation for a simulated PLDM connector node.
//    This node behaves like an extenal embedded device, that communicates
//    with PLDM commands and responses.   This is only intended for use 
//    in PLDM testing.
//
//    Portions of this code are based on the Platform Level Data Model
//    (PLDM) specifications from the Distributed Management Task Force 
//    (DMTF).  More information about PLDM can be found on the DMTF
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
#ifndef __AVR_ATmega328P__ 
#define __AVR_ATmega328P__
#endif

#include <avr/pgmspace.h>
#include <avr/io.h>
#include "node.h"
#include "mctp.h"
#include "pldm.h"
#include "pdrdata.h"
#include "entityStepper1.h"
#include "entitySimple1.h"
#include "EventGenerator.h"

static uint8   tid;
static uint8   globalEventEnableState = 0;
static char   eventFifoInsertId = 0;
static char   eventFifoExtractId = 0;

#ifdef UUID
    const unsigned char uuid_bytes[] PROGMEM = {UUID};
#else
    const unsigned char uuid_bytes[] PROGMEM = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
#endif

#define FRU_BYTE_TYPE const unsigned char
#define LINTABLE_TYPE const long
#define PDR_DATA_ATTRIBUTES PROGMEM
#define FRU_DATA_ATTRIBUTES PROGMEM
#define LINTABLE_DATA_ATTRIBUTES PROGMEM

#define UINT8_TYPE  0
#define SINT8_TYPE  1
#define UINT16_TYPE 2
#define SINT16_TYPE 3
#define UINT32_TYPE 4
#define SINT32_TYPE 5

#define KFFA_EFFECTER_ID      10
#define APROFILE_EFFECTER_ID  6
#define VPROFILE_EFFECTER_ID  5
#define PFINAL_EFFECTER_ID    4
#define INTERLOCK_EFFECTER_ID 1
#define TRIGGER_EFFECTER_ID   2
#define START_EFFECTER_ID     3

#define TRIGGER_SENSOR_ID     2
#define INTERLOCK_SENSOR_ID   1
#define MOTOR_STATE_SENSOR_ID 3
#define POSITION_SENSOR_ID    10
#define PERR_SENSOR_ID        5
#define VELOCITY_SENSOR_ID    4
#define VERR_SENSOR_ID        11
#define POS_LIMIT_SENSOR_ID   8
#define NEG_LIMIT_SENSOR_ID   9


#ifdef SERVO_CONTROL_MODE
    static  unsigned char requested_start_state = 2;  // initially stopped
#endif


static void transmitByte(unsigned char data) {
    // send the data to the MCTP buffer in little-endian fashion
    mctp_transmitFrameData(&data,1);
}

static void transmitShort(unsigned int data) {
    // send the data to the MCTP buffer in little-endian fashion
    unsigned char ch = (data&0xff);
    transmitByte(ch);
    data = data>>8;
    ch = (data&0xff);
    transmitByte(ch);
}

static void transmitLong(unsigned long data) {
    // send the data to the MCTP buffer in little-endian fashion
    transmitShort(data);
    data = data>>16;
    transmitShort(data);
}


void node_init() {
//    pdrCount = __pdr_number_of_records;
    tid = 0;
}

//*******************************************************************
// getPdrHeader()
//
// iterate through the repository to find the header for the pdr indexed by
// the given index parameter.
//
// parameters:
//    index - the index of the pdr to get
// returns:
//    a pointer to the pdr header for the specified pdr
static PdrCommonHeader* getPdrHeader(unsigned int index) {
    // adjust the index for the pdr structure - assume records start at 1,
    // and increase sequentially.  0 is a special case which will also retrieve
    // the first record
    if (index > 0) index--;
    PdrCommonHeader* result = (PdrCommonHeader*)(__pdr_data);
    unsigned long offset = 0;
    unsigned int counter = 0;
    while (counter != index) {
        unsigned long part1 = sizeof(PdrCommonHeader);
        unsigned long part2 = pgm_read_dword(&(result->dataLength));
        offset = offset + part1 + part2;
        result = (PdrCommonHeader*)(&__pdr_data[offset]);
        if (pgm_read_byte(&(__pdr_data[offset])) == 0) return 0;
        counter++;
    }
    return result;
}

//*******************************************************************
// getPdrHeader()
//
// iterate through the repository to find the header for the pdr indexed by
// the given index parameter.
//
// parameters:
//    index - the index of the pdr offset to get
// returns:
//    the data offset for the specified pdr
static unsigned int getPdrOffset(unsigned int index) {
    // adjust the index for the pdr structure - assume records start at 1,
    // and increase sequentially.  0 is a special case which will also retrieve
    // the first record
    if (index > 0) index--;

    PdrCommonHeader* hdr = (PdrCommonHeader*)(__pdr_data);
    unsigned int offset = 0;
    unsigned int counter = 0;
    while (counter != index) {
        unsigned long part1 = sizeof(PdrCommonHeader);
        unsigned long part2 = pgm_read_dword(&(hdr->dataLength));
        offset = offset + part1 + part2;
        hdr = (PdrCommonHeader*)(&__pdr_data[offset]);
        if (pgm_read_byte(&(__pdr_data[offset])) == 0) return 0;
        counter++;
    }
    return offset;
}

//*******************************************************************
// getNextRecord()
//
// get the record number for the next record in the repository
//
// parameters:
//    index - the index of the pdr offset to get
// returns:
//    the number of the next record in the repository
static unsigned long  getNextRecord(unsigned long index) {
    unsigned result = 0;
    if (index == 0) result = 2;
    else result = index+1;
    return result;
}

//*******************************************************************
// pdrSize()
//
// returns the total pdr size (including the header)
//
// parameters:
//    index - the index of the pdr to get size of
// returns:
//    the size in bytes of the indexed pdr
static unsigned int pdrSize(unsigned int index) {
    PdrCommonHeader * header = getPdrHeader(index);
    if (!header) return 0;
    unsigned int pdr_size = pgm_read_word(&(header->dataLength)); 
    return pdr_size + sizeof(PdrCommonHeader);
}

//*******************************************************************
// processCommandGetPdr()
//
// processes a getPdr command, implemeting the PLDM state machine for
// packet disassembly, if required.
//
// parameters:
//    rxHeader - a pointer to the request header
//    txHeader - a pointer to the response header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
static void processCommandGetPdr(PldmRequestHeader* rxHeader) 
{
    static char pdrTxState = 0;
    static char crc8;
    static unsigned int pdrNextHandle;
    static unsigned long pdrRecord;
    unsigned int extractionPoint;

    GetPdrCommand* request = (GetPdrCommand*)(mctp_context.rxBuffer + sizeof(*rxHeader));
    unsigned char errorcode = 0;
    switch (pdrTxState) {
    case 0:
        // transfer has not begun yet
        if (request->transferOperationFlag != 0x1)
            errorcode = RESPONSE_INVALID_TRANSFER_OPERATION_FLAG;
        else if (request->recordHandle > PDR_NUMBER_OF_RECORDS)
            errorcode = RESPONSE_INVALID_RECORD_HANDLE;
        else if (request->dataTransferHandle != 0x0000)
            errorcode = RESPONSE_INVALID_DATA_TRANSFER_HANDLE;
        else if (request->recordChangeNumber != 0x0000)
            errorcode = RESPONSE_INVALID_RECORD_CHANGE_NUMBER;
        if (errorcode) {
            // send the error response
            mctp_transmitFrameStart(sizeof(PldmResponseHeader)+sizeof(GetPdrResponse) + 5-1, 1);
            transmitByte(rxHeader->flags1 & 0x7f);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            mctp_transmitFrameData(&errorcode,1);  // response->completionCode = errorcode;
            transmitLong(0);                       // response->nextRecordHandle = 0;
            transmitLong(0);                       // response->NextRecordTransferHandle;
            transmitByte(0);                       // response->transferFlag = 0;
            transmitShort(0);                      // response->responseCount = 0;
            mctp_transmitFrameEnd();
            return;
        }
        if (request->requestCount >= pdrSize(request->recordHandle)) {
            // send the data (single part)
            mctp_transmitFrameStart( sizeof(PldmResponseHeader) + sizeof(GetPdrResponse) + pdrSize(request->recordHandle)+5-1,1);
            transmitByte(rxHeader->flags1 & 0x7f);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            transmitByte(RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
            transmitLong((getNextRecord(request->recordHandle) <= PDR_NUMBER_OF_RECORDS) ?
                getNextRecord(request->recordHandle) : 0);
            transmitLong(0);                      //response->nextDataTransferHandle = 0;
            transmitByte(0x05);                   // response->transferFlag = 0x05;   // start and end
            transmitShort(pdrSize(request->recordHandle)); // response->responseCount = pdrSize(request->recordHandle);
            unsigned int extractionPoint = getPdrOffset(request->recordHandle);
            // insert the pdr
            for (int i = 0;i < pdrSize(request->recordHandle); i++) {
                transmitByte(pgm_read_byte(&(__pdr_data[extractionPoint++])));
            }
            mctp_transmitFrameEnd();
            return;
        }
        // start sending the data (multi-part)
        mctp_transmitFrameStart( sizeof(PldmResponseHeader) + sizeof(GetPdrResponse) + request->requestCount+5-1,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
        transmitLong((getNextRecord(request->recordHandle) <= PDR_NUMBER_OF_RECORDS) ?
                getNextRecord(request->recordHandle) : 0);
        transmitLong(getPdrOffset(request->recordHandle) + request->requestCount);
        transmitByte(0x00);                // response->transferFlag = 0x0;   // start
        transmitShort(request->requestCount); // response->responseCount = request->requestCount;
        extractionPoint = request->dataTransferHandle+getPdrOffset(request->recordHandle);
        // insert the pdr
        crc8 = 0;
        for (int i = 0;i < request->requestCount;i++) {
            unsigned char byte = pgm_read_byte(&(__pdr_data[extractionPoint++])); 
            transmitByte(byte);
            crc8 = calc_new_crc8(crc8, byte);
        }
        mctp_transmitFrameEnd();
        pdrTxState = 1;
        pdrRecord = request->recordHandle;
        pdrNextHandle = getPdrOffset(request->recordHandle) + request->requestCount;
        return;
    case 1:
        // transfer has already begun
        if (request->transferOperationFlag != 0x0)
            errorcode = RESPONSE_INVALID_TRANSFER_OPERATION_FLAG;
        else if (request->recordHandle != pdrRecord)
            errorcode = RESPONSE_INVALID_RECORD_HANDLE;
        else if (request->dataTransferHandle != pdrNextHandle)
            errorcode = RESPONSE_INVALID_DATA_TRANSFER_HANDLE;
        else if (request->recordChangeNumber != 0x0000)
            errorcode = RESPONSE_INVALID_RECORD_CHANGE_NUMBER;
        if (errorcode) {
            // send the error response
            mctp_transmitFrameStart(sizeof(PldmResponseHeader)+sizeof(GetPdrResponse) + 5-1,1);
            transmitByte(rxHeader->flags1 & 0x7f);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            mctp_transmitFrameData(&errorcode,1);  // response->completionCode = errorcode;
            transmitLong(0);                       // response->nextRecordHandle = 0;
            transmitLong(0);                       // response->NextRecordTransferHandle;
            transmitByte(0);                       // response->transferFlag = 0;
            transmitShort(0);                      // response->responseCount = 0;
            mctp_transmitFrameEnd();
            return;
        }
        if (request->requestCount + request->dataTransferHandle >= getPdrOffset(request->recordHandle)+pdrSize(request->recordHandle)+1) {
            // transfer end part of the data
            mctp_transmitFrameStart( sizeof(PldmResponseHeader) + sizeof(GetPdrResponse) + pdrSize(request->recordHandle) -
                (request->dataTransferHandle-getPdrOffset(request->recordHandle)) + 5 - 1 + 1,1);
            transmitByte(rxHeader->flags1 & 0x7f);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            transmitByte(RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
            transmitLong((getNextRecord(request->recordHandle) <= PDR_NUMBER_OF_RECORDS) ?
                getNextRecord(request->recordHandle) : 0);
            transmitLong(0);                      // next data transfer handle
            transmitByte(0x04);                   // response->transferFlag = 0x04;   end
            transmitShort(pdrSize(request->recordHandle) -
                (request->dataTransferHandle-getPdrOffset(request->recordHandle))); 
            unsigned int extractionPoint = request->dataTransferHandle;
            // insert the pdr
            for (int i = 0;i < pdrSize(request->recordHandle) -
                (request->dataTransferHandle-getPdrOffset(request->recordHandle));i++) {
                unsigned char byte = pgm_read_byte(&(__pdr_data[extractionPoint++])); 
                transmitByte(byte);
                crc8 = calc_new_crc8(crc8, byte);
            }
            transmitByte(crc8);
            mctp_transmitFrameEnd();
            pdrTxState = 0;
            return;
        }
        // send the middle data (multi-part)
        mctp_transmitFrameStart( sizeof(PldmResponseHeader) + sizeof(GetPdrResponse) + request->requestCount + 5-1,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
        transmitLong((getNextRecord(request->recordHandle) <= PDR_NUMBER_OF_RECORDS) ?
            getNextRecord(request->recordHandle) : 0);
        transmitLong(request->dataTransferHandle + request->requestCount); // next data transfer handle
        transmitByte(0x01);                   // response->transferFlag = 0x01;   middle
        transmitShort(request->requestCount); 
        unsigned int extractionPoint = request->dataTransferHandle;
        // insert the pdr
        for (int i = 0;i < request->requestCount;i++) {
            unsigned char byte = pgm_read_byte(&(__pdr_data[extractionPoint++])); 
            transmitByte(byte);
            crc8 = calc_new_crc8(crc8, byte);
        }
        mctp_transmitFrameEnd();
        pdrTxState = 1;
        pdrNextHandle = request->dataTransferHandle + request->requestCount;
        return;
    }
}

//*******************************************************************
// processCommandGetFruTable()
//
// processes a getFruTable command, implemeting the PLDM state machine for
// packet disassembly, if required.
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
static void processCommandFruTable(PldmRequestHeader* rxHeader) 
{
    static char fruTxState = 0;
    static unsigned int fruNextHandle;

    unsigned long dataTransferHandle = *((long*)(mctp_context.rxBuffer + sizeof(*rxHeader)));
    unsigned char transferOperationFlag  = *(mctp_context.rxBuffer + sizeof(*rxHeader) + sizeof(unsigned long));
    unsigned char errorcode = 0;
    const unsigned short requestCount = 32;
    unsigned char padding = ((unsigned char)FRU_TOTAL_SIZE&0x03);
    if (padding) padding = 4-padding;

    switch (fruTxState) {
    case 0: // transfer has not begun yet
        if (transferOperationFlag != 0x1)
            errorcode = RESPONSE_INVALID_TRANSFER_OPERATION_FLAG;
        else if (dataTransferHandle != 0x0000)
            errorcode = RESPONSE_INVALID_DATA_TRANSFER_HANDLE;
        if (errorcode) {
            // send the error response
            mctp_transmitFrameStart(sizeof(PldmResponseHeader)+ 6 + 5-1, 1);
            transmitByte(rxHeader->flags1 & 0x7f);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            mctp_transmitFrameData(&errorcode,1);  // response->completionCode = errorcode;
            transmitLong(0);                       // response->NextTransferHandle;
            transmitByte(0);                       // response->transferFlag = 0;
            mctp_transmitFrameEnd();
            return;
        }
        if (requestCount >= FRU_TABLE_MAXIMUM_SIZE + padding) {          
            // send the data (single part)
            mctp_transmitFrameStart( sizeof(PldmResponseHeader) + 6 + FRU_TOTAL_SIZE + padding + 4 + 5-1, 1);
            transmitByte(rxHeader->flags1 & 0x7f);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            transmitByte(RESPONSE_SUCCESS);       // response->completionCode = RESPONSE_SUCCESS;
            transmitLong(0);                      // response->nextDataTransferHandle = 0;
            transmitByte(0x05);                   // response->transferFlag = 0x05;   // start and end
            // send the FRU data
            for (int i = 0;i < FRU_TOTAL_SIZE; i++) {
                transmitByte(pgm_read_byte(&(__fru_data[i])));
            }
            // send padding bytes if required
            for (int i = 0;i < padding; i++) {
                transmitByte(0x00);
            }
            transmitLong(0x00);                   // TODO: calculate and send CRC
            mctp_transmitFrameEnd();
            return;
        }
        // start sending the data (multi-part)
        mctp_transmitFrameStart( sizeof(PldmResponseHeader) + 6 + requestCount + 5-1,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
        transmitLong(dataTransferHandle + requestCount);  // next data transfer handle
        transmitByte(0x00);                               // response->transferFlag = 0x0;   // start
        // insert the pdr
        for (int i = 0;i < requestCount;i++) {
            unsigned char byte = pgm_read_byte(&(__fru_data[dataTransferHandle++])); 
            transmitByte(byte);
        }
        mctp_transmitFrameEnd();
        fruTxState = 1;
        fruNextHandle = dataTransferHandle;
        return;
    case 1: // transfer has already begun
        if (transferOperationFlag != 0x0) errorcode = RESPONSE_INVALID_TRANSFER_OPERATION_FLAG;
        else if (dataTransferHandle != fruNextHandle) errorcode = RESPONSE_INVALID_DATA_TRANSFER_HANDLE;
        if (errorcode) {
            // send the error response
            mctp_transmitFrameStart(sizeof(PldmResponseHeader)+ 6 + 5-1, 1);
            transmitByte(rxHeader->flags1 & 0x7f);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            mctp_transmitFrameData(&errorcode,1);  // response->completionCode = errorcode;
            transmitLong(0);                       // response->NextTransferHandle;
            transmitByte(0);                       // response->transferFlag = 0;
            mctp_transmitFrameEnd();
            return;
        }
        if (requestCount + dataTransferHandle >= FRU_TOTAL_SIZE + padding) {
            // transfer end part of the data
            mctp_transmitFrameStart( sizeof(PldmResponseHeader) + 10 + FRU_TOTAL_SIZE + padding -
                dataTransferHandle + 5 - 1 + 1,1);
            transmitByte(rxHeader->flags1 & 0x7f);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            transmitByte(RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
            transmitLong(0);                      // next data transfer handle
            transmitByte(0x04);                   // response->transferFlag = 0x04;   end
            // send the FRU data
            for (int i = 0;i < FRU_TOTAL_SIZE; i++) {
                transmitByte(pgm_read_byte(&(__fru_data[dataTransferHandle++])));
            }
            // send padding bytes if required
            for (int i = 0;i < padding; i++) {
                transmitByte(0x00);
            }
            transmitLong(0x00);                   // TODO: calculate and send CRC
            mctp_transmitFrameEnd();
            fruTxState = 0;
            return;
        }
        // send the middle data (multi-part)
        mctp_transmitFrameStart( sizeof(PldmResponseHeader) + 6 + requestCount + 5-1,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
        transmitLong(dataTransferHandle + requestCount); // next data transfer handle
        transmitByte(0x01);                   // response->transferFlag = 0x01;   middle
        for (int i = 0;i < requestCount;i++) {
            unsigned char byte = pgm_read_byte(&(__pdr_data[dataTransferHandle++])); 
            transmitByte(byte);
        }
        mctp_transmitFrameEnd();
        fruTxState = 1;
        fruNextHandle = dataTransferHandle;
        return;
    }
}

//*******************************************************************
// setStateEfffecterValue()
//
// set the value of a numeric state effecter if it exists.
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
static void setStateEffecterStates(PldmRequestHeader* rxHeader) {
    #ifdef ENTITY_STEPPER1
        unsigned char response = entityStepper1_setStateEffecterStates(rxHeader);
    #endif
    #ifdef ENTITY_SERVO1
        unsigned char response = entityServo1_setStateEffecterStates(rxHeader);
    #endif
    #ifdef ENTITY_PID1
        unsigned char response = entityPid1_setStateEffecterStates(rxHeader);
    #endif
    #ifdef ENTITY_SIMPLE1
        unsigned char response = entitySimple1_setStateEffecterStates(rxHeader);
    #endif
    
    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 1 + 5,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);   // completion code
        mctp_transmitFrameEnd();
} 

//*******************************************************************
// setStateEfffecterEnables()
//
// set the value of a state effecter enable if it exists.
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
static void setStateEffecterEnables(PldmRequestHeader* rxHeader) {
    #ifdef ENTITY_STEPPER1
        unsigned char response = entityStepper1_setStateEffecterEnables(rxHeader);
    #endif
    #ifdef ENTITY_SERVO1
        unsigned char response = entityServo1_setStateEffecterEnables(rxHeader);
    #endif
    #ifdef ENTITY_PID1
        unsigned char response = entityPid1_setStateEffecterEnables(rxHeader);
    #endif
    #ifdef ENTITY_SIMPLE1
        unsigned char response = entitySimple1_setStateEffecterEnables(rxHeader);
    #endif

    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 1 + 5,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);   // completion code
        mctp_transmitFrameEnd();
} 

//*******************************************************************
// getStateSensorReading()
//
// get the value of a state sensor state if it exists.
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
static void getStateSensorReading(PldmRequestHeader* rxHeader) {
    unsigned char body[10];
    unsigned char size;

    #ifdef ENTITY_STEPPER1
        unsigned char response = entityStepper1_getStateSensorReading(rxHeader, body, &size);
    #endif
    #ifdef ENTITY_SERVO1
        unsigned char response = entityServo1_getStateSensorReading(rxHeader, body, &size);
    #endif
    #ifdef ENTITY_PID1
        unsigned char response = entityPid1_getStateSensorReading(rxHeader, body, &size);
    #endif
    #ifdef ENTITY_SIMPLE1
        unsigned char response = entitySimple1_getStateSensorReading(rxHeader, body, &size);
    #endif

    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 1 + 5 + size,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);         // completion code
        mctp_transmitFrameData(body,size);
        mctp_transmitFrameEnd();
} 

//*******************************************************************
// getStateEfffecterStates()
//
// get the value of a numeric state effecter state if it exists.
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
static void getStateEffecterStates(PldmRequestHeader* rxHeader) {
    unsigned char body[10];
    unsigned char size;

    #ifdef ENTITY_STEPPER1
        unsigned char response = entityStepper1_getStateEffecterStates(rxHeader, body, &size);
    #endif
    #ifdef ENTITY_SERVO1
        unsigned char response = entityServo1_getStateEffecterStates(rxHeader, body, &size);
    #endif
    #ifdef ENTITY_PID1
        unsigned char response = entityPid1_getStateEffecterStates(rxHeader, body, &size);
    #endif
    #ifdef ENTITY_SIMPLE1
        unsigned char response = entitySimple1_getStateEffecterStates(rxHeader, body, &size);
    #endif

    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 1 + 5 + size,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);         // completion code
        mctp_transmitFrameData(body,size);
        mctp_transmitFrameEnd();
} 

//*******************************************************************
// setNumericEfffecterValue()
//
// set the value of a numeric state effecter if it exists.
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
static void setNumericEffecterValue(PldmRequestHeader* rxHeader) {
    #ifdef ENTITY_STEPPER1
        unsigned char response = entityStepper1_setNumericEffecterValue(rxHeader);
    #endif
    #ifdef ENTITY_SERVO1
        unsigned char response = entityServo1_setNumericEffecterValue(rxHeader);
    #endif
    #ifdef ENTITY_PID1
        unsigned char response = entityPid1_setNumericEffecterValue(rxHeader);
    #endif
    #ifdef ENTITY_SIMPLE1
        unsigned char response = entitySimple1_setNumericEffecterValue(rxHeader);
    #endif

    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 1 + 5,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);   // completion code
        mctp_transmitFrameEnd();    
}

//*******************************************************************
// getNumericEfffecterValue()
//
// set the value of a numeric state effecter if it exists.
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
static void getNumericEffecterValue(PldmRequestHeader* rxHeader) {
    unsigned char body[10];
    unsigned char size;

    #ifdef ENTITY_STEPPER1
        unsigned char response = entityStepper1_getNumericEffecterValue(rxHeader, body, &size);
    #endif
    #ifdef ENTITY_SERVO1
        unsigned char response = entityServo1_getNumericEffecterValue(rxHeader, body, &size);
    #endif
    #ifdef ENTITY_PID1
        unsigned char response = entityPid1_getNumericEffecterValue(rxHeader, body, &size);
    #endif
    #ifdef ENTITY_SIMPLE1
        unsigned char response = entitySimple1_getNumericEffecterValue(rxHeader, body, &size);
    #endif

    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 1 + size + 5,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);   // completion code
        mctp_transmitFrameData(body,size);
        mctp_transmitFrameEnd();
}

//*******************************************************************
// getSensorReading()
//
// return the value of a numeric sensor.
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
static void getSensorReading(PldmRequestHeader* rxHeader) {
    unsigned char body[10];
    unsigned char size;

    #ifdef ENTITY_STEPPER1
        unsigned char response = entityStepper1_getSensorReading(rxHeader, body, &size);
    #endif
    #ifdef ENTITY_SERVO1
        unsigned char response = entityServo1_getSensorReading(rxHeader, body, &size);
    #endif
    #ifdef ENTITY_PID1
        unsigned char response = entityPid1_getSensorReading(rxHeader, body, &size);
    #endif
    #ifdef ENTITY_SIMPLE1
        unsigned char response = entitySimple1_getSensorReading(rxHeader, body, &size);
    #endif

    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 1 + size + 5,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);   // completion code
        mctp_transmitFrameData(body,size);
        mctp_transmitFrameEnd();
}

//*******************************************************************
// setNumericSensorEnable()
//
// set the enable for a numeric effecter.
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
static void setNumericSensorEnable(PldmRequestHeader* rxHeader) {
    #ifdef ENTITY_STEPPER1
        unsigned char response = entityStepper1_setNumericSensorEnable(rxHeader);
    #endif
    #ifdef ENTITY_SERVO1
        unsigned char response = entityServo1_setNumericSensorEnable(rxHeader);
    #endif
    #ifdef ENTITY_PID1
        unsigned char response = entityPid1_setNumericSensorEnable(rxHeader);
    #endif
    #ifdef ENTITY_SIMPLE1
        unsigned char response = entitySimple1_setNumericSensorEnable(rxHeader);
    #endif

    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 1 + 5,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);   // completion code
        mctp_transmitFrameEnd();
}

//*******************************************************************
// setNumericEffecterEnable()
//
// set the enable for a numeric effecter.
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
static void setNumericEffecterEnable(PldmRequestHeader* rxHeader) {
    #ifdef ENTITY_STEPPER1
        unsigned char response = entityStepper1_setNumericEffecterEnable(rxHeader);
    #endif
    #ifdef ENTITY_SERVO1
        unsigned char response = entityServo1_setNumericEffecterEnable(rxHeader);
    #endif
    #ifdef ENTITY_PID1
        unsigned char response = entityPid1_setNumericEffecterEnable(rxHeader);
    #endif
    #ifdef ENTITY_SIMPLE1
        unsigned char response = entitySimple1_setNumericEffecterEnable(rxHeader);
    #endif

    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 1 + 5,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);   // completion code
        mctp_transmitFrameEnd();
}

//*******************************************************************
// setStateSensorEnables()
//
// set the enable for a state sensor.
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
static void setStateSensorEnables(PldmRequestHeader* rxHeader) {
    #ifdef ENTITY_STEPPER1
        unsigned char response = entityStepper1_setStateSensorEnables(rxHeader);
    #endif
    #ifdef ENTITY_SERVO1
        unsigned char response = entityServo1_setStateSensorEnables(rxHeader);
    #endif
    #ifdef ENTITY_PID1
        unsigned char response = entityPid1_setStateSensorEnables(rxHeader);
    #endif
    #ifdef ENTITY_SIMPLE1
        unsigned char response = entitySimple1_setStateSensorEnables(rxHeader);
    #endif

    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 1 + 5,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);   // completion code
        mctp_transmitFrameEnd();
}

//*******************************************************************
// setTID()
//
// return a value for the terminus id for this node
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
void setTid(PldmRequestHeader* rxHeader) {
    tid = *((uint8*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
    unsigned char response_code = RESPONSE_SUCCESS;
    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 1 + 5,1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response_code);   // completion code
        mctp_transmitFrameEnd();
}

//*******************************************************************
// getTID()
//
// get the value of the terminus id for this node
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
void getTid(PldmRequestHeader* rxHeader) {
    unsigned char response_code = RESPONSE_SUCCESS;
    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 1 + 1 + 5, 1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response_code);   // completion code
        transmitByte(tid);
        mctp_transmitFrameEnd();
}

//*******************************************************************
// getPldmVersion()
//
// respond by sending the Pldm Version supported by this node
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
void getPldmVersion(PldmRequestHeader* rxHeader) {
    unsigned char response_code = RESPONSE_SUCCESS;
    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 13 + 1 + 5, 1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response_code);   // completion code
        transmitLong(0x00000000);      // next transfer handle
        transmitByte(0x05);            // start and end
        transmitLong(0xF1F0F000);      // Version 1.0.0.0
        transmitLong(0x4A868FFB);      // CRC32 of the
        mctp_transmitFrameEnd();
}

//*******************************************************************
// getPldmTypes()
//
// respond by sending the Pldm types supported by this node
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
void getPldmTypes(PldmRequestHeader* rxHeader) {
    unsigned char response_code = RESPONSE_SUCCESS;
    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 8 + 1 + 5, 1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response_code);   // completion code
        transmitByte(0x15);            // types 0-7 (base, platform management, fru supported)
        transmitByte(0x00);            // types 8-15
        transmitByte(0x00);            // types 6-23
        transmitByte(0x00);            // types 24-31
        transmitByte(0x00);            // types 32-39
        transmitByte(0x00);            // types 40-47
        transmitByte(0x00);            // types 48-55
        transmitByte(0x00);            // types 56-63
        mctp_transmitFrameEnd();
}

//*******************************************************************
// getPldmCommands()
//
// respond by sending the commands supported by this node
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
void getPldmCommands(PldmRequestHeader* rxHeader) {
    unsigned char response_code = RESPONSE_SUCCESS;
    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 32 + 1 + 5, 1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response_code);  // completion code   
        switch(rxHeader->flags2&0x3F) {
            case 0:
                // pldm command and discovery
                transmitLong(0x0000003E);
                transmitLong(0x00000000);
                transmitLong(0x00000000);
                transmitLong(0x00000000);
                break;
            case 2:
                // pldm for platform management and control
                transmitLong(0x007F1810);
                transmitLong(0x07070003);
                transmitLong(0x00030000);
                transmitLong(0x00000000);
                break;
            case 4:
                // pldm for fru
                transmitLong(0x00000006);
                transmitLong(0x00000000);
                transmitLong(0x00000000);
                transmitLong(0x00000000);
                break;
            default:
                transmitLong(0x00000000);
                transmitLong(0x00000000);
                transmitLong(0x00000000);
                transmitLong(0x00000000);
                break;
        }
        mctp_transmitFrameEnd();
}

//*******************************************************************
// getTerminusUuid()
//
// respond by sending the UUID of this node
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
void getUuid(PldmRequestHeader* rxHeader) {
    unsigned char response_code = RESPONSE_SUCCESS;
    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 16 + 1 + 5, 1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response_code);   // completion code
        for (int i=0;i<16;i++) transmitByte(pgm_read_byte(uuid_bytes[i]));
        mctp_transmitFrameEnd();
}

//*******************************************************************
// processCommandEventMessageSupported()
//
// respond to a EventMessageSupported command.  Currently only polling
// is supported by the device.
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
void processCommandEventMessageSupported(PldmRequestHeader* rxHeader) {
    unsigned char response_code = RESPONSE_SUCCESS;
    
    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 4 + 1 + 5, 1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response_code);   // completion code
        transmitByte(globalEventEnableState);
        transmitByte(0x04);  // polled mode support only
        transmitByte(0x01);  // one class of event generated
        transmitByte(0x00);  // sensor event class generated
        mctp_transmitFrameEnd();
}

//*******************************************************************
// processSetEventReceiver()
//
// respond to a SetEventReceiver command.  Currently only polling
// is supported by the device.
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
void processSetEventReceiver(PldmRequestHeader* rxHeader) {
    unsigned char enable = *((char*)(rxHeader+1));

    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 1 + 5, 1);
    transmitByte(rxHeader->flags1 & 0x7f);
    transmitByte(rxHeader->flags2);
    transmitByte(rxHeader->command);
    if ((enable==0)||(enable==2)) transmitByte(RESPONSE_SUCCESS);
    else transmitByte(RESPONSE_ENABLE_METHOD_NOT_SUPPORTED);

    globalEventEnableState = 0;
    if (enable==2) globalEventEnableState = 1;
}

//*******************************************************************
// processPollForPlataformEvent()
//
// respond to a PollForPlatformEvent command.  C
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
void processPollForPlatformEvent(PldmRequestHeader* rxHeader) {
    if (!globalEventEnableState) {
        // send error if events are not enabled
        mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 1 + 5, 1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(RESPONSE_ERROR);   // completion code
        mctp_transmitFrameEnd();
        return;
    }
    
    // note: for simplicity, this function sends events as a
    // single transfer.  It is assumed that acknowledgements
    // always are targeted at the correct ID.
    unsigned char transferOperation = *(((char*)(rxHeader+1))+1);

    if (transferOperation==0x02) {
        // acknowledge only
        if (eventFifoInsertId!=eventFifoExtractId) {    
            // there are items in the fifo - call the entity to
            // acknowledge the current event
#ifdef ENTITY_SIMPLE1
            entitySimple1_acknowledgeEvent(eventFifoExtractId);
#endif
#ifdef ENTITY_STEPPER1
            entityStepper1_acknowledgeEvent(eventFifoExtractId);
#endif
            // "remove the event from the fifo"
            eventFifoExtractId = (eventFifoExtractId+1)&0xF;
        } 
        // send the response
        mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 3 + 1 + 5, 1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(RESPONSE_SUCCESS);   // completion code
        transmitByte(tid);
        if (eventFifoInsertId!=eventFifoExtractId) transmitShort(0xFFFF); 
        else transmitShort(0x0000); 
        mctp_transmitFrameEnd();
    } else {
        if (eventFifoInsertId!=eventFifoExtractId) {
            // there are items in the FIFO - call the entity to
            // respond to this request
#ifdef ENTITY_SIMPLE1
            entitySimple1_respondToPollEvent(rxHeader, eventFifoInsertId, eventFifoExtractId);
#endif
#ifdef ENTITY_STEPPER1
            entityStepper1_respondToPollEvent(rxHeader, eventFifoInsertId, eventFifoExtractId);
#endif
        } else {
            // send the response - there was nothing to retrieve
            mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 3 + 1 + 5, 1);
            transmitByte(rxHeader->flags1 & 0x7f);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            transmitByte(RESPONSE_SUCCESS);   // completion code
            transmitByte(tid);
            transmitShort(0x00); 
            mctp_transmitFrameEnd();
        }
    }
}

//*******************************************************************
// getFruTableMetadata()
//
// respond to a getFruTableMetadata command.  
//
// parameters:
//    rxHeader - a pointer to the request header
// returns:
//    void
// changes:
//    the contents of the transmit buffer
void getFruTableMetadata(PldmRequestHeader* rxHeader) {
    unsigned char response_code = RESPONSE_SUCCESS;
    
    // send the response
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 18 + 1 + 5, 1);
        transmitByte(rxHeader->flags1 & 0x7f);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response_code);   // completion code
        transmitByte(0x01);  // major version
        transmitByte(0x00);  // minor version
        transmitLong(FRU_TABLE_MAXIMUM_SIZE); 
        transmitLong(FRU_TOTAL_SIZE); 
        transmitShort(FRU_TOTAL_RECORD_SETS);
        transmitShort(FRU_NUMBER_OF_RECORDS);
        transmitLong(0x00000000);  // CRC32 - TODO: Calculate this checksum       
        mctp_transmitFrameEnd();
}

//*******************************************************************
// parseCommand()
//
// parse a new PLDM command and take appropriate action.  It is assumed
// that the new PLDM request is stored in the rx buffer.
//
// parameters:
//    none
// returns:
//    none
// changes:
//    the contents of the tx buffer may be altered.
static void parseCommand()
{
    // cast the relevant portions of the header so that
    // they are easier to use later.
    PldmRequestHeader* rxHeader = (PldmRequestHeader*)mctp_getPacket();

    // switch based on the command and type in the header
    if (((rxHeader->flags2)&0x3f)==0) {
        // PLDM Messanging Control and Discovery
        switch (rxHeader->command) {
        case CMD_GET_TID:
            getTid(rxHeader);
            break;
        case CMD_SET_TID:
            setTid(rxHeader);
            break;
        case CMD_GET_PLDM_VERSION:
            getPldmVersion(rxHeader);
            break;
        case CMD_GET_PLDM_TYPES:
            getPldmTypes(rxHeader);
            break;
        case CMD_GET_PLDM_COMMANDS:
            getPldmCommands(rxHeader);
            break;
        default:
            mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 5 + 1,1);
            transmitByte(rxHeader->flags1 & 0x7f);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            transmitByte(RESPONSE_ERROR_UNSUPPORTED_PLDM_CMD);   // completion code
            break;
        }
    } else if (((rxHeader->flags2)&0x3f)==2) {
        // PLDM for Platform Monitoring and Control
        switch (rxHeader->command) {
        case CMD_GET_TERMINUS_UID:
            getUuid(rxHeader);
            break;
        case CMD_GET_SENSOR_READING:
            getSensorReading(rxHeader);
            break;
        case CMD_SET_NUMERIC_SENSOR_ENABLE:
            setNumericSensorEnable(rxHeader);
            break;
        case CMD_GET_STATE_SENSOR_READINGS:
            getStateSensorReading(rxHeader);
            break;
        case CMD_SET_STATE_SENSOR_ENABLES:
            setStateSensorEnables(rxHeader);
            break;
        case CMD_SET_NUMERIC_EFFECTER_VALUE:
            setNumericEffecterValue(rxHeader);
            break;
        case CMD_GET_NUMERIC_EFFECTER_VALUE:
            getNumericEffecterValue(rxHeader);
            break;
        case CMD_SET_STATE_EFFECTER_STATES:
            setStateEffecterStates(rxHeader);
            break;
        case CMD_GET_STATE_EFFECTER_STATES:
            getStateEffecterStates(rxHeader);
            break;
        case CMD_GET_PDR_REPOSITORY_INFO:
        {
            mctp_transmitFrameStart(sizeof(GetPdrRepositoryInfoResponse) + sizeof(PldmRequestHeader) + 5,1);
            transmitByte(rxHeader->flags1 & 0x7f);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            transmitByte(RESPONSE_SUCCESS);   // completion code
            transmitByte(0);                  // repository state = available
            for (int i=0;i<13;i++) transmitByte(0); // update time
            for (int i=0;i<13;i++) transmitByte(0); // oem update time
            transmitLong(PDR_NUMBER_OF_RECORDS);            // pdr record count
            transmitLong(PDR_TOTAL_SIZE);   // repository size
            transmitLong(PDR_MAX_RECORD_SIZE);  // record size
            transmitByte(0);                  // no timeout
            mctp_transmitFrameEnd();
            break;
        }
        case CMD_GET_PDR:
            processCommandGetPdr(rxHeader);
            break;
        case CMD_SET_NUMERIC_EFFECTER_ENABLE:
            setNumericEffecterEnable(rxHeader);
            break;
        case CMD_SET_STATE_EFFECTER_ENABLES:
            setStateEffecterEnables(rxHeader);
            break;
        case CMD_EVENT_MESSAGE_SUPPORTED:
            processCommandEventMessageSupported(rxHeader);
            break;
        case CMD_POLL_FOR_PLATFORM_EVENT_MESSAGE:
            processPollForPlatformEvent(rxHeader);
            break;
        case CMD_SET_EVENT_RECEIVER:
            processSetEventReceiver(rxHeader);
            break;
        default:
            mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 5 + 1,1);
            transmitByte(rxHeader->flags1 & 0x7f);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            transmitByte(RESPONSE_ERROR_UNSUPPORTED_PLDM_CMD);   // completion code
            break;
        }
    } else if (((rxHeader->flags2)&0x3f)==4) {
        // PLDM for FRU Data
        switch (rxHeader->command) {
        case CMD_GET_FRU_TABLE_METADATA:
            getFruTableMetadata(rxHeader);
            break;
        case CMD_GET_FRU_RECORD_TABLE:
            processCommandFruTable(rxHeader); 
            break;
        default:
            mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 5 + 1,1);
            transmitByte(rxHeader->flags1 & 0x7f);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            transmitByte(RESPONSE_ERROR_UNSUPPORTED_PLDM_CMD);   // completion code
            break;
        }
    } 
    return;
}

//*******************************************************************
// putCommand()
//
// a public entry point to send a command to the connector node.
//
// parameters:
//    hdr - a pointer to the request header
//    command - a pointer to the body of the command
//    size - the number of bytes in the command body
// returns:
//    void
void node_putCommand(PldmRequestHeader* hdr, unsigned char* command, unsigned int size) {
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + size +5,1);
    mctp_transmitFrameData((unsigned char *)hdr,sizeof(PldmRequestHeader));
    mctp_transmitFrameData(command,size);
    mctp_transmitFrameEnd();
}

//*******************************************************************
// node_getResponse()
//
// Update the PLDM Rx finite state machine and return a pointer to 
// the most recent message response if there is one.  If there
// is no packet available, it returns 0.  If there is an MCTP 
// control message, the message is consumed.
//
// parameters: none
// returns: a pointer to the most recent message response.
//    void
unsigned char* node_getResponse(void) {
    if (!mctp_isPacketAvailable()) {
        mctp_updateRxFSM();
        return 0;
    } else {
        parseCommand();               // process the command
        return mctp_getPacket();  // clear the packet ready flag
    }
}

//===================================================================
// sendNumericSensorEvent()
//
// send a numeric sensor event based on the parameters passed to the
// function.  This helper function is intended to be called by child
// instances from within the high-priority loop.
//
// parameters:
//    egi - event generator instance related to this event.
//    sensorId - the ID of the senosr that caused the event
//    presentReading - the current reading of the sensor  
void node_sendNumericSensorEvent(
        PldmRequestHeader *rxHeader,
        unsigned char more,
        EventGeneratorInstance* egi, 
        unsigned int sensorId, 
        unsigned char previousEventState, 
        FIXEDPOINT_24_8 presentReading
) 
{  
    // begin the event frame
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 13 + 5,1);
    // transmit the header
    transmitByte( 0x80 );    // pldm datagram request message type, instance ID 0
    transmitByte( 0x00);     // header version = 00, pldm type = 0 (pldm messaging/discovery)
    transmitByte( CMD_PLATFORM_EVENT_MESSAGE ); 

    // transmit the platform event message common data
    transmitByte(0x01);         // format version
    transmitByte(0x01);         // terminus ID
    transmitByte(0x00);         // event class 0 = sensor

    // transmit the body
    transmitShort(sensorId);
    transmitByte(2);            // cause = numeric sensor state change
    transmitByte(egi->eventState);
    transmitByte(previousEventState);
    transmitByte(5);            // reading is a signed 32-bit integer
    transmitLong(presentReading);
    mctp_transmitFrameEnd();
}

//===================================================================
// sendStateSensorEvent()
//
// send a state sensor event based on the parameters passed to the
// function.  This helper function is intended to be called by child
// instances from within the high-priority loop.
//
// parameters:
//    egi - event generator instance related to this event.
//    sensorId - the ID of the senosr that caused the event
void node_sendStateSensorEvent(
        PldmRequestHeader *rxHeader,
        unsigned char more,
        EventGeneratorInstance* egi, 
        unsigned int sensorId, 
        unsigned char previousEventState) {

    // begin the event frame
    mctp_transmitFrameStart(sizeof(PldmRequestHeader) + 8 + 5,1);
    // transmit the header
    transmitByte( 0x80 );    // pldm datagram request message type, instance ID 0
    transmitByte( 0x00);     // header version = 00, pldm type = 0 (pldm messaging/discovery)
    transmitByte( CMD_PLATFORM_EVENT_MESSAGE ); 

    // transmit the platform event message common data
    transmitByte(0x01);         // format version
    transmitByte(0x01);         // terminus ID
    transmitByte(0x00);         // event class 0 = sensor

    // transmit the body
    transmitShort(sensorId);
    transmitByte(1);            // cause = state sensor state change
    transmitByte(egi->eventState);
    transmitByte(previousEventState);
    
    mctp_transmitFrameEnd();
}

//===================================================================
// updateEvents()
//
// called from the low-priority loop to update the state of the event
// handler.
//
// parameters:
void node_updateEvents() {
    #ifdef ENTITY_STEPPER1
    entityStepper1_updateEvents(&eventFifoInsertId);
    #endif
    #ifdef ENTITY_SIMPLE1
    entitySimple1_updateEvents(&eventFifoInsertId);
    #endif
}