//*******************************************************************
//    pldm_test.cpp
//
//    This file implements code that tests the pldm repository and 
//    generic PDR classes.  It creates a simulated pldm connector 
//    node that acts like a remote device.  This code queries the 
//    connector node to get the PDR information, then displays the 
//    information to the standard output device.
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
#include "pldm.h"
#include "PdrRepository.h"
#include "node.h"

#define MAX_BYTES_PER_PDR_XFER 40


//*******************************************************************
// main()
//
// the main program entry point.
int main(int argc, char*argv[])
{
    if (argc!=2) {
        std::cerr<<"Wrong number of arguments."<<endl;
        return -1;
    }
    PdrRepository pdrRepository;
    if (!pdrRepository.setDictionary(argv[1])) return -1;
    
    node node1;
    unsigned char buffer[256];
 
    // get the repository information
    PldmRequestHeader hdr;
    hdr.flags1 = 0; hdr.flags2 = 0; hdr.command = CMD_GET_PDR_REPOSITORY_INFO;
    node1.putCommand(&hdr, buffer, 0);
    unsigned char *response = node1.getResponse();
    PldmResponseHeader* rxHeader = (PldmResponseHeader*)response;
    GetPdrRepositoryInfoResponse* infoResponse = (GetPdrRepositoryInfoResponse*)(response + sizeof(PldmResponseHeader));
    if (infoResponse->completionCode != RESPONSE_SUCCESS) {
        std::cout << "Error Getting PDR Info" << std::endl;
        return -1;
    }
    std::cout << "PDR Info:" << std::endl;
    std::cout << "   Records       " << infoResponse->recordCount << std::endl;
    std::cout << "   MaxRecordSize " << infoResponse->largestRecordSize << std::endl;
    std::cout << "   TotalSize     " << infoResponse->repositorySize << std::endl;
    std::cout << std::endl;

    // loop to read all the PDR records from the node and build a local PDR repository
    pdrRepository.addPdrsFromNode(node1);

    pdrRepository.dump();
}
