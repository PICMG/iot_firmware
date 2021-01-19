//*******************************************************************
//    clientNode.cpp
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
#include "clientNode.h"
#include "PdrRepository.h"
#include "pdrdata.h"
#include <iostream>

void transmitLong(mctp_struct *mctp, unsigned long data) {
    // send the data to the MCTP buffer in little-endian fashion
    unsigned char ch = (data&0xff);
    mctp_transmitFrameData(mctp,&ch,1);
    data = data>>8;
    ch = (data&0xff);
    mctp_transmitFrameData(mctp,&ch,1);
    data = data>>8;
    ch = (data&0xff);
    mctp_transmitFrameData(mctp,&ch,1);
    data = data>>8;
    ch = (data&0xff);
    mctp_transmitFrameData(mctp,&ch,1);
}

void transmitShort(mctp_struct *mctp, unsigned int data) {
    // send the data to the MCTP buffer in little-endian fashion
    unsigned char ch = (data&0xff);
    mctp_transmitFrameData(mctp,&ch,1);
    data = data>>8;
    ch = (data&0xff);
    mctp_transmitFrameData(mctp,&ch,1);
}

void transmitByte(mctp_struct *mctp, unsigned char data) {
    // send the data to the MCTP buffer in little-endian fashion
    mctp_transmitFrameData(mctp,&data,1);
}

//*******************************************************************
// clientNode()
//
// default constructor
clientNode::clientNode() {
    pdrCount = __pdr_number_of_records;
    nextByte = 0;
}

void clientNode::init(mctp_struct *mctp) {
    this->mctp=mctp;
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
unsigned int clientNode::pdrSize(unsigned int index) {
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
void clientNode::processCommandGetPdr(PldmRequestHeader* rxHeader) 
{
    static char pdrTxState = 0;
    static unsigned int pdrNextHandle;
    static unsigned long pdrRecord;
    unsigned char* insertionPoint;
    unsigned int extractionPoint;

    GetPdrCommand* request = (GetPdrCommand*)(mctp->rxBuffer + sizeof(*rxHeader));
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
            mctp_transmitFrameStart(mctp,sizeof(PldmResponseHeader)+4);
            transmitByte(mctp, rxHeader->flags1 | 0x80);
            transmitByte(mctp, rxHeader->flags2);
            transmitByte(mctp,rxHeader->command);
            mctp_transmitFrameData(mctp,&errorcode,1);  // response->completionCode = errorcode;
            transmitLong(mctp,0);                       // response->nextRecordHandle = 0;
            transmitLong(mctp,0);                       // response->NextRecordTransferHandle;
            transmitByte(mctp,0);                       // response->transferFlag = 0;
            transmitShort(mctp,0);                      // response->responseCount = 0;
            mctp_transmitFrameEnd(mctp);
            return;
        }
        if (request->requestCount >= pdrSize(request->recordHandle)) {
            // send the data (single part)
            mctp_transmitFrameStart(mctp, sizeof(PldmResponseHeader) + pdrSize(request->recordHandle)+4);
            transmitByte(mctp, rxHeader->flags1 | 0x80);
            transmitByte(mctp, rxHeader->flags2);
            transmitByte(mctp,rxHeader->command);
            transmitByte(mctp,RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
            transmitLong(mctp,(getNextRecord(request->recordHandle) <= pdrCount) ?
                getNextRecord(request->recordHandle) : 0);
            transmitLong(mctp, 0);                      //response->nextDataTransferHandle = 0;
            transmitByte(mctp, 0x05);                   // response->transferFlag = 0x05;   // start and end
            transmitShort(mctp, pdrSize(request->recordHandle)); // response->responseCount = pdrSize(request->recordHandle);
            unsigned int extractionPoint = getPdrOffset(request->recordHandle);
            // insert the pdr
            for (int i = 0;i < pdrSize(request->recordHandle); i++) {
                transmitByte(mctp, __pdr_data[extractionPoint++]);
            }
            mctp_transmitFrameEnd(mctp);
            return;
        }
        // start sending the data (multi-part)
        mctp_transmitFrameStart(mctp, sizeof(PldmResponseHeader) + request->requestCount+4);
        transmitByte(mctp, rxHeader->flags1 | 0x80);
        transmitByte(mctp, rxHeader->flags2);
        transmitByte(mctp,rxHeader->command);
        transmitByte(mctp,RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
        transmitLong(mctp,(getNextRecord(request->recordHandle) <= pdrCount) ?
                getNextRecord(request->recordHandle) : 0);
        transmitLong(mctp, getPdrOffset(request->recordHandle) + request->requestCount);
        transmitByte(mctp, 0x00);                // response->transferFlag = 0x0;   // start
        transmitShort(mctp, request->requestCount); // response->responseCount = request->requestCount;
        extractionPoint = request->dataTransferHandle+getPdrOffset(request->recordHandle);
        // insert the pdr
        for (int i = 0;i < request->requestCount;i++) {
            transmitByte(mctp, __pdr_data[extractionPoint++]);
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
            mctp_transmitFrameStart(mctp,sizeof(PldmResponseHeader)+4);
            transmitByte(mctp, rxHeader->flags1 | 0x80);
            transmitByte(mctp, rxHeader->flags2);
            transmitByte(mctp,rxHeader->command);
            mctp_transmitFrameData(mctp,&errorcode,1);  // response->completionCode = errorcode;
            transmitLong(mctp,0);                       // response->nextRecordHandle = 0;
            transmitLong(mctp,0);                       // response->NextRecordTransferHandle;
            transmitByte(mctp,0);                       // response->transferFlag = 0;
            transmitShort(mctp,0);                      // response->responseCount = 0;
            mctp_transmitFrameEnd(mctp);
            return;
        }
        if (request->requestCount + request->dataTransferHandle >= getPdrOffset(request->recordHandle)+pdrSize(request->recordHandle)) {
            // transfer end part of the data
            mctp_transmitFrameStart(mctp, sizeof(PldmResponseHeader) + pdrSize(request->recordHandle) -
                (request->dataTransferHandle-getPdrOffset(request->recordHandle)) + 4);
            transmitByte(mctp, rxHeader->flags1 | 0x80);
            transmitByte(mctp, rxHeader->flags2);
            transmitByte(mctp,rxHeader->command);
            transmitByte(mctp,RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
            transmitLong(mctp,(getNextRecord(request->recordHandle) <= pdrCount) ?
                getNextRecord(request->recordHandle) : 0);
            transmitLong(mctp, 0);                      // next data transfer handle
            transmitByte(mctp, 0x04);                   // response->transferFlag = 0x04;   end
            transmitShort(mctp, pdrSize(request->recordHandle) -
                (request->dataTransferHandle-getPdrOffset(request->recordHandle))); 
            unsigned int extractionPoint = request->dataTransferHandle;
            // insert the pdr
            for (int i = 0;i < request->requestCount;i++) {
                transmitByte(mctp, __pdr_data[extractionPoint++]);
            }
            mctp_transmitFrameEnd(mctp);
            // send the last part of the data
            //response->completionCode = RESPONSE_SUCCESS;
            //response->nextRecordHandle = (getNextRecord(request->recordHandle) <= pdrCount) ?
            //    getNextRecord(request->recordHandle) : 0;
            //response->nextDataTransferHandle = 0;
            //response->transferFlag = 0x04;   // end
            //response->responseCount =
            //     pdrSize(request->recordHandle) -
            //   (request->dataTransferHandle-getPdrOffset(request->recordHandle));
            //unsigned int extractionPoint = request->dataTransferHandle;
            //for (unsigned int i = 0;i < response->responseCount;i++) {
            //    *insertionPoint++ = __pdr_data[extractionPoint++];
            //}
            pdrTxState = 0;
            return;
        }
        // send the middle data (multi-part)
        mctp_transmitFrameStart(mctp, sizeof(PldmResponseHeader) + request->requestCount + 4);
        transmitByte(mctp, rxHeader->flags1 | 0x80);
        transmitByte(mctp, rxHeader->flags2);
        transmitByte(mctp,rxHeader->command);
        transmitByte(mctp,RESPONSE_SUCCESS);        // response->completionCode = RESPONSE_SUCCESS;
        transmitLong(mctp,(getNextRecord(request->recordHandle) <= pdrCount) ?
            getNextRecord(request->recordHandle) : 0);
        transmitLong(mctp, request->dataTransferHandle + request->requestCount); // next data transfer handle
        transmitByte(mctp, 0x01);                   // response->transferFlag = 0x01;   middle
        transmitShort(mctp, request->requestCount); 
        unsigned int extractionPoint = request->dataTransferHandle;
        // insert the pdr
        for (int i = 0;i < request->requestCount;i++) {
            transmitByte(mctp, __pdr_data[extractionPoint++]);
        }
        mctp_transmitFrameEnd(mctp);
        //response->completionCode = RESPONSE_SUCCESS;
        //response->nextRecordHandle = (getNextRecord(request->recordHandle) <= pdrCount) ?
        //    getNextRecord(request->recordHandle) : 0;
        //response->nextDataTransferHandle =
        //    request->dataTransferHandle + request->requestCount;
        //response->transferFlag = 0x1;            // middle
        //response->responseCount = request->requestCount;
        //extractionPoint = request->dataTransferHandle;
        //for (int i = 0;i < request->requestCount;i++) {
        //    *insertionPoint++ = __pdr_data[extractionPoint++];
        //}
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
void clientNode::parseCommand()
{
    // cast the relevant portions of the header so that
    // they are easier to use later.
    PldmRequestHeader* rxHeader = (PldmRequestHeader*)mctp_getPacket(mctp);

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
        mctp_transmitFrameStart(mctp,sizeof(GetPdrRepositoryInfoResponse) + sizeof(PldmRequestHeader) + 4);
        transmitByte(mctp, rxHeader->flags1 | 0x80);
        transmitByte(mctp, rxHeader->flags2);
        transmitByte(mctp,rxHeader->command);
        transmitByte(mctp, RESPONSE_SUCCESS);   // completion code
        transmitByte(mctp, 0);                  // repository state = available
        for (int i=0;i<13;i++) transmitByte(mctp,0); // update time
        for (int i=0;i<13;i++) transmitByte(mctp,0); // oem update time
        transmitLong(mctp,pdrCount);            // pdr record count
        transmitLong(mctp, __pdr_total_size);   // repository size
        transmitLong(mctp, __pdr_max_record_size);  // record size
        transmitByte(mctp, 0);                  // no timeout
        mctp_transmitFrameEnd(mctp);
        break;
    }
    case CMD_GET_PDR:
        processCommandGetPdr(rxHeader);
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
void clientNode::putCommand(PldmRequestHeader* hdr, unsigned char* command, unsigned int size) {
    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + size +4);
    mctp_transmitFrameData(mctp,(unsigned char *)hdr,sizeof(PldmRequestHeader));
    mctp_transmitFrameData(mctp,command,size);
    mctp_transmitFrameEnd(mctp);
}

unsigned char* clientNode::getResponse(void) {
    while (!mctp_isPacketAvailable(mctp)) {
        mctp_updateRxFSM(mctp);
    }
    return mctp_getPacket(mctp);
}

//*******************************************************************
// setNumericEffecterValue()
//
// This function contains implementation of the setNumericEffectorValue command
// specified by DMTF specification DSP0248_1.1.0
//
// parameters:
//	  effecterID - the ID of the effecter being referenced
//    effecterPdr - the PDR for the effecter
//    data - the new value for the numeric effecter
// returns:
//    bool - whether the change was successful
bool clientNode::setNumericEffecterValue(unsigned long effecterID, GenericPdr* effecterpdr, double data){
    unsigned char buffer[7]; 
    std::string dataSize   = effecterpdr->getValue("effecterDataSize");
    double resolution = atof(effecterpdr->getValue("resolution").c_str());
    double offset     = atof(effecterpdr->getValue("offset").c_str());
    
    // perform the unit conversion (with rounding)
    unsigned long scaledData = (data - offset)/resolution + 0.5;
    unsigned int body_len = 0;

    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_NUMERIC_EFFECTER_VALUE;
    *((uint16*)buffer) = effecterID;

    if(dataSize=="uint8"){
        buffer[2] = 0; //effecter data size is uint8
        buffer[3] = (uint8)scaledData;
        body_len = 4;
    }
    else if(dataSize=="sint8"){
        buffer[2] = 1; //effecter data size is sint8
        buffer[3] = (sint8)scaledData;
        body_len = 4;
    }
    else if(dataSize=="uint16"){
        buffer[2] = 2; //effecter data size is uint16
        *((uint16*)(&buffer[3])) = (uint16)scaledData;
        body_len = 5;
    }
    else if(dataSize=="sint16"){
        buffer[2] = 3; //effecter data size is sint16
        *((sint16*)(&buffer[3])) = (sint16)scaledData;
        body_len = 5;
    }
    else if(dataSize=="uint32"){
        buffer[2] = 4; //effecter data size is uint32
        *((uint32*)(&buffer[3])) = (uint32)scaledData;
        body_len = 7;
    }
    else if(dataSize=="sint32"){
        buffer[2] = 5; //effecter data size is sint32
        *((sint32*)(&buffer[3])) = (sint32)scaledData;
        body_len = 7;
    }

    putCommand(&header, buffer, body_len);

    PldmResponseHeader* response;
    response = (PldmResponseHeader*)getResponse();
    if(response->completionCode==RESPONSE_SUCCESS){
        return true;
    }else{
        return false;
    }
}


//*******************************************************************
// getNumericEffecterValue()
//
// This function contains implementation of the getNumericEffectorValue command
// specified by DMTF specification DSP0248_1.1.0
//
// parameters:
//	  effecterID - the ID of the effecter being referenced
//    effecterPdr - the PDR for the effecter
// returns:
//    double - the value of the effecter or -1 for failure
double clientNode::getNumericEffecterValue(unsigned long effecterID, GenericPdr* effecterpdr){
    unsigned char buffer[2]; 
    double resolution = atof(effecterpdr->getValue("resolution").c_str());
    double offset     = atof(effecterpdr->getValue("offset").c_str());
    
    unsigned int body_len = 2;

    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_GET_NUMERIC_EFFECTER_VALUE;
    *((uint16*)buffer) = effecterID;
    
    // send the command
    putCommand(&header, buffer, body_len);

    // recieve the response
    PldmResponseHeader* response;
    response = (PldmResponseHeader*)getResponse();
    
    // process the response
    if(response->completionCode==RESPONSE_SUCCESS){
        char* body = (char*)(response+1);
        double data = 0;

        // get the data size
        uint8 dataSize = *((uint8*)(&body[0]));

        // switch depending on data size
        if(dataSize==0){ // data size is uint8
            uint8 value = *((uint8*)(&body[3]));
            data = value;
        }
        else if(dataSize==1){ // data size is sint8
            sint8 value = *((sint8*)(&body[3]));
            data = value;
        } 
        else if(dataSize==2){ // data size is uint16
            uint16 value = *((uint16*)(&body[4]));
            data = value;
        } 
        else if(dataSize==3){ // data size is sint16
            sint16 value = *((sint16*)(&body[4]));
            data = value;
        } 
        else if(dataSize==4){ // data size is uint32
            uint32 value = *((uint32*)(&body[6]));
            data = value;
        } 
        else if(dataSize==5){ // data size is sint32
            sint32 value = *((sint32*)(&body[6]));
            data = value;
        }

        // perform the unit conversion (with rounding)
        double scaledData = (data * resolution)+offset;

        return scaledData;
    }else{
        return -1;
    }
}

//*******************************************************************
// setStateEffecterStates()
//
// This function contains implementation of the setStateEffectorStates command
// specified by DMTF specification DSP0248_1.1.0
//
// parameters:
//	  effecterID - the ID of the effecter being referenced
//    effecterPdr - the PDR for the effecter
//    data - the new state value for the state effecter
// returns:
//    bool - whether the change was successful
bool clientNode::setStateEffecterStates(unsigned long effecterID, GenericPdr* effecterpdr, enum8 effecterState){
    // send command
    unsigned char buffer[5]; 
    unsigned int body_len = 5;
    
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_STATE_EFFECTER_STATES;
    *((uint16*)buffer) = effecterID;

    uint8 compositeEffecterCount = 1;
    enum8 setRequest = 1;

    buffer[2] = compositeEffecterCount;
    buffer[3] = setRequest;
    buffer[4] = effecterState;

    putCommand(&header, buffer, body_len);

    // get response
    PldmResponseHeader* response;
    response = (PldmResponseHeader*)getResponse();
    if(response->completionCode==RESPONSE_SUCCESS){
        return true;
    }else{
        return false;
    }
}

//*******************************************************************
// getStateEffecterStates()
//
// This function contains implementation of the getStateEffecterStates command
// specified by DMTF specification DSP0248_1.1.0
//
// parameters:
//	  effecterID - the ID of the effecter being referenced
//    effecterPdr - the PDR for the effecter
// returns:
//    enum8 - state value
enum8 clientNode::getStateEffecterStates(unsigned long effecterID, GenericPdr* effecterpdr){
    unsigned char buffer[2]; 
    
    unsigned int body_len = 2;

    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_GET_STATE_EFFECTER_STATES;
    *((uint16*)buffer) = effecterID;
    
    // send the command
    putCommand(&header, buffer, body_len);

    // recieve the response
    PldmResponseHeader* response;
    response = (PldmResponseHeader*)getResponse();
    char* body = (char*)(response+1);
    uint8 value = *((uint8*)(&body[3]));
    return value;
}

//*******************************************************************
// getSensorReading()
//
// This function contains implementation of the getSensorReading command
// specified by DMTF specification DSP0248_1.1.0
//
// parameters:
//	  sensorID - the ID of the sensor being referenced
//    sensorPdr - the PDR for the sensor
// returns:
//    double - the value of the sensor or -1 for failure
double clientNode::getSensorReading(unsigned long sensorID, GenericPdr* sensorpdr){
    unsigned char buffer[3]; 
    double resolution = atof(sensorpdr->getValue("resolution").c_str());
    double offset     = atof(sensorpdr->getValue("offset").c_str());
    
    unsigned int body_len = 3;

    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_GET_SENSOR_READING;
    *((uint16*)buffer) = sensorID;
    
    // this changes the state of the state machine depending on value.
    // if true, sets the sensor state to "initilizing" when called.
    bool8 rearmEventState = true;
    buffer[2] = rearmEventState;

    // sending the command
    putCommand(&header, buffer, body_len);

    // recieving the response
    PldmResponseHeader* response;
    response = (PldmResponseHeader*)getResponse();
    
    // processing the response
    if(response->completionCode==RESPONSE_SUCCESS){
        char* body = (char*)(response+1);
        double data = 0;

        // getting the data size
        uint8 dataSize = *((uint8*)(&body[0]));

        // switch depending on data size
        if(dataSize==0){ // data size is uint8
            uint8 value = *((uint8*)(&body[6]));
            data = value;
        }
        else if(dataSize==1){ // data size is sint8
            sint8 value = *((sint8*)(&body[6]));
            data = value;
        } 
        else if(dataSize==2){ // data size is uint16
            uint16 value = *((uint16*)(&body[6]));
            data = value;
        } 
        else if(dataSize==3){ // data size is sint16
            sint16 value = *((sint16*)(&body[6]));
            data = value;
        } 
        else if(dataSize==4){ // data size is uint32
            uint32 value = *((uint32*)(&body[6]));
            data = value;
        } 
        else if(dataSize==5){ // data size is sint32
            sint32 value = *((sint32*)(&body[6]));
            data = value;
        }

        // perform the unit conversion (with rounding)
        double scaledData = (data * resolution)+offset;
    
        return scaledData;

    }else{
        return -1;
    }
}

//*******************************************************************
// getStateSensorReadings()
//
// This function contains implementation of the getStateSensorReadings command
// specified by DMTF specification DSP0248_1.1.0
//
// parameters:
//	  sensorID - the ID of the sensor being referenced
//    sensorPdr - the PDR for the sensor
// returns:
//    enum8 - state value
enum8 clientNode::getStateSensorReadings(unsigned long sensorID, GenericPdr* sensorpdr){
    unsigned char buffer[4]; 
    
    unsigned int body_len = 4;

    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_GET_STATE_SENSOR_READINGS;
    *((uint16*)buffer) = sensorID;
    buffer[2]=1;
    buffer[3]=0;        
    
    // send the command
    putCommand(&header, buffer, body_len);

    // recieve the response
    PldmResponseHeader* response;
    response = (PldmResponseHeader*)getResponse();
    char* body = (char*)(response+1);
    uint8 value = *((uint8*)(&body[3]));
            
    return value;
}

//*******************************************************************
// setNumericEffecterEnable()
//
// This function contains implementation of the setNumericEffecterEnable command
// specified by DMTF specification DSP0248_1.1.0
//
// parameters:
//	  effecterID - the ID of the effecter being referenced
//    effecterPdr - the PDR for the effecter
//    enableState - the enable state of the effecter
// returns:
//    bool - whether the change was successful
bool clientNode::setNumericEffecterEnable(unsigned long effecterID, GenericPdr* effecterpdr, uint8 enableState){   
    // send command
    unsigned char buffer[3]; 
    unsigned int body_len = 3;
    
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_NUMERIC_EFFECTER_ENABLE;
    *((uint16*)buffer) = effecterID;

    buffer[2]=enableState;

    putCommand(&header, buffer, body_len);

    // get response
    PldmResponseHeader* response;
    response = (PldmResponseHeader*)getResponse();
    if(response->completionCode==RESPONSE_SUCCESS){
        return true;
    }else{
        return false;
    }
}

//*******************************************************************
// setStateEffecterEnables()
//
// This function contains implementation of the setStateEffecterEnables command
// specified by DMTF specification DSP0248_1.1.0
//
// parameters:
//	  effecterID - the ID of the effecter being referenced
//    effecterPdr - the PDR for the effecter
//    enableState - the enable state of the effecter
// returns:
//    bool - whether the change was successful
bool clientNode::setStateEffecterEnables(unsigned long effecterID, GenericPdr* effecterpdr, uint8 enableState){   
    // send command
    unsigned char buffer[5]; 
    unsigned int body_len = 5;
    
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_STATE_EFFECTER_ENABLES;
    *((uint16*)buffer) = effecterID;

    buffer[2]=1;
    buffer[3]=enableState;
    buffer[4]=1;

    putCommand(&header, buffer, body_len);

    // get response
    PldmResponseHeader* response;
    response = (PldmResponseHeader*)getResponse();
    if(response->completionCode==RESPONSE_SUCCESS){
        return true;
    }else{
        return false;
    }
}
