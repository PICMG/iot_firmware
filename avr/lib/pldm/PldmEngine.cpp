//*******************************************************************
//    PldmEngine.cpp
//
//    This file implements a client-side pldm engine that manages
//    a collection of endpoints and allows interaction with the 
//    endpoints through sending commands and receiving responses.
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
#include <string>
#include <map>
#include <list>
#include <dirent.h>
#include <chrono>
#include "PldmEngine.h"

//*******************************************************************
// Constructor
//
PldmEngine::PldmEngine() {

}

//*******************************************************************
// Destructor
//
PldmEngine::~PldmEngine() {
    // delete memory allocated in the terminus map
    // This will also close any open ports.
    while (termini.size()) {
        Terminus * t = termini.begin()->second;
        termini.erase(termini.begin());
        delete t;
    }
}

//********************************************************************
// sendGetTidCommand
//  
// send the GetTid command to the PLDM endpoint.  This command is
// used as part of the discovery process to determine if a device is
// present on a given port.
// NOTE: the function that sends this command should await a response
//   from the endpoint.
//
// NOTE: the function that sends this command should await a response
//   from the endpoint.
//
// parameters:
//   pldm_node - a reference to a pldm node to send the command to
// returns:
//   nothing
void PldmEngine::sendGetTidCommand(clientNode &pldm_node) {
    // form the command
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_GET_TID;
        
    // send the command
    pldm_node.putCommand(&header, 0, 0);
}

//********************************************************************
// sendSetTidCommand
//  
// send the GetTid command to the PLDM endpoint.  This command is
// used as part of the discovery process to determine if a device is
// present on a given port.
//
// NOTE: the function that sends this command should await a response
//   from the endpoint.
//
// parameters:
//   pldm_node - a reference to a pldm node to send the command to
//   newTid - the new terminus Id for the node
// returns:
//   nothing
void PldmEngine::sendSetTidCommand(clientNode &pldm_node, uint8 newTid) {
    // form the command
    PldmRequestHeader header;
    header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_TID;
    
    // send the command
    pldm_node.putCommand(&header, &newTid, 1);
}

//*******************************************************************
// waitForResponse()
//
// wait up to 1 second for a response from the specified mctp stack.
// This function is used in conjunction with the discovery process 
// in order to determine if a node is running a PLDM stack.
//
// parameters:
//   mctp - a pointer to an initialized mctp structure
//   response - on success, this pointer will be updated to point 
//      to the response that was received
// returns:
//   1 on success, otherwise 0
int PldmEngine::waitForResponse(mctp_struct *mctp, unsigned char **response) {
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
// sendPldmCommand()
//
// send a PldmCommand to the pldm endpoint specified by the address
// paramter.  Parameters to send with the command are stored in the 
// map<int,string> argument of this function.  Return values from the 
// function are placed in the list<int,string> return value with each
// return value represented in a seperate string.
//
// parameters:
//    command - the PLDM command to send
//    address - the URI of the PLDM endpoint to send the command to
//    parameters - the input parameters to the pldm command
// returns:
//    a list of the fields returned by the pldm response with the
//    first entry as the response code.
map<int, string> PldmEngine::sendPldmCommand(string command, string address, map<int, string> &parameters)
{
    map<int,string> result;
    unsigned int tid;
    GenericPdr *pdr;
    static map<string, uint8> commands = {
        {"SET_EVENT_RECEIVER",              0x04}, // SetEventReceiver
        {"GET_EVENT_RECEIVER",              0x05}, // GetEventReceiver
        {"SET_NUMERIC_SENSOR_ENABLE",       0x10}, // SetNumericSensorEnable
        {"GET_SENSOR_READING",              0x11}, // GetSensorReading
        {"GET_SENSOR_THRESHOLDcdS",           0x12}, // GetSensorThresholds
        {"SET_SENSOR_THRESHOLDS",           0x13}, // SetSensorThresholds
        {"RESTORE_SENSOR_THRESHOLDS",       0x14}, // RestoreSensorThresholds
        {"GET_SENSOR_HYSTERESIS",           0x15}, // GetSensorHysteresis
        {"SET_SENSOR_HYSTERESIS",           0x16}, // SetSensorHysteresis
        {"SET_STATE_SENSOR_ENABLES",        0x20}, // SetStateSensorEnables
        {"GET_STATE_SENSOR_READINGS",       0x21}, // GetStateSensorReadings
        {"SET_NUMERIC_EFFECTER_ENABLE",     0x30}, // SetNumericEffecterEnable
        {"SET_NUMERIC_EFFECTER_VALUE",      0x31}, // SetNumericEffecterValue
        {"GET_NUMERIC_EFFECTER_VALUE",      0x32}, // GetNumericEffecterValue
        {"SET_STATE_EFFECTER_ENABLES",      0x38}, // SetStateEffecterEnables
        {"SET_STATE_EFFECTER_STATES",       0x39}, // SetStateEffecterStates
        {"GET_STATE_EFFECTER_STATES",       0x3A}, // GetStateEffecterStates
    };
    
    // if the command is not supported, return "unspported command" error
    if (!commands.count(command)) {
        result.insert(pair<int,string>(0,to_string(RESPONSE_ERROR_UNSUPPORTED_PLDM_CMD)));
        return result;
    }

    // see if the endpoint exists
    if (!pldmRoot.getPdrFromURI(address,pdr,tid)) {
        result.insert(pair<int,string>(0,to_string(RESPONSE_INVALID_SENSOR_ID)));
        return result;
    }

    // find the terminus associated with the endpoint
    if (termini.count(tid)==0) {
        // this error would be the natural response to an invalid terminus
        result.insert(pair<int,string>(0,to_string(RESPONSE_TRANSFER_TIMEOUT)));
        return result;
    }
    Terminus *terminus = termini.find(tid)->second;

    // here if the command is known and the uri exists
    switch (commands.find(command)->second) {
//    case CMD_SET_EVENT_RECEIVER:
//        break;
//    case CMD_GET_EVENT_RECEIVER:
//        break;
//    case CMD_POLL_FOR_PLATFORM_EVENT_MESSAGE:
//        break;
//    case CMD_SET_NUMERIC_SENSOR_ENABLE:
//        break;
    case CMD_GET_SENSOR_READING:
        // call the endpoint to get he reading
        return terminus->pldmEndpoint.getSensorReading(pdr, parameters);            
//    case CMD_GET_SENSOR_THRESHOLDS:
//        break;
//    case CMD_SET_SENSOR_THRESHOLDS:
//        break;
//    case CMD_RESTORE_SENSOR_THRESHOLDS:
//        break;
//    case CMD_GET_SENSOR_HYSTERESIS:
//        break;
//    case CMD_SET_SENSOR_HYSTERESIS:
//        break;
//    case CMD_SET_STATE_SENSOR_ENABLES:
//        break;
    case CMD_GET_STATE_SENSOR_READINGS:
        return terminus->pldmEndpoint.getStateSensorReadings(pdr, parameters, terminus->localRepository);
//    case CMD_SET_NUMERIC_EFFECTER_ENABLE:
//        return terminus->pldmEndpoint.setNumericEffecterEnable(pdr, parameters);
    case CMD_SET_NUMERIC_EFFECTER_VALUE:
        return terminus->pldmEndpoint.setNumericEffecterValue(pdr, parameters);
    case CMD_GET_NUMERIC_EFFECTER_VALUE:
        return terminus->pldmEndpoint.getNumericEffecterValue(pdr, parameters);
//    case CMD_SET_STATE_EFFECTER_ENABLES:
//        return terminus->pldmEndpoint.setStateEffecterEnables(pdr, parameters);
    case CMD_SET_STATE_EFFECTER_STATES:
        return terminus->pldmEndpoint.setStateEffecterStates(pdr, parameters, terminus->localRepository);
//    case CMD_GET_STATE_EFFECTER_STATES:
//        break;
    }
    return result;
}

//*******************************************************************
// init()
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
//
// parameters:
//   porttype - the partial porttype to add to the repository. The
//      function searches for any ports that begin with the specified
//      porttype and end with a number.  For instance, port type
//      /dev/ttyUSB, will find devices connected to ttyUSB0, ttyUSB1, etc.
//
// returns:
//   true if no errors, otherwise false;
bool PldmEngine::init(string porttype) 
{
    DIR *dir;
    struct dirent *entry;       
    list<string> device_list;   

    // separate the partial file name from the folder
    string path = porttype;
    string filename;
    size_t slashpos = path.find_last_of("/"); 
    if ( slashpos == string::npos) {
        // here if the string does not contain a forward slash - assume the
        // argument is a partial filename only
        filename = porttype;
        path = ".";
    } 
    else {
        filename = path.substr(slashpos+1,string::npos);
        path = path.substr(0,slashpos);
    }

    // attempt to open the directory
    dir = opendir(path.c_str()); 
    if (!dir) {
        cerr<<"error opening directory."<<endl;
        false;
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
            pldmRoot.attach(terminus->localRepository, terminus->deviceHandle);
            pldmRoot.link(terminus->localRepository, terminus->deviceHandle);
            termini.insert(pair<int,Terminus *>(terminus->deviceHandle,terminus));
        }       
    }
    
    return true;
}

//*******************************************************************
// showMap()
//
// output a map of the device endpoints to the specified output stream
//
void PldmEngine::showMap(ostream &out) {
    pldmRoot.dump(out);
}