#pragma once
#include <map>
#include "JsonAbstractValue.h"
#include "JsonValue.h"
#include "JsonObject.h"
#include "JsonValue.h"
#include "GenericPdr.h"
#include "pldm.h"
#include "node.h"

class PdrRepository 
{	
private:
	map<uint32, GenericPdr*> repository;
	JsonObject* dictionary;
	int getPdrPart(node& node1, uint32 recordHandle, uint32& nextRecordHandle);
	JsonObject* findPdrTemplateFromId(unsigned long id);
public:
	PdrRepository();
	~PdrRepository();
	bool setDictionary(string dictionary_file);
	bool addPdrsFromNode(node node1);
	void dump();
};
