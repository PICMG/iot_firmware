#pragma once
#include "pldm.h"

class PldmNode
{
public:
	virtual void putCommand(PldmRequestHeader* hdr, unsigned char* command, unsigned int size) = 0;
	virtual unsigned char* getResponse(void) = 0;
};

