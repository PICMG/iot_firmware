//*******************************************************************
//    PdrRepository.cpp
//
//    This file provides implementation of a PDR repositiory class that is
//    intended to be used as part of the PICMG pldm library reference
//    code. This class contains a collection of GenericPdr ojects and 
//    a dictionary Json object that can be used to decode the bytes within 
//    the PDRs.
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
#include "JsonFactory.h"
#include "JsonAbstractValue.h"
#include "JsonArray.h"
#include "JsonObject.h"
#include "JsonValue.h"
#include "PdrRepository.h"
#include "GenericPdr.h"

#define MAX_BYTES_PER_PDR_XFER 40

//*******************************************************************
// PdrRepository()
// 
// Constructor for PDR Repository.
PdrRepository::PdrRepository() :dictionary(NULL) {

}

//*******************************************************************
// PdrRepository()
// 
// Destructor for PDR Repository.  This function frees any memory
// allocated to the object
PdrRepository::~PdrRepository() {
	// free memory for each element in the map
	for (map<uint32,GenericPdr*>::iterator it = repository.begin(); it != repository.end(); ++it) {
		delete it->second;
		it->second = 0;
	}
}

//*******************************************************************
// findPdrTemplateFromId()
//
// Given the id number of a PDR type, this function searches the 
// dictionary of known PDRs and returns a Json object that describes
// the PDR with the specified type ID.
//
// parameters:
//    id - the pdr type (id) to find in the dictionary
// returns:
//    a pointer to a JsonObject that describes the PDR with the
//    specified type.  If a matching PDR template cannot be found, 
//    NULL is returned.
JsonObject* PdrRepository::findPdrTemplateFromId(unsigned long id) {
    if (!dictionary) return NULL;

    JsonAbstractValue* av;
    JsonArray& pdrs = *((JsonArray*)dictionary->find("pdr_defs"));

    // loop for each tempate in the dictionary
    for (int i = 0;i < pdrs.size();i++) {
        // get the template
        av = pdrs.getElement(i);
        if ((!av) || (typeid(*av) != typeid(JsonObject))) continue;
        JsonObject& tmplt = *((JsonObject*)av);

        // find the value of the PDRType field
        av = tmplt.find("PDRType");
        if ((!av) || (typeid(*av) != typeid(JsonObject))) continue;
        JsonObject& pdrtype = *((JsonObject*)av);

        // find the enumerated values for this field - there should only be one choice
        av = pdrtype.find("values");
        if ((!av) || (typeid(*av) != typeid(JsonArray))) continue;
        JsonArray& values = *((JsonArray*)av);

        // for the first enumerated value, check the ID
        av = values.getElement(0);
        if ((!av) || (typeid(*av) != typeid(JsonObject))) continue;
        unsigned long template_id = av->getInteger("value");
        if (template_id == id) return &tmplt;
    }
    return NULL;
}

//*******************************************************************
// loadJsonFile()
//
// Given the filename of a Json File, load the dictionary from the 
// file.
//
// parameters:
//    filename - the name of the json file to load
// returns:
//    a pointer to json structure that was loaded, otherwise NULL
JsonAbstractValue* PdrRepository::loadJsonFile(const char* filename) {
    JsonFactory jf;

    ifstream jsonfile(filename, ifstream::binary);
    if (!jsonfile.is_open()) {
        cerr << "error opening file" << endl;
        return NULL;
    }

    // get the length of the file 
    jsonfile.seekg(0, jsonfile.end);
    streamoff length = jsonfile.tellg();
    jsonfile.seekg(0, jsonfile.beg);

    // create a buffer that is large enough to hold the file
    char* buffer = new char[(unsigned int)length + 1];

    // read the file into the buffer
    jsonfile.read(buffer, length);

    // add the terminating null character
    buffer[length] = 0;

    // construct the json objects from the file structure
    JsonAbstractValue* json = jf.build(buffer);

    delete[] buffer;
    return json;
}

//*******************************************************************
// setDictionary()
//
// Set the dictionary for the repository from the specified file.
//
// parameters:
//    filename - the name of the json file that hold the dictionary.
// returns:
//    true of dictionary is successfully loaded, otherwise false
bool PdrRepository::setDictionary(string filename) {
    // load the pdr template file
    JsonAbstractValue* tmplt_defs = loadJsonFile(filename.c_str());
    if ((!tmplt_defs) || (typeid(*tmplt_defs) != typeid(JsonObject))) {
        cerr << "Unable to open PLDM template Json file" << endl;
        return false;
    }

    // find the PDR description array within the file
    JsonAbstractValue* av;
    av = ((JsonObject*)tmplt_defs)->find("pdr_defs");
    if ((!av) || (typeid(*av) != typeid(JsonArray))) {
        cerr << "Couldn't locate PDR templates in PLDM template Json file" << endl;
        return false;
    }
    dictionary = (JsonObject*)tmplt_defs;
    return true;
}

//*******************************************************************
// getPdrPart()
//
// this function sends a GetPDR pldm command through the specified 
// connector node in order to receive a portion of a PDR. 
//
// parameters:
//    node - the connector node to communicate with.
//    recordHandle - the record handle from the previous call, or 0
//       for the start of a new record.
//    nextRecordHandle - this parameter will be updated with the value
//       to use for recordHandle on the next call of this function.
// returns:
//    the response code received from the communication with the
//    connector node.
// changes:
//    If a new PDR is completely received, it is added to the repository
int PdrRepository::getPdrPart(PldmNode& node1, uint32 recordHandle, uint32 & nextRecordHandle) {
    // read part of the pdr from the device - 
    // returns -1 on error, 0 if not complete, 1 if the transfer is complete
    static uint32 lastRecordHandle = 0xffff;
    static uint32 lastDataHandle = 0x0000;
    static unsigned char* localRecord = 0;
    static unsigned int * pdrCounter;
    static GenericPdr* partialPdrPtr = 0;
    PldmRequestHeader hdr;
    GetPdrCommand cmd;

    // create the command
    hdr.flags1 = 0; hdr.flags2 = 0; hdr.command = CMD_GET_PDR;
    cmd.dataTransferHandle = lastDataHandle;
    cmd.recordChangeNumber = 0;
    cmd.recordHandle = recordHandle;
    cmd.transferOperationFlag = (lastRecordHandle == recordHandle) ? 0 : 1;
    cmd.requestCount = MAX_BYTES_PER_PDR_XFER;

    // request the next part of the PDR
    node1.putCommand(&hdr, (unsigned char*)(&cmd), sizeof(cmd));

    // await the response
    unsigned char* response = node1.getResponse();
    PldmResponseHeader* rxHeader = (PldmResponseHeader*)response;
    GetPdrResponse* getPdrResponse = (GetPdrResponse*)(response + sizeof(PldmResponseHeader)-1);

    if (getPdrResponse->completionCode != RESPONSE_SUCCESS) {
        lastRecordHandle = 0xffff;
        lastDataHandle = 0x0000;
        return -1;
    }

    // here if the response was successfull - assemble the result
    // setup pointers if this is the first packet
    if (lastRecordHandle == 0xFFFF) {
        if (partialPdrPtr) {
            delete partialPdrPtr;
            partialPdrPtr = 0;
        }
        PdrCommonHeader* header = (PdrCommonHeader*)((unsigned char*)getPdrResponse + sizeof(GetPdrResponse));
        
        // this is the start of the packet
        partialPdrPtr = new GenericPdr();
    }

    // copy the data into the result
    partialPdrPtr->appendRawData(
        (unsigned char*)((unsigned char*)getPdrResponse) + sizeof(GetPdrResponse),
        getPdrResponse->responseCount);


    ((GenericPdr*)partialPdrPtr)->setDictionary(findPdrTemplateFromId(partialPdrPtr->getPdrType()));

    // update state based on completion code
    switch (getPdrResponse->transferFlag) {
    case 0: // start
        lastRecordHandle = recordHandle;
        lastDataHandle = getPdrResponse->nextDataTransferHandle;
        break;
    case 1: // middle
        lastDataHandle = getPdrResponse->nextDataTransferHandle;
        break;
    case 4: // end
        lastRecordHandle = 0xffff;
        lastDataHandle = 0x0000;
        nextRecordHandle = getPdrResponse->nextRecordHandle;

        // TODO, check the CRC
        
        // add the pdr to the map;
        repository[recordHandle] = partialPdrPtr;
        partialPdrPtr = 0;
        return 1;
        break;
    case 5: // start and end
        lastRecordHandle = 0xffff;
        lastDataHandle = 0x0000;
        nextRecordHandle = getPdrResponse->nextRecordHandle;

        // add the pdr to the map;
        repository[recordHandle] = partialPdrPtr;
        partialPdrPtr = 0;
        return 1;
    default:
        return -1;
    }
    return 0;
}

//*******************************************************************
// getStateSet()
//
// returns a map representation of a specified state set.  The stateSetId
// parameter should match either a state set found in DSP0249, or the 
// vendorStateSetHandle found pdr records.  The vendorID (if specified)
// should match the OEM vendor id from the PDR record.  If the vendor
// ID is not specified, the Vendor ID for DMTF is assumed.
//
// parameters:
//    stateSetID - the Id of the state set to find
//    vendorID - the vendor ID of the state set originator
// returns:
//    a map of the states in the state set, if found, otherwise an
//    empty map.
map<unsigned int,string> PdrRepository::getStateSet(uint32 stateSetId) {
    map<unsigned int,string> result;

    // find the state set
    JsonArray *setArray = ((JsonArray*)(dictionary->find("state_defs")));
    if (!setArray) return result;

    for (unsigned int i = 0; i< setArray->size(); i++) {
        JsonObject *stateSet = ((JsonObject*)(setArray->getElement(i)));
        if (!stateSet) continue;

        if (stateSetId != stateSet->getInteger("stateSetId")) continue;

        // here if the state set has been found - populate the map
        JsonArray *states = ((JsonArray*)(stateSet->find("states")));
        if (!states) continue;
        for (int j=0;j<states->size();j++) {
            JsonObject *state = ((JsonObject*)(states->getElement(j)));
            if (!state) continue;
            result.insert(pair<unsigned int,string>(state->getInteger("value"),state->getValue("name")));
        }
    }
    return result;
}

//*******************************************************************
// getEntityTypeString()
//
// returns string representation of the entity type.  For standard
// entity types defined in DSP0249, the defined string is returned.
// for OEM entity types, the entity name defined in the OEMEntityId PDR
// is returned.
//
// parameters:
//    entityType - the type of the string to find
// returns:
//    the string entity name
string PdrRepository::getEntityTypeString(uint16 entityType) {
    // handle different ranges differently
    if ((entityType>=8192)&&(entityType<=16383)) {
        return "Chassis-specific";
    }
    if ((entityType>=16383)&&(entityType<=24575)) {
        return "Board-set specific";
    }

    // find the entity types set
    JsonArray *entityNameArray = ((JsonArray*)(dictionary->find("entity_defs")));
    if (!entityNameArray) return "Unknown";

    for (unsigned int i = 0; i< entityNameArray->size(); i++) {
        JsonObject *entity = ((JsonObject*)(entityNameArray->getElement(i)));
        if (!entity) continue;
        if (entity->getInteger("value")==entityType) {
            return entity->getValue("name");
        }
    }
    return "Unknown";
}

//*******************************************************************
// addPdrsFromNode()
//
// this function successively sends GetPDR pldm commands through the 
// specified connector node in order to receive all the PDRs in the
// connected devices PDR repository.  When done, this PdrRepository
// will be populated with copies of the PDRs received from the connector
// node.
//
// parameters:
//    node - the connector node to communicate with.
// returns:
//    true if no errors, otherwise false.
// changes:
//    The pdr repository will contain copies of the PDRs received from
//    the connector node.
bool PdrRepository::addPdrsFromNode(PldmNode& node1) {
    // loop to read all the PDR records from the node and build a local PDR repository
    uint32 recordHandle = 0;
    uint32 nextRecordHandle = 0;

    while (true) {
        int result = 0;
        while (result == 0) {
            result = getPdrPart(node1, recordHandle, nextRecordHandle);
        }
        if (result < 0) return false;
        recordHandle = nextRecordHandle;
        if (nextRecordHandle==0) break;
    }

    // search through the PDRs and find any OEM state records. 
    // if a state record is found, add the state record to our dictionary
    // of states
    for (map<uint32,GenericPdr*>::iterator it = repository.begin();it!=repository.end();it++) {
        GenericPdr *pdr = it->second;
        if (pdr->getPdrType()==PDR_TYPE_OEM_STATE_SET) {
            // this is an oem state set - check for required keys
            if (!pdr->keyExists("OEMStateSetIDHandle")) continue;
            if (!pdr->keyExists("vendorIANA")) continue;
            if (!pdr->keyExists("stateCount")) continue;
            
            // create the new state set
            JsonObject *stateSet = new JsonObject();
            JsonValue * name       = new JsonValue("OEM State Set"); 
            stateSet->put("name",name);
            JsonValue * vendorId   = new JsonValue(pdr->getValue("vendorIANA")); 
            stateSet->put("vendorId",vendorId);
            JsonValue * stateSetId = new JsonValue(pdr->getValue("OEMStateSetIDHandle")); 
            stateSet->put("stateSetId", stateSetId);
            JsonArray * states     = new JsonArray();
            for (unsigned int i=0; i< atoi(pdr->getValue("stateCount").c_str()); i++) {
                // construct the names of the fields to search for
                string minStateValue = "minStateValue[";
                minStateValue.append(to_string(i+1));
                minStateValue.append("]");
                string stateName = "stateName[";
                stateName.append(to_string(i+1));
                stateName.append("][1]");

                // check to see if the keys exist
                if (!pdr->keyExists(stateName)) continue;
                if (!pdr->keyExists(minStateValue)) continue;

                // create the state object
                JsonObject *state = new JsonObject;
                
                // create the fields
                JsonValue * name  = new JsonValue(pdr->getValue(stateName)); 
                state->put("name",name);
                JsonValue * value = new JsonValue(pdr->getValue(minStateValue)); 
                state->put("value", value);
                
                // add the new state to the object
                states->add(state);
            }
            stateSet->put("states",states);
            JsonArray *setArray = ((JsonArray*)(dictionary->find("state_defs")));
            if (setArray) {
                setArray->add(stateSet);
            }
        }
    }

    // search through the PDRs and find any OEMEntityID records. 
    for (map<uint32,GenericPdr*>::iterator it = repository.begin();it!=repository.end();it++) {
        GenericPdr *pdr = it->second;
        if (pdr->getPdrType()==PDR_TYPE_OEM_ENTITY_ID) {
            // this is an oem entity id - check for required keys
            if (!pdr->keyExists("OEMEntityIDHandle")) continue;
            if (!pdr->keyExists("entityIDName[1]")) continue;
            
            // create the entity object
            JsonObject *entity = new JsonObject;
                
            // create the fields
            JsonValue * name  = new JsonValue(pdr->getValue("entityIDName[1]")); 
            entity->put("name",name);
            JsonValue * value = new JsonValue(pdr->getValue("OEMEntityIDHandle")); 
            entity->put("value", value);
                
            JsonArray *entityArray = ((JsonArray*)(dictionary->find("entity_defs")));
            if (entityArray) {
                entityArray->add(entity);
            }
        }
    }

    return true;
}

//*******************************************************************
// getPdrFromRecordHandle()
//
// return a pointer to a specific PDR if it exists in the repository,
// otherwise, return null pointer.
//
// parameters:
//    recordNumber - the number of the record handle to find.
// returns:
//    void
GenericPdr * PdrRepository::getPdrFromRecordHandle(uint32 recordNumber) {
    for (map<uint32, GenericPdr*>::iterator it = repository.begin(); it != repository.end(); ++it) {
        GenericPdr * pdr = it->second;
        if (pdr->keyExists("recordHandle")) {
            if (atol(pdr->getValue("recordHandle").c_str())==recordNumber) return pdr;
        }
    }
    return 0;
}

//*******************************************************************
// dump()
//
// dump the contents of this repository to the standard output device.
//
// parameters:
//    none.
// returns:
//    void
void PdrRepository::dump() {
    for (map<uint32, GenericPdr*>::iterator it = repository.begin(); it != repository.end(); ++it) {
        it->second->dump();
        std::cout << std::endl;
    }
}
