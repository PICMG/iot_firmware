//*******************************************************************
//    GenericPdr.h
//
//    This file provides definition for a generic PDR class that is
//    intended to be used as part of the PICMG pldm library reference
//    code. This class contains a buffer of bytes that represents the 
//    raw data of the pdr itself.  The class also contains a dictionary
//    Json object that can be used to decode the bytes within the PDR.
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
	long         * offsets;  // an index of pre-computed field offsets within the rawData
	JsonObject   * dict;     // a pointer to the meta-data dictionary for this pdr
private:
	unsigned long getDataOffset(string key);
	bool isKeyEnabled(string key);
public:
	// construction/destruction
	GenericPdr();
	~GenericPdr();

	// configuration
	void setDictionary(JsonObject* dict);

	// communication helper function - copy new data into the abstract pdr buffer
	void appendRawData(unsigned char* newData, unsigned int size);

	// common header getters
	uint32 getRecordHandle();
	uint8  getHeaderVersion();
	uint8  getPdrType();
	uint16 getRecordChangeNumber();
	uint16 getDataLength();

	// field management
	bool keyExists(string key);
	string getValue(string key);
	string getDataType(string key);
	list<string> getEnumOptions(string key);

	// visualization
	virtual void dump();
};

