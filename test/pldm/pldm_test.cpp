#include <iostream>
#include <fstream>

#include <uchar.h>
#include <map>
#include "pldm.h"
#include "PdrRepository.h"
#include "node.h"

#define MAX_BYTES_PER_PDR_XFER 40


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
