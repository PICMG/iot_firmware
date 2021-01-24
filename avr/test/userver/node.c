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
#include "node.h"
#include "mctp.h"
#include "pldm.h"
#include "pdrdata.h"
#include "control_servo.h"

static	mctp_struct *mctp;
uint8   tid;

#define UINT8_TYPE  0
#define SINT8_TYPE  1
#define UINT16_TYPE 2
#define SINT16_TYPE 3
#define UINT32_TYPE 4
#define SINT32_TYPE 5

#define SERVO_CONTROL_MODE    1

#define KFFA_EFFECTER_ID      1
#define APROFILE_EFFECTER_ID  2
#define VPROFILE_EFFECTER_ID  3
#define PFINAL_EFFECTER_ID    4
#define INTERLOCK_EFFECTER_ID 5
#define TRIGGER_EFFECTER_ID   6
#define START_EFFECTER_ID     7

#define TRIGGER_SENSOR_ID     7
#define INTERLOCK_SENSOR_ID   6
#define MOTOR_STATE_SENSOR_ID 5
#define POSITION_SENSOR_ID    3
#define PERR_SENSOR_ID        1
#define VELOCITY_SENSOR_ID    4
#define VERR_SENSOR_ID        2
#define POS_LIMIT_SENSOR_ID   8
#define NEG_LIMIT_SENSOR_ID   9


#ifdef SERVO_CONTROL_MODE
    static  unsigned char requested_start_state = 2;  // initially stopped
#endif


static void transmitByte(unsigned char data) {
    // send the data to the MCTP buffer in little-endian fashion
    mctp_transmitFrameData(mctp,&data,1);
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


void node_init(mctp_struct *mctp_ptr) {
//    pdrCount = __pdr_number_of_records;
    mctp=mctp_ptr;
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

    GetPdrCommand* request = (GetPdrCommand*)(mctp->rxBuffer + sizeof(*rxHeader));
    unsigned char errorcode = 0;
    switch (pdrTxState) {
    case 0:
        // transfer has not begun yet
        if (request->transferOperationFlag != 0x1)
            errorcode = RESPONSE_INVALID_TRANSFER_OPERATION_FLAG;
        else if (request->recordHandle > __pdr_number_of_records)
            errorcode = RESPONSE_INVALID_RECORD_HANDLE;
        else if (request->dataTransferHandle != 0x0000)
            errorcode = RESPONSE_INVALID_DATA_TRANSFER_HANDLE;
        else if (request->recordChangeNumber != 0x0000)
            errorcode = RESPONSE_INVALID_RECORD_CHANGE_NUMBER;
        if (errorcode) {
            // send the error response
            mctp_transmitFrameStart(mctp,sizeof(PldmResponseHeader)+sizeof(GetPdrResponse) + 4-1);
            transmitByte(rxHeader->flags1 | 0x80);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            mctp_transmitFrameData(mctp,&errorcode,1);  // response->completionCode = errorcode;
            transmitLong(0);                       // response->nextRecordHandle = 0;
            transmitLong(0);                       // response->NextRecordTransferHandle;
            transmitByte(0);                       // response->transferFlag = 0;
            transmitShort(0);                      // response->responseCount = 0;
            mctp_transmitFrameEnd(mctp);
            return;
        }
        if (request->requestCount >= pdrSize(request->recordHandle)) {
            // send the data (single part)
            mctp_transmitFrameStart(mctp, sizeof(PldmResponseHeader) + sizeof(GetPdrResponse) + pdrSize(request->recordHandle)+4-1);
            transmitByte(rxHeader->flags1 | 0x80);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            transmitByte(RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
            transmitLong((getNextRecord(request->recordHandle) <= __pdr_number_of_records) ?
                getNextRecord(request->recordHandle) : 0);
            transmitLong(0);                      //response->nextDataTransferHandle = 0;
            transmitByte(0x05);                   // response->transferFlag = 0x05;   // start and end
            transmitShort(pdrSize(request->recordHandle)); // response->responseCount = pdrSize(request->recordHandle);
            unsigned int extractionPoint = getPdrOffset(request->recordHandle);
            // insert the pdr
            for (int i = 0;i < pdrSize(request->recordHandle); i++) {
                transmitByte(pgm_read_byte(&(__pdr_data[extractionPoint++])));
            }
            mctp_transmitFrameEnd(mctp);
            return;
        }
        // start sending the data (multi-part)
        mctp_transmitFrameStart(mctp, sizeof(PldmResponseHeader) + sizeof(GetPdrResponse) + request->requestCount+4-1);
        transmitByte(rxHeader->flags1 | 0x80);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
        transmitLong((getNextRecord(request->recordHandle) <= __pdr_number_of_records) ?
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
        mctp_transmitFrameEnd(mctp);
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
            mctp_transmitFrameStart(mctp,sizeof(PldmResponseHeader)+sizeof(GetPdrResponse) + 4-1);
            transmitByte(rxHeader->flags1 | 0x80);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            mctp_transmitFrameData(mctp,&errorcode,1);  // response->completionCode = errorcode;
            transmitLong(0);                       // response->nextRecordHandle = 0;
            transmitLong(0);                       // response->NextRecordTransferHandle;
            transmitByte(0);                       // response->transferFlag = 0;
            transmitShort(0);                      // response->responseCount = 0;
            mctp_transmitFrameEnd(mctp);
            return;
        }
        if (request->requestCount + request->dataTransferHandle >= getPdrOffset(request->recordHandle)+pdrSize(request->recordHandle)+1) {
            // transfer end part of the data
            mctp_transmitFrameStart(mctp, sizeof(PldmResponseHeader) + sizeof(GetPdrResponse) + pdrSize(request->recordHandle) -
                (request->dataTransferHandle-getPdrOffset(request->recordHandle)) + 4 - 1 + 1);
            transmitByte(rxHeader->flags1 | 0x80);
            transmitByte(rxHeader->flags2);
            transmitByte(rxHeader->command);
            transmitByte(RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
            transmitLong((getNextRecord(request->recordHandle) <= __pdr_number_of_records) ?
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
            mctp_transmitFrameEnd(mctp);
            pdrTxState = 0;
            return;
        }
        // send the middle data (multi-part)
        mctp_transmitFrameStart(mctp, sizeof(PldmResponseHeader) + sizeof(GetPdrResponse) + request->requestCount + 4-1);
        transmitByte(rxHeader->flags1 | 0x80);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
        transmitLong((getNextRecord(request->recordHandle) <= __pdr_number_of_records) ?
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
        mctp_transmitFrameEnd(mctp);
        pdrTxState = 1;
        pdrNextHandle = request->dataTransferHandle + request->requestCount;
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
    // extract the information from the body
    unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
    unsigned char effecter_count = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2);
    unsigned char response = RESPONSE_SUCCESS; 
    if (effecter_count != 1) {
        response = RESPONSE_INVALID_STATE_VALUE; 
    } 
    else {
        unsigned char action = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2 + 1);
        unsigned char req_state = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2 + 1 + 1);
        switch (effecter_id) {
    #ifdef SERVO_CONTROL_MODE
        case START_EFFECTER_ID:
            if (action) {
                if ((req_state==1)||(req_state==2)) {
                    // 1 = stop, 2 = run
                    if (!control_setState(req_state)) {
                        response = RESPONSE_INVALID_STATE_VALUE;
                    } else {
                        requested_start_state = req_state;
                    }
                } else {
                    response = RESPONSE_UNSUPPORTED_EFFECTERSTATE;
                }
            }
            break;
    #endif
        case TRIGGER_EFFECTER_ID:
            if (action) {
                if ((req_state==1)||(req_state==2)) {
                    // TODO - set the TRIGGER output
                } else {
                    response = RESPONSE_UNSUPPORTED_EFFECTERSTATE;
                }
            }
            break;
        case INTERLOCK_EFFECTER_ID:
            if (action) {
                if ((req_state==1)||(req_state==2)) {
                    // TODO - set the INTERLOCK output
                } else {
                    response = RESPONSE_UNSUPPORTED_EFFECTERSTATE;
                }
            }
            break;
        default:
            response = RESPONSE_INVALID_EFFECTER_ID; 
            break;
        }
    }

    // send the response
    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + 1 + 4);
        transmitByte(rxHeader->flags1 | 0x80);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);   // completion code
        mctp_transmitFrameEnd(mctp);
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
    // extract the information from the body
    unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
    unsigned char effecter_count = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2);
    unsigned char effecter_op_state     = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+3);
    unsigned char effecter_event_enable = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+4);
    unsigned char response = RESPONSE_SUCCESS; 
    if (effecter_count != 1) {
        response = RESPONSE_INVALID_STATE_VALUE; 
    } 
    else {
        switch (effecter_id) {
    #ifdef SERVO_CONTROL_MODE
        case START_EFFECTER_ID:
            // TODO: do something with the enable setting
            break;
    #endif
        case TRIGGER_EFFECTER_ID:
            // TODO: do something with the enable setting
            break;
        case INTERLOCK_EFFECTER_ID:
            // TODO: do something with the enable setting
            break;
        default:
            response = RESPONSE_INVALID_EFFECTER_ID; 
            // TODO: do something with the enable setting
            break;
        }
    }

    // send the response
    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + 1 + 4);
        transmitByte(rxHeader->flags1 | 0x80);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);   // completion code
        mctp_transmitFrameEnd(mctp);
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
    // extract the information from the body
    unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
    unsigned char response = RESPONSE_SUCCESS; 
    unsigned char body[5] = {1,1,0,0,0};  // 1 sensor, enabled, no update pending, 
    unsigned char hasbody = 1;
    switch (effecter_id) {
#ifdef SERVO_CONTROL_MODE
    case MOTOR_STATE_SENSOR_ID:
        body[2] = control_getState();
        body[3] = control_getState();
        break;
    case POS_LIMIT_SENSOR_ID:
        // TODO, get the value of the position limit switch
        break;
    case NEG_LIMIT_SENSOR_ID:
        // TODO, get the value of the position limit switch
        break;
#endif
    case TRIGGER_SENSOR_ID:
        // TODO, get the trigger state from the hardware
        break;
    case INTERLOCK_SENSOR_ID:
        // TODO, get the interlock state from the hardware
        break;
    default:
        response = RESPONSE_INVALID_EFFECTER_ID; 
        hasbody = 0;
        break;
    }

    // send the response
    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + 1 + 4 + ((hasbody)?5:1));
        transmitByte(rxHeader->flags1 | 0x80);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);         // completion code
        if (hasbody) mctp_transmitFrameData(mctp,body,5);
        mctp_transmitFrameEnd(mctp);
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
    // extract the information from the body
    unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
    unsigned char response = RESPONSE_SUCCESS; 
    unsigned char body[4] = {1,1,0,0};  // 1 sensor, enabled, no update pending, 
    switch (effecter_id) {
#ifdef SERVO_CONTROL_MODE
    case START_EFFECTER_ID:
        body[2] = requested_start_state;
        body[3] = requested_start_state;
        break;
#endif
    case TRIGGER_EFFECTER_ID:
        // TODO, get the trigger state from the hardware
        break;
    case INTERLOCK_EFFECTER_ID:
        // TODO, get the interlock state from the hardware
        break;
    default:
        response = RESPONSE_INVALID_EFFECTER_ID; 
        break;
    }

    // send the response
    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + 1 + 4 + 4);
        transmitByte(rxHeader->flags1 | 0x80);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);         // completion code
        mctp_transmitFrameData(mctp,body,4);
        mctp_transmitFrameEnd(mctp);
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
    // extract the information from the body
    unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
    unsigned char effecter_numtype = *(((char*)rxHeader)+sizeof(PldmRequestHeader)+2);
    unsigned char response = RESPONSE_SUCCESS; 
    switch (effecter_id) {
#ifdef SERVO_CONTROL_MODE
    case APROFILE_EFFECTER_ID:
        if (effecter_numtype == SINT32_TYPE) {
            requested_acceleration = *((long*)(((char*)rxHeader)+sizeof(PldmRequestHeader)+2+1));
        } else {
            response = RESPONSE_ERROR_INVALID_DATA;
        }
        break;
    case VPROFILE_EFFECTER_ID:
        if (effecter_numtype == SINT32_TYPE) {
            requested_velocity = *((long*)(((char*)rxHeader)+sizeof(PldmRequestHeader)+2+1));            
        } else {
            response = RESPONSE_ERROR_INVALID_DATA;
        }
        break;
    case PFINAL_EFFECTER_ID:
        if (effecter_numtype == SINT32_TYPE) {
            requested_position = *((long*)(((char*)rxHeader)+sizeof(PldmRequestHeader)+2+1));                        
        } else {
            response = RESPONSE_ERROR_INVALID_DATA;
        }
        break;
    case KFFA_EFFECTER_ID:
        if (effecter_numtype == SINT32_TYPE) {
            requested_kffa = *((long*)(((char*)rxHeader)+sizeof(PldmRequestHeader)+2+1));                                    
        } else {
            response = RESPONSE_ERROR_INVALID_DATA;
        }
        break;
#endif
    default:
        response = RESPONSE_INVALID_EFFECTER_ID; 
        break;
    }

    // send the response
    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + 1 + 4);
        transmitByte(rxHeader->flags1 | 0x80);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response);   // completion code
        mctp_transmitFrameEnd(mctp);    
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
    // extract the information from the body
    unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
    long          return_data = 0;
    switch (effecter_id) {
#ifdef SERVO_CONTROL_MODE
    case APROFILE_EFFECTER_ID:
        return_data = requested_acceleration;
        break;
    case VPROFILE_EFFECTER_ID:
        return_data = requested_velocity;
        break;
    case PFINAL_EFFECTER_ID:
        return_data = requested_position;
        break;
    case KFFA_EFFECTER_ID:
        return_data = requested_kffa;
        break;
#endif
    default:
        mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + 1 + 4);
        transmitByte(rxHeader->flags1 | 0x80);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(RESPONSE_INVALID_EFFECTER_ID);   // completion code
        mctp_transmitFrameEnd(mctp);
        break;
    }

    // send the response
    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + 1 + 10 + 4);
        transmitByte(rxHeader->flags1 | 0x80);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(RESPONSE_SUCCESS);   // completion code
        transmitByte(5); // data size = sint32
        transmitByte(0); // enabled, pending
        transmitLong(return_data);
        transmitLong(return_data);
        mctp_transmitFrameEnd(mctp);
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
    // extract the information from the body
    unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
    unsigned char body[6] = {5,1,0,0,0,0};
    sint32   reading = 0;
    unsigned char response_code = RESPONSE_SUCCESS;
    switch (effecter_id) {
#ifdef SERVO_CONTROL_MODE
    case POSITION_SENSOR_ID:
        // TODO: return actual sensor reading
        reading = 10000;
        break;
    case PERR_SENSOR_ID:
        // TODO: return actual sensor reading
        reading = 0x00000001;
        break;
    case VELOCITY_SENSOR_ID:
        // TODO: return actual sensor reading
        reading = 0x00000100;
        break;
    case VERR_SENSOR_ID:
        // TODO: return actual sensor reading
        reading = 0x00001000;
        break;
#endif
    default:
        response_code = RESPONSE_INVALID_SENSOR_ID;   // completion code
        break;
    }

    // send the response
    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + 1 + 10 + 4);
        transmitByte(rxHeader->flags1 | 0x80);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response_code);   // completion code
        mctp_transmitFrameData(mctp,body,6);
        transmitLong(reading);
        mctp_transmitFrameEnd(mctp);
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
static void setNumericEffecterEnable(PldmRequestHeader* rxHeader) {
    // extract the information from the body
    unsigned int  effecter_id  = *((int*)(((char*)rxHeader)+sizeof(PldmRequestHeader)));
    unsigned int enable_state = *((char*)(((char*)rxHeader)+sizeof(PldmRequestHeader)+2));
    unsigned char response_code = RESPONSE_SUCCESS;
    switch (effecter_id) {
#ifdef SERVO_CONTROL_MODE
    case APROFILE_EFFECTER_ID:
        // TODO: use the emable to do something
        break;
    case VPROFILE_EFFECTER_ID:
        // TODO: use the emable to do something
        break;
    case PFINAL_EFFECTER_ID:
        // TODO: use the emable to do something
        break;
    case KFFA_EFFECTER_ID:
        // TODO: use the emable to do something
        break;
#endif
    default:
        response_code = RESPONSE_INVALID_EFFECTER_ID;   // completion code
        break;
    }

    // send the response
    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + 1 + 4);
        transmitByte(rxHeader->flags1 | 0x80);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response_code);   // completion code
        mctp_transmitFrameEnd(mctp);
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
    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + 1 + 4);
        transmitByte(rxHeader->flags1 | 0x80);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response_code);   // completion code
        mctp_transmitFrameEnd(mctp);
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
    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + 1 + 1 + 4);
        transmitByte(rxHeader->flags1 | 0x80);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(response_code);   // completion code
        transmitByte(tid);
        mctp_transmitFrameEnd(mctp);
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
    PldmRequestHeader* rxHeader = (PldmRequestHeader*)mctp_getPacket(mctp);

    // switch based on the command type byte in the header
    switch (rxHeader->command) {
    case CMD_GET_TID:
        getTid(rxHeader);
        break;
    case CMD_SET_TID:
        setTid(rxHeader);
        break;
    case CMD_GET_SENSOR_READING:
        getSensorReading(rxHeader);
        break;
    case CMD_GET_STATE_SENSOR_READINGS:
        getStateSensorReading(rxHeader);
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
        mctp_transmitFrameStart(mctp,sizeof(GetPdrRepositoryInfoResponse) + sizeof(PldmRequestHeader) + 4);
        transmitByte(rxHeader->flags1 | 0x80);
        transmitByte(rxHeader->flags2);
        transmitByte(rxHeader->command);
        transmitByte(RESPONSE_SUCCESS);   // completion code
        transmitByte(0);                  // repository state = available
        for (int i=0;i<13;i++) transmitByte(0); // update time
        for (int i=0;i<13;i++) transmitByte(0); // oem update time
        transmitLong(__pdr_number_of_records);            // pdr record count
        transmitLong(__pdr_total_size);   // repository size
        transmitLong(__pdr_max_record_size);  // record size
        transmitByte(0);                  // no timeout
        mctp_transmitFrameEnd(mctp);
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
    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + size +4);
    mctp_transmitFrameData(mctp,(unsigned char *)hdr,sizeof(PldmRequestHeader));
    mctp_transmitFrameData(mctp,command,size);
    mctp_transmitFrameEnd(mctp);
}

unsigned char* node_getResponse(void) {
    if (!mctp_isPacketAvailable(mctp)) {
        mctp_updateRxFSM(mctp);
        return 0;
    } else {
        parseCommand();               // process the command
        return mctp_getPacket(mctp);  // clear the packet ready flag
    }
}