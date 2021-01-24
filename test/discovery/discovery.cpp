#include <iostream>
#include <string>
#include <list>
#include <dirent.h>
#include <chrono>
#include "uart.h"
#include "mctp.h"
#include "node.h"
#include "PdrRepository.h"
#include "PldmEntity.h"

using namespace std;

class Terminus {
    public:
        int           deviceHandle;
        mctp_struct   mctpContext;
        node          pldmEndpoint;
        PdrRepository localRepository;

        Terminus();
        ~Terminus();
};

Terminus::Terminus() : deviceHandle(0) {};
Terminus::~Terminus() {if (deviceHandle) uart_close(deviceHandle);}

void sendGetTidCommand(node &pldm_node) {
    // form the command
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_GET_TID;
        
    // send the command
    pldm_node.putCommand(&header, 0, 0);
}

void sendSetTidCommand(node &pldm_node, uint8 newTid) {
    // form the command
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_TID;
    
    // send the command
    pldm_node.putCommand(&header, &newTid, 1);
}

// wait up to 1 second for a response
int waitForResponse(mctp_struct *mctp, unsigned char **response) {
    chrono::high_resolution_clock::time_point tstart = chrono::high_resolution_clock::now();
    
    while (!mctp_isPacketAvailable(mctp)) {
        chrono::high_resolution_clock::time_point tnow = chrono::high_resolution_clock::now();
        double telapsed = chrono::duration_cast<chrono::milliseconds>( tnow - tstart ).count();;
        if (telapsed>1000) return 0;
        mctp_updateRxFSM(mctp);
    }
    *response = (unsigned char*)(mctp_getPacket(mctp));
    return 1;
}

//*******************************************************************
// main entry point.
// this function attempts to detect PICMG IoT devices attached to 
// a subset of serial ports (specified by the command line parameter).
//
// The process is as follows:
// I. For each port:
//    A. Initialize the uart connection
//    B. attempt a PLDM getTID command.  If the command completes
//        1. set the TID using the setTID command
//        2. remove any pdr records from the repository that are
//           associated with the TID.
//        3. read the PDR records into the local PDR.
// 
// once the ports are initialized, the discovery agent will periodically
// poll the ports to see if there have been any changes.  The process is
// the same as discovery.
int main(int argc, char *argv[]) {
    DIR *dir;
    struct dirent *entry;       
    list<string> device_list;   
    map<int,Terminus *> termini;       // a map of all known termini in the system
    PldmEntity entity;                 // root node of system entity map

    // check the number of arguments
    if (argc!=2) {
        cerr<<"Wrong number of arguments."<<endl;
        cerr<<"Usage: discovery devpath"<<endl;
        cerr<<"devpath is the path to the devices to check along with the beginning"<<endl;
        cerr<<"part of the device name to match."<<endl;
        cerr<<"Examples:"<<endl;
        cerr<<"    /dev/ttyUSB will match all devices that start with ttyUSB"<<endl;
        cerr<<"    /dev/       will match all devices in the dev folder"<<endl;
        return -1;
    }

    // separate the partial file name from the folder
    string path = argv[1];
    string filename;
    size_t slashpos = path.find_last_of("/"); 
    if ( slashpos == string::npos) {
        // here if the string does not contain a forward slash - assume the
        // argument is a partial filename only
        filename = argv[1];
        path = ".";
    } else {
        filename = path.substr(slashpos+1,string::npos);
        path = path.substr(0,slashpos);
    }

    // attempt to open the directory
    dir = opendir(path.c_str()); 
    if (!dir) {
        cerr<<"error opening directory."<<endl;
        return -1;
    }

    // loop for each entry in the directory
    while ((entry = readdir(dir)) != NULL) {
        // check to see if the port name matches
        if (filename.compare(0,string::npos,entry->d_name,filename.length()) == 0) {
            // Here if the port name matches - add it to the list of known ports
            string portname = path;
            portname += "/";
            portname += entry->d_name;
            device_list.push_back(portname);
        } 
    }

    // close the directory
    closedir(dir); 

    // for each element in the list, attempt to communicate with the endpoint    
    for (list<string>::iterator it = device_list.begin(); it!= device_list.end(); it++) {
        cout<<"checking: "<<it->c_str()<<endl;
        int file_handle = uart_init(it->c_str());
        if (file_handle) {            
            Terminus *terminus = new Terminus;
            terminus->deviceHandle = file_handle;
            
            // here if the uart was opened - initialize the mctp communications layer
            mctp_init(file_handle, &(terminus->mctpContext));

            // initialize the pldm interface for this node
            terminus->pldmEndpoint.init(&(terminus->mctpContext));

            // see request the current TID for the node
            PldmResponseHeader *response;
            sendGetTidCommand(terminus->pldmEndpoint);
            if (!waitForResponse(&(terminus->mctpContext), (unsigned char **)(&response))) {
                // no response - continue to next port.  Assume this is not a
                // pldm-connected device
                delete terminus;
                continue;
            }

            // a response was received - has the TID already been assigned?
            uint8 tid = *((uint8*)(response+1));
            if (tid == 0) {
                // tid not yet assigned - assign it
                sendSetTidCommand(terminus->pldmEndpoint, terminus->deviceHandle);

                if (!waitForResponse(&(terminus->mctpContext), (unsigned char **)(&response))) {
                    // unable to set the TID - this should never happen
                    // continue with the next port.
                    delete terminus;
                    continue;
                }

                // remove any pdrs associated with this TID                
            }

            // read the PDRs for this node into the repository, fixing the TID and
            // TerminusHandle values as they are read
            cout<<"Found node at: "<<it->c_str()<<endl;
            terminus->localRepository.setDictionary("pldm_definitions.json");
            terminus->localRepository.addPdrsFromNode(terminus->pldmEndpoint);
            entity.attach(terminus->localRepository, terminus->deviceHandle);
            entity.link(terminus->localRepository, terminus->deviceHandle);
            termini.insert(pair<int,Terminus *>(terminus->deviceHandle,terminus));
        }       
    }
    
    // close all uart connections
    entity.dump();

    // delete memory allocated in the terminus map
    while (termini.size()) {
        Terminus * t = termini.begin()->second;
        termini.erase(termini.begin());
        delete t;
    }
    return 0;
}