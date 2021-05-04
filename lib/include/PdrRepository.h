//*******************************************************************
//    PdrRepository.h
//
//    This file defines a PDR repositiory class that is
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
#pragma once
#include <map>
#include "JsonAbstractValue.h"
#include "JsonValue.h"
#include "JsonObject.h"
#include "JsonValue.h"
#include "GenericPdr.h"
#include "pldm.h"
#include "pldmnode.h"

class PdrRepository 
{	
private:
	map<uint32, GenericPdr*> repository;
	JsonObject* dictionary;
	int getPdrPart(PldmNode& node1, uint32 recordHandle, uint32& nextRecordHandle);
	JsonObject* findPdrTemplateFromId(unsigned long id);
    JsonAbstractValue* loadJsonFile(const char* filename);
public:
	PdrRepository();
	~PdrRepository();
	bool setDictionary(string dictionary_file);
	string getEntityTypeString(uint16 entityType);
	bool addPdrsFromNode(PldmNode& node1);
	GenericPdr * getPdrFromRecordHandle(uint32 recordNumber);
	map<unsigned int,string> getStateSet(uint32 stateSetId);
	sint32 getStateNumberFromName(uint32 stateSetId, string stateName);
	void dump();
};
