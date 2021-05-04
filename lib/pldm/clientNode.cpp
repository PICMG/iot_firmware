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
}

void clientNode::init(mctp_struct *mctp) {
    this->mctp=mctp;
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
//    effecterPdr - the PDR for the effecter
//    data - the new value for the numeric effecter
// returns:
//    bool - whether the change was successful
bool clientNode::setNumericEffecterValue(GenericPdr* effecterpdr, double data){
    unsigned char buffer[7]; 
    std::string dataSize   = effecterpdr->getValue("effecterDataSize");
    double resolution = atof(effecterpdr->getValue("resolution").c_str());
    double offset     = atof(effecterpdr->getValue("offset").c_str());
    
    // perform the unit conversion (with rounding)
    unsigned long scaledData = (data - offset)/resolution + 0.5;
    unsigned int body_len = 0;

    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_NUMERIC_EFFECTER_VALUE;
    *((uint16*)buffer) = atoi(effecterpdr->getValue("effecterID").c_str());

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
//    effecterPdr - the PDR for the effecter
// returns:
//    double - the value of the effecter or -1 for failure
double clientNode::getNumericEffecterValue(GenericPdr* effecterpdr){
    unsigned char buffer[2]; 
    double resolution = atof(effecterpdr->getValue("resolution").c_str());
    double offset     = atof(effecterpdr->getValue("offset").c_str());
    
    unsigned int body_len = 2;

    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_GET_NUMERIC_EFFECTER_VALUE;
    *((uint16*)buffer) = atoi(effecterpdr->getValue("effecterID").c_str());
    
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
//    effecterPdr - the PDR for the effecter
//    data - the new state value for the state effecter
// returns:
//    bool - whether the change was successful
bool clientNode::setStateEffecterStates(GenericPdr* effecterpdr, enum8 effecterState){
    // send command
    unsigned char buffer[5]; 
    unsigned int body_len = 5;
    
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_STATE_EFFECTER_STATES;
    *((uint16*)buffer) = atoi(effecterpdr->getValue("effecterID").c_str());

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
//    effecterPdr - the PDR for the effecter
// returns:
//    enum8 - state value
enum8 clientNode::getStateEffecterStates(GenericPdr* effecterpdr){
    unsigned char buffer[2]; 
    
    unsigned int body_len = 2;

    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_GET_STATE_EFFECTER_STATES;
    *((uint16*)buffer) = atoi(effecterpdr->getValue("effecterID").c_str());
    
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
//    sensorPdr - the PDR for the sensor
// returns:
//    double - the value of the sensor or -1 for failure
double clientNode::getSensorReading(GenericPdr* sensorpdr){
    unsigned char buffer[3]; 
    double resolution = atof(sensorpdr->getValue("resolution").c_str());
    double offset     = atof(sensorpdr->getValue("offset").c_str());
    
    unsigned int body_len = 3;

    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_GET_SENSOR_READING;
    *((uint16*)buffer) = atoi(sensorpdr->getValue("sensorID").c_str());
    
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
enum8 clientNode::getStateSensorReadings(GenericPdr* sensorpdr){
    unsigned char buffer[4]; 
    
    unsigned int body_len = 4;

    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_GET_STATE_SENSOR_READINGS;
    *((uint16*)buffer) = atoi(sensorpdr->getValue("sensorID").c_str());
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
//    effecterPdr - the PDR for the effecter
//    enableState - the enable state of the effecter
// returns:
//    bool - whether the change was successful
bool clientNode::setNumericEffecterEnable(GenericPdr* effecterpdr, uint8 enableState){   
    // send command
    unsigned char buffer[3]; 
    unsigned int body_len = 3;
    
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_NUMERIC_EFFECTER_ENABLE;
    *((uint16*)buffer) = atoi(effecterpdr->getValue("effecterID").c_str());

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
//    effecterPdr - the PDR for the effecter
//    enableState - the enable state of the effecter
// returns:
//    bool - whether the change was successful
bool clientNode::setStateEffecterEnables(GenericPdr* effecterpdr, uint8 enableState){   
    // send command
    unsigned char buffer[5]; 
    unsigned int body_len = 5;
    
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_STATE_EFFECTER_ENABLES;
    *((uint16*)buffer) = atoi(effecterpdr->getValue("effecterID").c_str());

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

//*******************************************************************
// setNumericEffecterValue()
//
// This function contains implementation of the setNumericEffectorValue command
// specified by DMTF specification DSP0248_1.1.0
//
// parameters:
//    pdr - the PDR for the effecter
//    params - a map of the parameters for this function.  The values expected are:
//       0 = the effecter data size.  This should be expressed as the 
//           enumeration name
//       1 = the new value to set.
// returns:
//    a map of response parameters.  The values are:
//       0 = completion code (as an enumerated value)
map<int,string> clientNode::setNumericEffecterValue(GenericPdr* pdr, map<int,string> &params) {
    uint8 buffer[7];
    map<int,string> result;
    
    if ((!pdr->keyExists("resolution"))||(!pdr->keyExists("offset"))||
        (!pdr->keyExists("effecterDataSize"))||(!pdr->keyExists("effecterID"))) {
        // this pdr is not for an numeric effecter, or it is missing fields
        result.insert(pair<int,string>(0,to_string(RESPONSE_INVALID_EFFECTER_ID)));
        return result;
    }
    
    // check the number of parameters
    if (params.size() != 2) {
        result.insert(pair<int,string>(0,to_string(RESPONSE_ERROR_INVALID_DATA)));
        return result;
    }
    if ((params.count(0)==0)||(params.count(1)==0)) {
        result.insert(pair<int,string>(0,to_string(RESPONSE_ERROR_INVALID_DATA)));
        return result;        
    }

    // get the conversion paramters from the pdr
    double resolution = atof(pdr->getValue("resolution").c_str());
    double offset     = atof(pdr->getValue("offset").c_str());

    // perform the unit conversion (with rounding) back to device units
    long data = atol(params.find(1)->second.c_str());
    unsigned long scaledData = (data - offset)/resolution + 0.5;
    unsigned int body_len = 0;

    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_NUMERIC_EFFECTER_VALUE;
    *((uint16*)buffer) = atoi(pdr->getValue("effecterID").c_str());

    string dataSize = params.find(0)->second.c_str();
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
    else {
        // unknown data size
        result.insert(pair<int,string>(0,to_string(RESPONSE_ERROR_INVALID_DATA)));
        return result;        
    }

    // send the command to the device
    putCommand(&header, buffer, body_len);

    // await a response
    PldmResponseHeader* response;
    response = (PldmResponseHeader*)getResponse();
    result.insert(pair<int,string>(0,to_string((uint16)response->completionCode)));

    // return the result
    return result;
}

//*******************************************************************
// getNumericEffecterValue()
//
// This function contains implementation of the getNumericEffectorValue command
// specified by DMTF specification DSP0248_1.1.0
//
// parameters:
//    pdr - the PDR for the effecter
//    params - a map of the parameters for this function.  The values expected are:
//          <<empty>>
// returns:
//    a map of response parameters.  The values are:
//       0 = completion code (as an enumerated value)
//       1 = effecter data size
//       2 = effecter operational state
//       3 = pending value
//       4 = present value
map<int,string> clientNode::getNumericEffecterValue(GenericPdr* pdr, map<int,string> &params) {
    map<int,string> result;

    if ((!pdr->keyExists("resolution"))||(!pdr->keyExists("offset"))||
        (!pdr->keyExists("effecterDataSize")||(!pdr->keyExists("effecterID")))) {
        // this pdr is not for an numeric effecter, or it is missing fields
        result.insert(pair<int,string>(0,to_string(RESPONSE_INVALID_EFFECTER_ID)));
        return result;
    }

    // check the number of parameters
    if (params.size() != 0) {
        result.insert(pair<int,string>(0,to_string(RESPONSE_ERROR_INVALID_DATA)));
        return result;
    }

    // get the paramters to convert form raw units to real-world units
    double resolution = atof(pdr->getValue("resolution").c_str());
    double offset     = atof(pdr->getValue("offset").c_str());
   
    // construct the header and send the command
    unsigned char buffer[2]; 
    unsigned int body_len = 2;
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_GET_NUMERIC_EFFECTER_VALUE;
    *((uint16*)buffer) = atoi(pdr->getValue("effecterID").c_str());
    putCommand(&header, buffer, body_len);

    // recieve the response
    PldmResponseHeader* response;
    response = (PldmResponseHeader*)getResponse();
    
    // add the response code to the outputs
    result.insert(pair<int,string>(0,to_string((uint16)response->completionCode)));

    // process the response
    if(response->completionCode==RESPONSE_SUCCESS){
        char* body = (char*)(response+1);

        // extract the effecter data size
        uint8 dataSize = *((uint8*)(&body[0]));
        map<string,unsigned int> enums = pdr->getEnumOptions("effecterDataSize"); 
        for (map<string,unsigned int>::iterator it = enums.begin(); it!= enums.end(); ++it) {
            if ((*it).second==dataSize) {
                result.insert(pair<int,string>(1,to_string((uint16)response->completionCode)));
                break;    
            }
        }
        // provide a default value just in case.
        result.insert(pair<int,string>(1,"uint8"));
        
        // get the operational state
        switch (body[1]) {
            case 0:
                result.insert(pair<int,string>(2,"enabled-updatePending"));
                break;
            case 1:
                result.insert(pair<int,string>(2,"enabled-noUpdatePending"));
                break;
            case 2:
                result.insert(pair<int,string>(2,"disabled"));
                break;
            case 3:
                result.insert(pair<int,string>(2,"unavailable"));
                break;
            case 4:
                result.insert(pair<int,string>(2,"statusUnknown"));
                break;
            case 5:
                result.insert(pair<int,string>(2,"failed"));
                break;
            case 6:
                result.insert(pair<int,string>(2,"initializing"));
                break;
            case 7:
                result.insert(pair<int,string>(2,"shuttingDown"));
                break;
            case 8:
                result.insert(pair<int,string>(2,"inTest"));
                break;
            default:
                result.insert(pair<int,string>(2,"unknown"));
        }

        // switch depending on data size
        if(dataSize==0){ // data size is uint8
            uint8 value = *((uint8*)(&body[2]));
            result.insert(pair<int,string>(3,to_string(((double)value)*resolution + offset)));
            value = *((uint8*)(&body[2]));
            result.insert(pair<int,string>(4,to_string(((double)value)*resolution + offset)));
        }
        else if(dataSize==1){ // data size is sint8
            sint8 value = *((sint8*)(&body[2]));
            result.insert(pair<int,string>(3,to_string(((double)value)*resolution + offset)));
            value = *((sint8*)(&body[2]));
            result.insert(pair<int,string>(4,to_string(((double)value)*resolution + offset)));
        } 
        else if(dataSize==2){ // data size is uint16
            uint16 value = *((uint16*)(&body[2]));
            result.insert(pair<int,string>(3,to_string(((double)value)*resolution + offset)));
            value = *((uint16*)(&body[4]));
            result.insert(pair<int,string>(4,to_string(((double)value)*resolution + offset)));
        } 
        else if(dataSize==3){ // data size is sint16
            sint16 value = *((sint16*)(&body[2]));
            result.insert(pair<int,string>(3,to_string(((double)value)*resolution + offset)));
            value = *((sint16*)(&body[4]));
            result.insert(pair<int,string>(4,to_string(((double)value)*resolution + offset)));
        } 
        else if(dataSize==4){ // data size is uint32
            uint32 value = *((uint32*)(&body[2]));
            result.insert(pair<int,string>(3,to_string(((double)value)*resolution + offset)));
            value = *((uint32*)(&body[6]));
            result.insert(pair<int,string>(4,to_string(((double)value)*resolution + offset)));
        } 
        else if(dataSize==5){ // data size is sint32
            sint32 value = *((sint32*)(&body[2]));
            result.insert(pair<int,string>(3,to_string(((double)value)*resolution + offset)));
            value = *((sint32*)(&body[6]));
            result.insert(pair<int,string>(4,to_string(((double)value)*resolution + offset)));
        }
    }

    return result;
}

//*******************************************************************
// setStateEffecterStates()
//
// This function contains implementation of the getStateEffecterStates command
// specified by DMTF specification DSP0248_1.1.0
//
// parameters:
//    effecterPdr - the PDR for the effecter
//    params - a map of the parameters for this function.  The values expected are:
//       0 = composite effecter count
//       1 = set request (as a string)
//       2 = effecter state (as a string)
//       ..  more set requests and effecter states as needed for effecter count
// returns:
//    a map of response parameters.  The values are:
//       0 = completion code (as an enumerated value)
map<int,string> clientNode::setStateEffecterStates(GenericPdr* pdr, map<int,string> &params, PdrRepository &repo) {
    map<int,string> result;

    if ((!pdr->keyExists("compositeEffecterCount"))||(!pdr->keyExists("stateSetID"))||
        (!pdr->keyExists("effecterID"))) {
        // this pdr is not for an state effecter, or it is missing fields
        result.insert(pair<int,string>(0,to_string(RESPONSE_INVALID_EFFECTER_ID)));
        return result;
    }

    // check the number of parameters
    if (params.size() < 3) {
        result.insert(pair<int,string>(0,to_string(RESPONSE_ERROR_INVALID_DATA)));
        return result;
    }
    if ((params.count(0)==0)||(params.count(1)==0)||(params.count(2)==0)) {
        result.insert(pair<int,string>(0,to_string(RESPONSE_ERROR_INVALID_DATA)));
        return result;        
    }

    // get the state set for this endpoint
    unsigned long setId = atol(pdr->getValue("stateSetID").c_str());

    // send command
    unsigned char buffer[64]; 
    
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_STATE_EFFECTER_STATES;
    *((uint16*)buffer) = atoi(pdr->getValue("effecterID").c_str());

    uint8 compositeEffecterCount = atoi(params[0].c_str());
    unsigned int body_len = 3 + 2*compositeEffecterCount;
    buffer[2] = compositeEffecterCount;
    for (unsigned int i=0; i<compositeEffecterCount;i++) {
        uint8 setRequest;
        sint32 stateNumber = repo.getStateNumberFromName(setId, params[2+i*2]);
        if (stateNumber<0) {
            // invalid state
            result.insert(pair<int,string>(0,to_string(RESPONSE_INVALID_STATE_VALUE)));
            return result;
        }
        string opState = params[1+i*2];
        if (opState == "noChange") {
            setRequest = 0;
        }
        else if (opState == "requestSet") {
            setRequest = 1;
        }
        else {
            // invalid set request
            result.insert(pair<int,string>(0,to_string(RESPONSE_INVALID_STATE_VALUE)));
            return result;
        }
        buffer[3+i*2] = setRequest;
        buffer[4+i*2] = stateNumber;
    }

    // send the command to the device
    putCommand(&header, buffer, body_len);

    // await a response
    PldmResponseHeader* response;
    response = (PldmResponseHeader*)getResponse();
    result.insert(pair<int,string>(0,to_string((uint16)response->completionCode)));

    // return the result
    return result;
}

//*******************************************************************
// getStateEffecterStates()
//
// This function contains implementation of the getStateEffecterStates command
// specified by DMTF specification DSP0248_1.1.0
//
// parameters:
//    effecterPdr - the PDR for the effecter
//    params - a map of the parameters for this function.  The values expected are:
//      <<none>>
// returns:
//    a map of response parameters.  The values are:
//       0 = completion code (as an enumerated value)
//       1 = composite effecter count (as an integer)
//       2 = operational state (as a string)
//       3 = present state (as a string)
//       4 = pendding state (as a string)
//       ..  more set requests and effecter states as needed for effecter count
map<int,string> clientNode::getStateEffecterStates(GenericPdr* pdr, map<int,string> &params, PdrRepository &repo) {
    map<int,string> result;

    // check the right keys
    if ((!pdr->keyExists("effecterID"))||(!pdr->keyExists("stateSetID"))) {
        // this pdr is not for an state effecter, or it is missing fields
        result.insert(pair<int,string>(0,to_string(RESPONSE_INVALID_EFFECTER_ID)));
        return result;
    }

    // check the number of parameters
    if (params.size() !=0) {
        result.insert(pair<int,string>(0,to_string(RESPONSE_ERROR_INVALID_DATA)));
        return result;
    }

    // prepare the request
    unsigned char buffer[2];     
    unsigned int body_len = 2;
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_GET_STATE_EFFECTER_STATES;
    *((uint16*)buffer) = atoi(pdr->getValue("effecterID").c_str());
    
    // send the command
    putCommand(&header, buffer, body_len);

    // recieve the response
    PldmResponseHeader* response;
    response = (PldmResponseHeader*)getResponse();
    char* body = (char*)(response+1);

    // get the state set for this endpoint
    unsigned long setId = atol(pdr->getValue("stateSetID").c_str());
    map<unsigned int,string> states = repo.getStateSet(setId);

    // build the result structure from the response data
    result.insert(pair<int,string>(0,to_string((uint16)response->completionCode)));

    uint8 compositeEffecterCount = body[0];
    result.insert(pair<int,string>(1,to_string((uint16)compositeEffecterCount)));

    for (uint8 i = 0; i<compositeEffecterCount; i++) {
        // get the operational state
        uint8 effecterOperationalState = body[1+i*3];
        switch (effecterOperationalState) {
            case 0:
                result.insert(pair<int,string>(2+i*3,"enabled-updatePending"));
                break;
            case 1:
                result.insert(pair<int,string>(2+i*3,"enabled-noUpdatePending"));
                break;
            case 2:
                result.insert(pair<int,string>(2+i*3,"disabled"));
                break;
            case 3:
                result.insert(pair<int,string>(2+i*3,"unavailable"));
                break;
            case 4:
                result.insert(pair<int,string>(2+i*3,"statusUnknown"));
                break;
            case 5:
                result.insert(pair<int,string>(2+i*3,"failed"));
                break;
            case 6:
                result.insert(pair<int,string>(2+i*3,"initializing"));
                break;
            case 7:
                result.insert(pair<int,string>(2+i*3,"shuttingDown"));
                break;
            case 8:
                result.insert(pair<int,string>(2+i*3,"inTest"));
                break;
            default:
                result.insert(pair<int,string>(2+i*3,"unknown"));
        }
        uint8 pendingState = body[2+i*3];
        if (states.count(pendingState)) {
            result.insert(pair<int,string>(2+i*3,states[pendingState]));
        } 
        else {
            result.insert(pair<int,string>(2+i*3,"unknown"));
        }
        uint8 presentState = body[3+i*3];
        if (states.count(presentState)) {
            result.insert(pair<int,string>(3+i*3,states[presentState]));
        } 
        else {
            result.insert(pair<int,string>(3+i*3,"unknown"));
        }
    }
    // return the result
    return result;
}

//*******************************************************************
// getSensorReading()
//
// This function contains implementation of the getSensorReading command
// specified by DMTF specification DSP0248_1.1.0
//
// parameters:
//    pdr - the PDR for the effecter
//    params - a map of the parameters for this function.  The values expected are:
//       0 = rearmEventState (true/false)
// returns:
//    a map of response parameters.  The values are:
//       0 = completion code (as an enumerated value)
//       1 = sensor data size (as string)
//       2 = sensor operational state (as string)
//       3 = sensor event message enable (as string)
//       4 = present state (as string)
//       5 = previous state (as string)
//       6 = event state (as string)
//       7 = present reading
map<int,string> clientNode::getSensorReading(GenericPdr* pdr, map<int,string> &params) {
    static const map<int,string> state = {
        {0,"Unknown"},{1,"Normal"},{2,"Warning"},{3,"Critical"},{4,"Fatal"},
        {5,"LowerWarning"},{6,"LowerCritical"},{7,"LowerFatal"},
        {8,"UpperWarning"},{9,"UpperCritical"},{10,"UpperFatal"}};
    static const map<int,string> opState = {
        {0,"enabled-updatePending"},{1,"enabled-noUpdatePending"},{2,"disabled"},
        {3,"unavailable"},{4,"statusUnknown"},{5,"failed"},{6,"initializing"},
        {7,"shuttingDown"},{8,"inTest"}};
    static const map<int,string> eventMsgEnable = {
        {0,"noEventGeneration"},{1,"eventsDisabled"},{2,"eventsEnabled"},{3,"opEventsOnlyEnabled"},
        {4,"stateEventsOnlyEnabled"}};
    
    map<int,string> result;

    if ((!pdr->keyExists("resolution"))||(!pdr->keyExists("offset"))||
        (!pdr->keyExists("sensorDataSize")||(!pdr->keyExists("sensorID")))) {
        // this pdr is not for an numeric sensor, or it is missing fields
        result.insert(pair<int,string>(0,to_string(RESPONSE_INVALID_EFFECTER_ID)));
        return result;
    }

    // check the number of parameters
    if (params.size() != 1) {
        result.insert(pair<int,string>(0,to_string(RESPONSE_ERROR_INVALID_DATA)));
        return result;
    }
    if (!params.count(0)) {
        result.insert(pair<int,string>(0,to_string(RESPONSE_ERROR_INVALID_DATA)));
        return result;
    }

    // get the paramters to convert form raw units to real-world units
    double resolution = atof(pdr->getValue("resolution").c_str());
    double offset     = atof(pdr->getValue("offset").c_str());
   
    // construct the header and send the command
    unsigned char buffer[3]; 
    unsigned int body_len = 3;
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_GET_NUMERIC_EFFECTER_VALUE;
    *((uint16*)buffer) = atoi(pdr->getValue("sensorID").c_str());
    buffer[2] = atoi(params[0].c_str());
    putCommand(&header, buffer, body_len);

    // recieve the response
    PldmResponseHeader* response;
    response = (PldmResponseHeader*)getResponse();
    
    // add the response code to the outputs
    result.insert(pair<int,string>(0,to_string((uint16)response->completionCode)));

    // process the response
    if(response->completionCode==RESPONSE_SUCCESS){
        char* body = (char*)(response+1);

        // result 1 = sensor data size (as string)
        // extract the effecter data size
        uint8 dataSize = *((uint8*)(&body[0]));
        map<string,unsigned int> enums = pdr->getEnumOptions("effecterDataSize"); 
        for (map<string,unsigned int>::iterator it = enums.begin(); it!= enums.end(); ++it) {
            if ((*it).second==dataSize) {
                result.insert(pair<int,string>(1,to_string((uint16)response->completionCode)));
                break;    
            }
        }
        // provide a default value just in case.
        result.insert(pair<int,string>(1,"uint8"));
        
        // result 2 = sensor operational state (as string)
        // get the operational state
        if (opState.count(body[1])) {
            result.insert(pair<int,string>(2,opState.find(body[1])->second));
        } 
        else {
            result.insert(pair<int,string>(2,"unknown"));
        }

        // result 3 = sensor event message enable (as string)
        if (eventMsgEnable.count(body[2])) {
            result.insert(pair<int,string>(3,eventMsgEnable.find(body[2])->second));
        } 
        else {
            result.insert(pair<int,string>(3,"unknown"));
        }
        
        // result 4 = present state (as string)
        if (state.count(body[3])) {
            result.insert(pair<int,string>(4,state.find(body[3])->second));
        } 
        else {
            result.insert(pair<int,string>(4,"unknown"));
        }
        
        // result 5 = previous state (as string)
        if (state.count(body[4])) {
            result.insert(pair<int,string>(5,state.find(body[4])->second));
        } else {
            result.insert(pair<int,string>(5,"unknown"));
        }
        
        // result 6 = event state (as string)
        if (state.count(body[5])) {
            result.insert(pair<int,string>(6,state.find(body[5])->second));
        } else {
            result.insert(pair<int,string>(6,"unknown"));
        }
        
        // result 7 = present reading    
        // switch depending on data size
        switch (body[0]) { 
        case 0: // data size is uint8
            result.insert(pair<int,string>(7,to_string(((double)(*((uint8*)(&body[6]))))*resolution + offset)));
            break;
        case 1: // data size is sint8
            result.insert(pair<int,string>(7,to_string(((double)(*((sint8*)(&body[6]))))*resolution + offset)));
            break;
        case 2: // data size is uint16
            result.insert(pair<int,string>(7,to_string(((double)(*((uint16*)(&body[6]))))*resolution + offset)));
            break;
        case 3: // data size is sint16
            result.insert(pair<int,string>(7,to_string(((double)(*((sint16*)(&body[6]))))*resolution + offset)));
            break;
        case 4: // data size is uint32
            result.insert(pair<int,string>(7,to_string(((double)(*((uint32*)(&body[6]))))*resolution + offset)));
            break;
        case 5: // data size is sint32
            result.insert(pair<int,string>(7,to_string(((double)(*((sint32*)(&body[6]))))*resolution + offset)));
            break;
        default:
            result.insert(pair<int,string>(7,"0"));
            break;
        }
    }
    return result;
}

//*******************************************************************
// getStateSensorReadings()
//
// This function contains implementation of the getStateSensorReadings command
// specified by DMTF specification DSP0248_1.1.0
//
// parameters:
//    pdr - the PDR for the sensor
//    params - a map of the parameters for this function.  The values expected are:
//       0 = sensorRearm bitfield as a base10 integer.
// returns:
//    a map of response parameters.  The values are:
//       0 = completion code (as an enumerated value)
//       1 = composite sensor count (as an integer)
//       2 = operational state (as a string)
//       3 = present state (as a string)
//       4 = previous state (as a string)
//       5 = event state (as a string)
//       ..  more set requests and effecter states as needed for effecter count
map<int,string> clientNode::getStateSensorReadings(GenericPdr* pdr, map<int,string> &params, PdrRepository &repo) {
    static const map<int,string> opState = {
        {0,"enabled-updatePending"},{1,"enabled-noUpdatePending"},{2,"disabled"},
        {3,"unavailable"},{4,"statusUnknown"},{5,"failed"},{6,"initializing"},
        {7,"shuttingDown"},{8,"inTest"}};
    map<int,string> result;

    // check the right keys
    if ((!pdr->keyExists("sensorID"))||(!pdr->keyExists("stateSetID"))) {
        // this pdr is not for an state sensor, or it is missing fields
        result.insert(pair<int,string>(0,to_string(RESPONSE_INVALID_EFFECTER_ID)));
        return result;
    }

    // check the number of parameters
    if (params.size() !=1) {
        result.insert(pair<int,string>(0,to_string(RESPONSE_ERROR_INVALID_DATA)));
        return result;
    }
    if (!params.count(0)) {
        result.insert(pair<int,string>(0,to_string(RESPONSE_ERROR_INVALID_DATA)));
        return result;
    }

    // prepare the request
    unsigned char buffer[4];     
    unsigned int body_len = 4;
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_GET_STATE_SENSOR_READINGS;
    *((uint16*)buffer) = atoi(pdr->getValue("sensorID").c_str());
    buffer[2] = atoi(params.find(0)->second.c_str());
    buffer[3] = 0;

    // send the command
    putCommand(&header, buffer, body_len);

    // recieve the response
    PldmResponseHeader* response;
    response = (PldmResponseHeader*)getResponse();
    char* body = (char*)(response+1);

    // get the state set for this endpoint
    unsigned long setId = atol(pdr->getValue("stateSetID").c_str());
    map<unsigned int,string> states = repo.getStateSet(setId);

    // build the result structure from the response data 
    // result 0 - completion code
    result.insert(pair<int,string>(0,to_string((uint16)response->completionCode)));

    // result 1 - composite sensor count
    uint8 compositeSensorCount = body[0];
    result.insert(pair<int,string>(1,to_string((uint16)compositeSensorCount)));

    for (uint8 i = 0; i<compositeSensorCount; i++) {
        // get the operational state
        uint8 sensorOperationalState = body[1+i*4];
        // result 2 = sensor operational state (as string)
        // get the operational state
        if (sensorOperationalState) {
            result.insert(pair<int,string>(2+i*4,opState.find(sensorOperationalState)->second));
        } 
        else {
            result.insert(pair<int,string>(2+i*4,"unknown"));
        }
        
        uint8 presentState = body[2+i*4];
        if (states.count(presentState)) {
            result.insert(pair<int,string>(4+i*4,states[presentState]));
        } 
        else {
            result.insert(pair<int,string>(4+i*4,"unknown"));
        }
        uint8 previousState = body[3+i*4];
        if (states.count(previousState)) {
            result.insert(pair<int,string>(5+i*4,states[previousState]));
        } 
        else {
            result.insert(pair<int,string>(5+i*4,"unknown"));
        }
        uint8 eventState = body[4+i*4];
        if (states.count(eventState)) {
            result.insert(pair<int,string>(5+i*4,states[eventState]));
        } 
        else {
            result.insert(pair<int,string>(5+i*4,"unknown"));
        }
    }
    // return the result
    return result;
}
