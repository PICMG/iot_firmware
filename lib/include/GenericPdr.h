#pragma once
#include <string>
#include <array>
#include "pldm.h"
#include "JsonObject.h"
#include "JsonArray.h"
#include "JsonValue.h"

class GenericPdr
{
	unsigned char* rawData;  // a buffer that contains the raw data for the PDR
	unsigned int   rawSize;  // the size of the raw data that has been allocated
	long         * offsets;
	JsonObject   * dict;
private:
	unsigned long getDataOffset(string key);
	bool isKeyEnabled(string key);
public:
	GenericPdr();
	~GenericPdr();

	void setDictionary(JsonObject* dict);

	// communication helper function - copy new data into the abstract pdr buffer
	void appendRawData(unsigned char* newData, unsigned int size);

	// common header getters
	uint32 getRecordHandle();
	uint8  getHeaderVersion();
	uint8  getPdrType();
	uint16 getRecordChangeNumber();
	uint16 getDataLength();
	bool keyExists(string key);
	string getValue(string key);
	string getDataType(string key);
	list<string> getEnumOptions(string key);
	virtual void dump();
};

