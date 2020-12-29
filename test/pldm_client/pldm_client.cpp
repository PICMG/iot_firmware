//*******************************************************************
//    pldm_client.cpp
//
//    This file implements a simple pldm client device that reads 
//    the pldm repository of a PLDM device connected over a 
//    specified serial port..
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
#include <iostream>
#include <fstream>

#include <uchar.h>
#include <map>
#include "uart.h"
#include "mctp.h"
#include "pldm.h"
#include "node.h"
#include "PdrRepository.h"

#define MAX_BYTES_PER_PDR_XFER 40

//void putCommand(mctp_struct * mctp, PldmRequestHeader* hdr, unsigned char* command, unsigned int size) {
//    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + size +4);
//    mctp_transmitFrameData(mctp,(unsigned char *)hdr,sizeof(PldmRequestHeader));
//    mctp_transmitFrameData(mctp,command,size);
//    mctp_transmitFrameEnd(mctp);
//}

//*******************************************************************
// main()
//
// the main program entry point.
int main(int argc, char*argv[])
{
    unsigned char buffer[256];
    node node1;

    if (argc!=3) {
        std::cerr<<"Wrong number of arguments."<<endl;
        std::cerr<<"   pldm_client dictionary_file uart_port";
        return -1;
    }

    // load the dictionary from the specified file.  The dictionary
    // includes meta-data that helps this program interpret the 
    // information it receives from the server node.
    PdrRepository pdrRepository;
    if (!pdrRepository.setDictionary(argv[1])) return -1;
    
    // initialize the uart connection
    unsigned int uart_handle = uart_init(argv[2]);
    if (uart_handle<=0) {
        std::cerr<<"error establishing uart connection."<<endl;
        return -1;
    }

    // initialize the mctp connection
    mctp_struct mctp;
    mctp_init(uart_handle, &mctp);
    node1.init(&mctp);

    // get the repository information by sending the request through
    // the mctp interface
    PldmRequestHeader hdr;
    hdr.flags1 = 0; hdr.flags2 = 0; hdr.command = CMD_GET_PDR_REPOSITORY_INFO;
    node1.putCommand(&hdr, buffer, 0);

    // wait for the packet to be available, processing bytes as they come in
    while(mctp_isPacketAvailable(&mctp)==0){
        mctp_updateRxFSM(&mctp);
    }

    unsigned char *response = mctp_getPacket(&mctp);
    PldmResponseHeader* rxHeader = (PldmResponseHeader*)response;
    GetPdrRepositoryInfoResponse* infoResponse = (GetPdrRepositoryInfoResponse*)(response + sizeof(PldmResponseHeader)-1);
    if (infoResponse->completionCode != RESPONSE_SUCCESS) {
        std::cout << "Error Getting PDR Info" << std::endl;
        return -1;
    }
    std::cout << "PDR Info:" << std::endl;
    std::cout << "   Records       " << infoResponse->recordCount << std::endl;
    std::cout << "   MaxRecordSize " << infoResponse->largestRecordSize << std::endl;
    std::cout << "   TotalSize     " << infoResponse->repositorySize << std::endl;
    std::cout << std::endl;

    pdrRepository.addPdrsFromNode(node1);

    pdrRepository.dump();
}
