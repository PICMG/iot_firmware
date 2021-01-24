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
// node()
//
// default constructor
node::node() {
    nextByte = 0;
}

void node::init(mctp_struct *mctp) {
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
void node::putCommand(PldmRequestHeader* hdr, unsigned char* command, unsigned int size) {
    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + size +4);
    mctp_transmitFrameData(mctp,(unsigned char *)hdr,sizeof(PldmRequestHeader));
    mctp_transmitFrameData(mctp,command,size);
    mctp_transmitFrameEnd(mctp);
}

unsigned char* node::getResponse(void) {
    while (!mctp_isPacketAvailable(mctp)) {
        mctp_updateRxFSM(mctp);
    }
    return mctp_getPacket(mctp);
}