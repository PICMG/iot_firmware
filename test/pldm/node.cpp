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
#include "node.h"
#include "pdrdata.h"

//*******************************************************************
// node()
//
// default constructor
node::node() {
    pdrCount = __pdr_number_of_records;
    nextByte = 0;
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
        unsigned long part2 = result->dataLength;
        offset = offset + part1 + part2;
        result = (PdrCommonHeader*)(&__pdr_data[offset]);
        if (__pdr_data[offset] == 0) return 0;
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
        unsigned long part2 = hdr->dataLength;
        offset = offset + part1 + part2;
        hdr = (PdrCommonHeader*)(&__pdr_data[offset]);
        if (__pdr_data[offset] == 0) return 0;
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
unsigned int node::pdrSize(unsigned int index) {
    PdrCommonHeader * header = getPdrHeader(index);
    if (!header) return 0;
    return header->dataLength + sizeof(PdrCommonHeader);
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
void node::processCommandGetPdr(PldmRequestHeader* rxHeader, PldmResponseHeader* txHeader) 
{
    static char pdrTxState = 0;
    static unsigned int pdrNextHandle;
    static unsigned long pdrRecord;
    unsigned char* insertionPoint;
    unsigned int extractionPoint;

    GetPdrCommand* request = (GetPdrCommand*)(rxBuffer + sizeof(*rxHeader));
    GetPdrResponse* response = (GetPdrResponse*)(txBuffer + sizeof(*txHeader));
    unsigned char errorcode = 0;
    unsigned char* hpr;
    switch (pdrTxState) {
    case 0:
        // transfer has not begun yet
        if (request->transferOperationFlag != 0x1)
            errorcode = RESPONSE_INVALID_TRANSFER_OPERATION_FLAG;
        else if (request->recordHandle > pdrCount)
            errorcode = RESPONSE_INVALID_RECORD_HANDLE;
        else if (request->dataTransferHandle != 0x0000)
            errorcode = RESPONSE_INVALID_DATA_TRANSFER_HANDLE;
        else if (request->recordChangeNumber != 0x0000)
            errorcode = RESPONSE_INVALID_RECORD_CHANGE_NUMBER;
        if (errorcode) {
            // send the error response
            response->completionCode = errorcode;
            response->nextRecordHandle = 0;
            response->nextDataTransferHandle = 0;
            response->transferFlag = 0;
            response->responseCount = 0;
            return;
        }
        if (request->requestCount >= pdrSize(request->recordHandle)) {
            // send the data (single part)
            response->completionCode = RESPONSE_SUCCESS;
            response->nextRecordHandle = (getNextRecord(request->recordHandle) <= pdrCount) ?
                getNextRecord(request->recordHandle) : 0;
            response->nextDataTransferHandle = 0;
            response->transferFlag = 0x05;   // start and end
            response->responseCount = pdrSize(request->recordHandle);
            unsigned char* insertionPoint =
                txBuffer + sizeof(PldmResponseHeader) +
                sizeof(GetPdrResponse);
            unsigned int extractionPoint = getPdrOffset(request->recordHandle);
            // insert the pdr
            for (int i = 0;i < response->responseCount; i++) {
                *insertionPoint++ = __pdr_data[extractionPoint++];
            }
            return;
        }
        // send the data (multi-part)
        response->completionCode = RESPONSE_SUCCESS;
        response->nextRecordHandle = (getNextRecord(request->recordHandle) <= pdrCount) ?
            getNextRecord(request->recordHandle) : 0;
        response->nextDataTransferHandle = getPdrOffset(request->recordHandle) +
            request->requestCount;
        response->transferFlag = 0x0;            // start
        response->responseCount = request->requestCount;
        insertionPoint = txBuffer + sizeof(PldmResponseHeader) + sizeof(GetPdrResponse);
        extractionPoint = request->dataTransferHandle+getPdrOffset(request->recordHandle);
        // insert the pdr
        for (int i = 0;i < request->requestCount;i++) {
            *insertionPoint++ = __pdr_data[extractionPoint++];
        }
        pdrTxState = 1;
        pdrRecord = request->recordHandle;
        pdrNextHandle = response->nextDataTransferHandle;
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
            response->completionCode = errorcode;
            response->nextRecordHandle = 0;
            response->nextDataTransferHandle = 0;
            response->transferFlag = 0;
            response->responseCount = 0;
            return;
        }
        if (request->requestCount + request->dataTransferHandle >= getPdrOffset(request->recordHandle)+pdrSize(request->recordHandle)) {
            // send the last part of the data
            response->completionCode = RESPONSE_SUCCESS;
            response->nextRecordHandle = (getNextRecord(request->recordHandle) <= pdrCount) ?
                getNextRecord(request->recordHandle) : 0;
            response->nextDataTransferHandle = 0;
            response->transferFlag = 0x04;   // end
            response->responseCount =
                 pdrSize(request->recordHandle) -
                (request->dataTransferHandle-getPdrOffset(request->recordHandle));
            unsigned char* insertionPoint =
                txBuffer + sizeof(PldmResponseHeader) +
                sizeof(GetPdrResponse);
            unsigned int extractionPoint = request->dataTransferHandle;
            for (unsigned int i = 0;i < response->responseCount;i++) {
                *insertionPoint++ = __pdr_data[extractionPoint++];
            }
            pdrTxState = 0;
            return;
        }
        // send the middle data (multi-part)
        response->completionCode = RESPONSE_SUCCESS;
        response->nextRecordHandle = (getNextRecord(request->recordHandle) <= pdrCount) ?
            getNextRecord(request->recordHandle) : 0;
        response->nextDataTransferHandle =
            request->dataTransferHandle + request->requestCount;
        response->transferFlag = 0x1;            // middle
        response->responseCount = request->requestCount;
        insertionPoint =
            txBuffer + sizeof(PldmResponseHeader) +
            sizeof(GetPdrResponse);
        extractionPoint = request->dataTransferHandle;
        for (int i = 0;i < request->requestCount;i++) {
            *insertionPoint++ = __pdr_data[extractionPoint++];
        }
        pdrTxState = 1;
        pdrNextHandle = request->dataTransferHandle + request->requestCount;
        return;
    }
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
void node::parseCommand()
{
    // cast the relevant portions of the header so that
    // they are easier to use later.
    PldmRequestHeader* rxHeader = (PldmRequestHeader*)rxBuffer;

    PldmResponseHeader* txHeader = (PldmResponseHeader*)txBuffer;
    txHeader->flags1 = rxHeader->flags1 | 0x80;
    txHeader->flags2 = rxHeader->flags2;
    txHeader->command = rxHeader->command;

    // switch based on the command type byte in the header
    switch (rxHeader->command) {
    case CMD_GET_SENSOR_READING:
        //GetSensorReading(cmdBuffer);
        break;
    case CMD_GET_STATE_SENSOR_READINGS:
        //GetStateSensorReadings(cmdBuffer);
        break;
    case CMD_SET_NUMERIC_EFFECTER_VALUE:
        //SetNumericEffecterValue(cmdBuffer);
        break;
    case CMD_GET_NUMERIC_EFFECTER_VALUE:
        //GetNumericEffecterValue(cmdBuffer);
        break;
    case CMD_SET_STATE_EFFECTER_STATES:
        //SetStateEffecterStates(cmdBuffer);
        break;
    case CMD_GET_STATE_EFFECTER_STATES:
        //GetStateEffecterStates(cmdBuffer);
        break;
    case CMD_GET_PDR_REPOSITORY_INFO:
    {
        GetPdrRepositoryInfoResponse* response = (GetPdrRepositoryInfoResponse*)(txBuffer + sizeof(*txHeader));
        response->completionCode = RESPONSE_SUCCESS;
        response->repositoryState = 0;           // available
        response->recordCount = pdrCount;
        response->repositorySize = __pdr_total_size;
        response->largestRecordSize = __pdr_max_record_size;
        response->dataTransferHandleTimeout = 0; // no timeout
        break;
    }
    case CMD_GET_PDR:
        processCommandGetPdr(rxHeader, txHeader);
        break;
    }
    return;
}

//*******************************************************************
// getResponse()
//
// a public entry point for the connector node to get a response 
// to the most recent command.
//
// parameters:
//    none
// returns:
//    a pointer to the response message
unsigned char* node::getResponse() {
    parseCommand();
    return txBuffer;
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
void node::putCommand(PldmRequestHeader* hdr, unsigned char* command, unsigned int size) {
    unsigned char* cp = rxBuffer;
    unsigned char* cmdptr = (unsigned char*)hdr;
    for (unsigned int i = 0;i < sizeof(*hdr);i++) *(cp++) = *(cmdptr++);
    for (unsigned int i = 0;i < size; i++) *(cp++) = *(command++);
}
