#pragma once
#include "pldm.h"
#include "pldmnode.h"

class node : public PldmNode
{
	unsigned char rxBuffer[512];
	unsigned char txBuffer[512];

	//unsigned char  pdrRepository[1024];
	//unsigned int   pdrIndex[16];
	//unsigned char  pdrType[16];
	unsigned char  pdrCount;
	unsigned int   nextByte;
	unsigned int   maxPDRSize = 64;

	void addPDR(unsigned char *pdr, unsigned int type, unsigned char size);
	void parseCommand();
	unsigned int pdrSize(unsigned int index);
	void processCommandGetPdr(PldmRequestHeader* rxHeader, PldmResponseHeader* txHeader);
public:
	node();
	virtual void putCommand(PldmRequestHeader* hdr, unsigned char* command, unsigned int size);
	virtual unsigned char* getResponse(void);
};

