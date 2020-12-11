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
JsonObject* PdrRepository::findPdrTemplateFromId(unsigned long id) {
    if (!dictionary) return NULL;

    JsonAbstractValue* av;
    JsonArray& pdrs = *((JsonArray*)dictionary->find("pdr_defs"));

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

JsonAbstractValue* loadJsonFile(const char* filename) {
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

PdrRepository::PdrRepository() :dictionary(NULL) {}

PdrRepository::~PdrRepository() {
	// free memory for each element in the map
	for (map<uint32,GenericPdr*>::iterator it = repository.begin(); it != repository.end(); ++it) {
		delete it->second;
		it->second = 0;
	}
	// base class destructor will be automatically called to destruct the map
}

bool PdrRepository::setDictionary(string filename) {
    // load the pdr template file
    JsonAbstractValue* tmplt_defs = loadJsonFile("..\\..\\PdrMaker\\PdrMaker\\pldm_definitions.json");
    if ((!tmplt_defs) || (typeid(*tmplt_defs) != typeid(JsonObject))) {
        cerr << "Unable to open PLDM template Json file" << endl;
        return 0;
    }

    // find the PDR description array within the file
    JsonAbstractValue* av;
    av = ((JsonObject*)tmplt_defs)->find("pdr_defs");
    if ((!av) || (typeid(*av) != typeid(JsonArray))) {
        cerr << "Couldn't locate PDR templates in PLDM template Json file" << endl;
        return 0;
    }
    dictionary = (JsonObject*)tmplt_defs;
}

int PdrRepository::getPdrPart(node& node1, uint32 recordHandle, uint32 & nextRecordHandle) {
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
    GetPdrResponse* getPdrResponse = (GetPdrResponse*)(response + sizeof(PldmResponseHeader));

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
/*
        switch (header->PDRType) {
        case PDR_TYPE_NUMERIC_EFFECTER:
            partialPdrPtr = new NumericEffecterPDR();
            break;
        case PDR_TYPE_STATE_EFFECTER:
            partialPdrPtr = new StateEffecterPDR();
            break;
        case PDR_TYPE_NUMERIC_SENSOR:
            partialPdrPtr = new GenericPdr();
            break;
        case PDR_TYPE_STATE_SENSOR:
            partialPdrPtr = new StateSensorPDR();
            break;
        case PDR_TYPE_TERMINUS_LOCATOR:
            partialPdrPtr = new TerminusLocatorPDR();
            break;
        case PDR_TYPE_FRU_RECORD_SET:
            partialPdrPtr = new FruRecordSetPDR();
            break;
        case PDR_TYPE_ENTITY_ASSOCIATION:
            partialPdrPtr = new EntityAssociationPDR();
            break;
        case PDR_TYPE_OEM_ENTITY_ID:
            partialPdrPtr = new OemEntityIdPDR();
            break;
        case PDR_TYPE_OEM_STATE_SET:
            partialPdrPtr = new OemStateSetPDR();
            break;
        default:
            // TODO: add support for other PDR types
            return -1;
        }
        */
    }

    // copy the data into the result
    partialPdrPtr->appendRawData(
        (unsigned char*)((unsigned char*)getPdrResponse) + sizeof(GetPdrResponse),
        getPdrResponse->responseCount);


    if (typeid(*partialPdrPtr) == typeid(GenericPdr))
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


bool PdrRepository::addPdrsFromNode(node node1) {
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
    return true;
}

void PdrRepository::dump() {
    // free memory for each element in the map
    for (map<uint32, GenericPdr*>::iterator it = repository.begin(); it != repository.end(); ++it) {
        it->second->dump();
        std::cout << std::endl;
    }
}
