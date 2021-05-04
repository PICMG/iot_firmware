//*******************************************************************
//    pldm_client.cpp
//
//    This file implements a simple pldm server device that responds to 
//    the pldm repository requests.
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

void putCommand(mctp_struct * mctp, PldmRequestHeader* hdr, unsigned char* command, unsigned int size) {
    mctp_transmitFrameStart(mctp,sizeof(PldmRequestHeader) + size +4);
    mctp_transmitFrameData(mctp,(unsigned char *)hdr,sizeof(PldmRequestHeader));
    mctp_transmitFrameData(mctp,command,size);
    mctp_transmitFrameEnd(mctp);
}

//*******************************************************************
// main()
//
// the main program entry point.
int main(int argc, char*argv[])
{
    if (argc!=2) {
        std::cerr<<"Wrong number of arguments."<<endl;
        std::cerr<<"   pldm_server uart_port";
        return -1;
    }
   
    // initialize the uart connection
    unsigned int uart_handle = uart_init(argv[1]);
    if (uart_handle<=0) {
        std::cerr<<"error establishing uart connection."<<endl;
        return -1;
    }

    // initialize the mctp connection
    mctp_struct mctp;
    mctp_init(uart_handle, &mctp);
    
    node node1;
    node1.init(&mctp);
    unsigned char buffer[256];
 
    // wait for requests and respond to them as they come in
    std::cout<<"waiting for request"<<std::endl;
    while (1) {
         if (mctp_isPacketAvailable(&mctp)==0){
            // update the fsm
            mctp_updateRxFSM(&mctp);
         } else {
             // here to process a packet -- this copies the entire mctp
             // rx buffer to the node's buffer for processing.
             // The additional buffer copy should be avoided in the
             // ucontroller implementation
            std::cout<<"request received"<<std::endl;
             node1.parseCommand();
            std::cout<<"response sent"<<std::endl;
         }
    }
}
