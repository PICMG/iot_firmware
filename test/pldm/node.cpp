#include "node.h"
#include "pdrdata.h"

static PdrCommonHeader* getPdrHeader(unsigned int index) {
    // iterate through the repository to find the header for the pdr indexed by
    // the given index parameter.

    // adjust the index for the pdr structure - assume records start at 1,
    // and increase sequentially.  0 is a special case which will also retrieve
    // the first record
    if (index > 0) index--;
    PdrCommonHeader* result = (PdrCommonHeader*)(__pdr_data);
    unsigned long offset = 0;
    unsigned int counter = 0;
    while (counter != index) {
        unsigned long part1 = sizeof(PdrCommonHeader);
        unsigned long part2 = result->dataLength;
        offset = offset + part1 + part2;
        result = (PdrCommonHeader*)(&__pdr_data[offset]);
        if (__pdr_data[offset] == 0) return 0;
        counter++;
    }
    return result;
}

static unsigned int getPdrOffset(unsigned int index) {
    // iterate through the repository to find the header for the pdr indexed by
    // the given index parameter.

    // adjust the index for the pdr structure - assume records start at 1,
    // and increase sequentially.  0 is a special case which will also retrieve
    // the first record
    if (index > 0) index--;

    PdrCommonHeader* hdr = (PdrCommonHeader*)(__pdr_data);
    unsigned int offset = 0;
    unsigned int counter = 0;
    while (counter != index) {
        unsigned long part1 = sizeof(PdrCommonHeader);
        unsigned long part2 = hdr->dataLength;
        offset = offset + part1 + part2;
        hdr = (PdrCommonHeader*)(&__pdr_data[offset]);
        if (__pdr_data[offset] == 0) return 0;
        counter++;
    }
    return offset;
}

static unsigned long  getNextRecord(unsigned long index) {
    unsigned result = 0;
    if (index == 0) result = 2;
    else result = index+1;
    return result;
}

node::node() {
    pdrCount = __pdr_number_of_records;
    nextByte = 0;
}

// returns the total pdr size (including the header)
unsigned int node::pdrSize(unsigned int index) {
    PdrCommonHeader * header = getPdrHeader(index);
    if (!header) return 0;
    return header->dataLength + sizeof(PdrCommonHeader);
}

void node::processCommandGetPdr(PldmRequestHeader* rxHeader, PldmResponseHeader* txHeader) 
{
    static char pdrTxState = 0;
    static unsigned int pdrNextHandle;
    static unsigned long pdrRecord;
    unsigned char* insertionPoint;
    unsigned int extractionPoint;

    GetPdrCommand* request = (GetPdrCommand*)(rxBuffer + sizeof(*rxHeader));
    GetPdrResponse* response = (GetPdrResponse*)(txBuffer + sizeof(*txHeader));
    unsigned char errorcode = 0;
    unsigned char* hpr;
    switch (pdrTxState) {
    case 0:
        // transfer has not begun yet
        if (request->transferOperationFlag != 0x1)
            errorcode = RESPONSE_INVALID_TRANSFER_OPERATION_FLAG;
        else if (request->recordHandle > pdrCount)
            errorcode = RESPONSE_INVALID_RECORD_HANDLE;
        else if (request->dataTransferHandle != 0x0000)
            errorcode = RESPONSE_INVALID_DATA_TRANSFER_HANDLE;
        else if (request->recordChangeNumber != 0x0000)
            errorcode = RESPONSE_INVALID_RECORD_CHANGE_NUMBER;
        if (errorcode) {
            // send the error response
            response->completionCode = errorcode;
            response->nextRecordHandle = 0;
            response->nextDataTransferHandle = 0;
            response->transferFlag = 0;
            response->responseCount = 0;
            return;
        }
        if (request->requestCount >= pdrSize(request->recordHandle)) {
            // send the data (single part)
            response->completionCode = RESPONSE_SUCCESS;
            response->nextRecordHandle = (getNextRecord(request->recordHandle) <= pdrCount) ?
                getNextRecord(request->recordHandle) : 0;
            response->nextDataTransferHandle = 0;
            response->transferFlag = 0x05;   // start and end
            response->responseCount = pdrSize(request->recordHandle);
            unsigned char* insertionPoint =
                txBuffer + sizeof(PldmResponseHeader) +
                sizeof(GetPdrResponse);
            unsigned int extractionPoint = getPdrOffset(request->recordHandle);
            // insert the pdr
            for (int i = 0;i < response->responseCount; i++) {
                *insertionPoint++ = __pdr_data[extractionPoint++];
            }
            return;
        }
        // send the data (multi-part)
        response->completionCode = RESPONSE_SUCCESS;
        response->nextRecordHandle = (getNextRecord(request->recordHandle) <= pdrCount) ?
            getNextRecord(request->recordHandle) : 0;
        response->nextDataTransferHandle = getPdrOffset(request->recordHandle) +
            request->requestCount;
        response->transferFlag = 0x0;            // start
        response->responseCount = request->requestCount;
        insertionPoint = txBuffer + sizeof(PldmResponseHeader) + sizeof(GetPdrResponse);
        extractionPoint = request->dataTransferHandle+getPdrOffset(request->recordHandle);
        // insert the pdr
        for (int i = 0;i < request->requestCount;i++) {
            *insertionPoint++ = __pdr_data[extractionPoint++];
        }
        pdrTxState = 1;
        pdrRecord = request->recordHandle;
        pdrNextHandle = response->nextDataTransferHandle;
        return;
    case 1:
        // transfer has already begun
        if (request->transferOperationFlag != 0x0)
            errorcode = RESPONSE_INVALID_TRANSFER_OPERATION_FLAG;
        else if (request->recordHandle != pdrRecord)
            errorcode = RESPONSE_INVALID_RECORD_HANDLE;
        else if (request->dataTransferHandle != pdrNextHandle)
            errorcode = RESPONSE_INVALID_DATA_TRANSFER_HANDLE;
        else if (request->recordChangeNumber != 0x0000)
            errorcode = RESPONSE_INVALID_RECORD_CHANGE_NUMBER;
        if (errorcode) {
            // send the error response
            response->completionCode = errorcode;
            response->nextRecordHandle = 0;
            response->nextDataTransferHandle = 0;
            response->transferFlag = 0;
            response->responseCount = 0;
            return;
        }
        if (request->requestCount + request->dataTransferHandle >= getPdrOffset(request->recordHandle)+pdrSize(request->recordHandle)) {
            // send the last part of the data
            response->completionCode = RESPONSE_SUCCESS;
            response->nextRecordHandle = (getNextRecord(request->recordHandle) <= pdrCount) ?
                getNextRecord(request->recordHandle) : 0;
            response->nextDataTransferHandle = 0;
            response->transferFlag = 0x04;   // end
            response->responseCount =
                 pdrSize(request->recordHandle) -
                (request->dataTransferHandle-getPdrOffset(request->recordHandle));
            unsigned char* insertionPoint =
                txBuffer + sizeof(PldmResponseHeader) +
                sizeof(GetPdrResponse);
            unsigned int extractionPoint = request->dataTransferHandle;
            for (unsigned int i = 0;i < response->responseCount;i++) {
                *insertionPoint++ = __pdr_data[extractionPoint++];
            }
            pdrTxState = 0;
            return;
        }
        // send the middle data (multi-part)
        response->completionCode = RESPONSE_SUCCESS;
        response->nextRecordHandle = (getNextRecord(request->recordHandle) <= pdrCount) ?
            getNextRecord(request->recordHandle) : 0;
        response->nextDataTransferHandle =
            request->dataTransferHandle + request->requestCount;
        response->transferFlag = 0x1;            // middle
        response->responseCount = request->requestCount;
        insertionPoint =
            txBuffer + sizeof(PldmResponseHeader) +
            sizeof(GetPdrResponse);
        extractionPoint = request->dataTransferHandle;
        for (int i = 0;i < request->requestCount;i++) {
            *insertionPoint++ = __pdr_data[extractionPoint++];
        }
        pdrTxState = 1;
        pdrNextHandle = request->dataTransferHandle + request->requestCount;
        return;
    }
}

void node::parseCommand()
{
    // cast the relevant portions of the header so that
    // they are easier to use later.
    PldmRequestHeader* rxHeader = (PldmRequestHeader*)rxBuffer;

    PldmResponseHeader* txHeader = (PldmResponseHeader*)txBuffer;
    txHeader->flags1 = rxHeader->flags1 | 0x80;
    txHeader->flags2 = rxHeader->flags2;
    txHeader->command = rxHeader->command;

    // switch based on the command type byte in the header
    switch (rxHeader->command) {
    case CMD_GET_SENSOR_READING:
        //GetSensorReading(cmdBuffer);
        break;
    case CMD_GET_STATE_SENSOR_READINGS:
        //GetStateSensorReadings(cmdBuffer);
        break;
    case CMD_SET_NUMERIC_EFFECTER_VALUE:
        //SetNumericEffecterValue(cmdBuffer);
        break;
    case CMD_GET_NUMERIC_EFFECTER_VALUE:
        //GetNumericEffecterValue(cmdBuffer);
        break;
    case CMD_SET_STATE_EFFECTER_STATES:
        //SetStateEffecterStates(cmdBuffer);
        break;
    case CMD_GET_STATE_EFFECTER_STATES:
        //GetStateEffecterStates(cmdBuffer);
        break;
    case CMD_GET_PDR_REPOSITORY_INFO:
    {
        GetPdrRepositoryInfoResponse* response = (GetPdrRepositoryInfoResponse*)(txBuffer + sizeof(*txHeader));
        response->completionCode = RESPONSE_SUCCESS;
        response->repositoryState = 0;           // available
        response->recordCount = pdrCount;
        response->repositorySize = __pdr_total_size;
        response->largestRecordSize = __pdr_max_record_size;
        response->dataTransferHandleTimeout = 0; // no timeout
        break;
    }
    case CMD_GET_PDR:
        processCommandGetPdr(rxHeader, txHeader);
        break;
    }
    return;
}

unsigned char* node::getResponse() {
    parseCommand();
    return txBuffer;
}

void node::putCommand(PldmRequestHeader* hdr, unsigned char* command, unsigned int size) {
    unsigned char* cp = rxBuffer;
    unsigned char* cmdptr = (unsigned char*)hdr;
    for (unsigned int i = 0;i < sizeof(*hdr);i++) *(cp++) = *(cmdptr++);
    for (unsigned int i = 0;i < size; i++) *(cp++) = *(command++);
}