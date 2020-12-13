//*******************************************************************
//    GenericPdr.cpp
//
//    This file provides implementation of a generic PDR class that is
//    intended to be used as part of the PICMG pldm library reference
//    code. This class contains a buffer of bytes that represents the 
//    raw data of the pdr itself.  The class also contains a dictionary
//    Json object that can be used to decode the bytes within the PDR.
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
#include <string>
#include <cstring>  // for memcpy
#include <codecvt>
#include <locale>
#include <iostream>
#include <iomanip>
#include "JsonFactory.h"
#include "JsonAbstractValue.h"
#include "JsonArray.h"
#include "JsonObject.h"
#include "JsonValue.h"
#include "GenericPdr.h"

using namespace std;

//*******************************************************************
// GenericPdr()
//
// Constructor for the GenericPDR Class
GenericPdr::GenericPdr() : offsets(NULL), rawData(NULL), rawSize(0), dict(NULL) {
}

//*******************************************************************
// ~GenericPdr()
//
// Destructor for the GenericPdr class - free any dynamically 
// allocated memory.
GenericPdr::~GenericPdr() {
	if (offsets) delete[] offsets;
	if (rawData) delete[] rawData;
}

//*******************************************************************
// appendRawData()
//
// This is a communication helper function that copies new data
// into the GenericPdr's raw data buffer.  The rawData buffer
// is resized as required.
//
// parameters:
//    newData - a pointer to the new data to be copied
//    size    - the number of bytes to be copied
// returns:
//    void
void GenericPdr::appendRawData(unsigned char* newData, unsigned int size) {
	// add the data into the buffer, resizing as required
	if (!rawData) {
		// create a new buffer
		rawData = new unsigned char[size];
	}
	else {
		// expand the buffer size
		unsigned char* newBuffer = new unsigned char[size + rawSize];
		memcpy(newBuffer, rawData, rawSize);
		delete[] rawData;
		rawData = newBuffer;
	}
	// copy the new data into the buffer
	memcpy(rawData + rawSize, newData, size);
	rawSize += size;
}

//*******************************************************************
// getRecordHandle()
//
// return the record handle from the pdr data stored in the object's
// rawData representation of the PDR.
//
// parameters:
//    none
// returns:
//    the PDR record handle for this PDR
uint32 GenericPdr::getRecordHandle() {
	return ((PdrCommonHeader*)rawData)->recordHandle;
}

//*******************************************************************
// getHeaderVersion()
//
// return the header version from the pdr data stored in the object's
// rawData representation of the PDR.
//
// parameters:
//    none
// returns:
//    the PDR header version for this PDR
uint8 GenericPdr::getHeaderVersion() {
	return ((PdrCommonHeader*)rawData)->PDRHeaderVersion;
}

//*******************************************************************
// getPdrType()
//
// return the Pdr type from the pdr data stored in the object's
// rawData representation of the PDR.
//
// parameters:
//    none
// returns:
//    the PDR type for this PDR
uint8 GenericPdr::getPdrType() {
	return ((PdrCommonHeader*)rawData)->PDRType;
}

uint16 GenericPdr::getRecordChangeNumber() {
	return ((PdrCommonHeader*)rawData)->recordChangeNumber;
}

uint16 GenericPdr::getDataLength() {
	return ((PdrCommonHeader*)rawData)->dataLength;
}

bool GenericPdr::keyExists(string key) {
	return (dict->find(key)!=NULL);
}

bool GenericPdr::isKeyEnabled(string key) {
	if (key.empty()) return false;                   // not valid - so not enabled
	if (key.find('[') == string::npos) return true;  // not an array key - so, always enabled

	// get the metadata for this object
	JsonAbstractValue* av = dict->find(key);
	if (!av) return false;
	if (typeid(*av) == typeid(JsonObject)) {
		// check for limit conditions, return if there are none
		av = ((JsonObject*)av)->find("limitConditions");		
		if (!av) return true;

		// check to see if each of the limit conditions is met
		if (typeid(*av) != typeid(JsonArray)) return true;  // no limits
		JsonArray* ary = (JsonArray*)av;
		for (unsigned int i = 0; i < ary->size(); i++) {
			av = ary->getElement(i);
			if (typeid(*av) == typeid(JsonObject)) {
				// here if the element is a json object - check the 
				// limit and value conditions to see if they are met
				JsonAbstractValue* limit = ((JsonObject*)av)->find("limit");
				JsonAbstractValue* value = ((JsonObject*)av)->find("value");
				if ((!limit) || (!value)) continue;
				if (typeid(*limit) != (typeid(JsonValue))) continue;
				if (typeid(*value) != (typeid(JsonValue))) continue;
				unsigned int limitNum = limit->getInteger("");
				string valueKey = value->getValue("");
				if (atoi(getValue(valueKey.substr(1, string::npos)).c_str()) < limitNum) return false;
			}
		}
	}
	return true;
}

unsigned long GenericPdr::getDataOffset(string requested_key) {
	if (!offsets) return 0;
	unsigned long result = 0;

	// see if the offset has already been calculated - if so, use it
	for (unsigned long i = 0;i < dict->size();i++) {
		string key = dict->getElementKey(i);
		if (key == requested_key) {
			if (offsets[i] >= 0) return offsets[i];
		}
	}

	// otherwise, calculate the offset
	for (unsigned long i = 0;i < dict->size();i++) {
		string key = dict->getElementKey(i);

		if (isKeyEnabled(key)) {
			// if the key has been found, return the offset
			if (key == requested_key) {
				offsets[i] = result;
				return result;
			}
			string type = getDataType(key);
			
			if (type == "ASCII") {
				// find the terimating null character
				while (rawData[result]!= 0) result ++;
				// advance past the terminating null
				result++;
			}
			else if (type.find("UTF") != string::npos) {
				// find the terminating null word
				while ((rawData[result] != 0) || (rawData[result + 1] != 0)) result += 2;
				// advance past the terminating null word
				result += 2;
			}
			else if (type.find("8") != string::npos) {
				result += 1;
			}
			else if (type.find("16") != string::npos) {
				result += 2;
			}
			else if (type.find("32") != string::npos) {
				result += 4;
			}
		}
	}
	return 0;
}

void GenericPdr::setDictionary(JsonObject* dict) {
	this->dict = dict;
	offsets = new long[dict->size()];
	for (int i = 0;i < dict->size();i++) offsets[i] = -1;
}

string GenericPdr::getValue(string key) {
	if (!isKeyEnabled(key)) return "";
	unsigned long offset = getDataOffset(key);
	string type = getDataType(key);

	if (type == "ASCII") {
		string result = (const char*)(&rawData[offset]);
		return result;
	}
	if (type.find("UTF")!=string::npos) {
		// find the terminating null word
		u16string wideresult = u"";
		unsigned long charnum = offset;
		while ((rawData[charnum] != 0) || (rawData[charnum+1] != 0)) {
			wideresult.append(1,(char16_t)((((unsigned int)rawData[charnum])<<8) + rawData[charnum+1]));
			charnum += 2;
		}
//		u16string wideresult = (char16_t*)(&rawData[offset]);
		string result = std::wstring_convert<codecvt_utf8_utf16<char16_t>,char16_t>().to_bytes(wideresult);
		return result;
	}
	if ((type.find("enum") != string::npos)||(type.find("bool") != string::npos)) {
		unsigned int val = ((unsigned int)rawData[offset]);
		// return the enumeration that matches the value
		JsonObject* meta = (JsonObject*)dict->find(key);
		JsonArray* enums = (JsonArray*)meta->find("values");

		for (int enum_choice = 0; enum_choice < enums->size();enum_choice++) {
			JsonObject* choice = (JsonObject*)enums->getElement(enum_choice);
			if (choice->getInteger("value") == val) return choice->getValue("key");
		}
		return "unknown";
	}
	else if (type.find("8") != string::npos) {
		if (type[0]=='s') return to_string((int)rawData[offset]);
		return to_string((unsigned int)rawData[offset]);
	}
	else if (type.find("16") != string::npos) {
		if (type[0] == 's') 
			return to_string((signed int)((((unsigned int)rawData[offset+1])<<8)+(unsigned int)rawData[offset]));
		return to_string(((((unsigned int)rawData[offset+1]) << 8) + (unsigned int)rawData[offset]));
	}
	else if (type.find("32") != string::npos) {
		if (type[0] == 's')
			return to_string((signed long)(
				(((unsigned long)rawData[offset + 3]) << 24) + (((unsigned long)rawData[offset+2])<<16) +
				(((unsigned long)rawData[offset + 1]) << 8) + (unsigned long)rawData[offset])
			);
		if (type[0] == 'u')
			return to_string(
				(((unsigned long)rawData[offset + 3]) << 24) + (((unsigned long)rawData[offset + 2]) << 16) +
				(((unsigned long)rawData[offset + 1]) << 8) + (unsigned long)rawData[offset]);
		// here for float 32;
		float f;
		unsigned char* fp = (unsigned char*)&f;
		for (int i = 0;i<4;i++) fp[i] = rawData[offset+i];
		return to_string(f);
	}
	return "";
}

string GenericPdr::getDataType(string key) {
	JsonAbstractValue* av = dict->find(key);
	if (typeid(*av) == typeid(JsonValue)) {
		string type = av->getValue("");
		if (type.empty()) return "";
		// perform indirection if required
		if (type[0] == '@') {
			return getValue(type.substr(1, string::npos));
		}
		return type;
	}
	if (typeid(*av) == typeid(JsonObject)) {
		// check the metadata for the type
		string type = ((JsonObject*)av)->getValue("type");
		if (type.empty()) return "";
		// perform indirection if required
		if (type[0] == '@') {
			return getValue(type.substr(1, string::npos));
		}
		return type;
	}
	return "";
}


void GenericPdr::dump() {
	for (unsigned int i = 0;i < dict->size();i++) {
		string key = dict->getElementKey(i);
		if (isKeyEnabled(key)) {
			cout << setfill(' ') << left << setw(35) << key;
			cout << setw(0) << ": ";
			if (getDataType(key).find("bitfield") != string::npos) {
				cout << right << setw(4) << setfill('0') << hex << atol(getValue(key).c_str())<<endl;
			}
			else {
				cout << right << setw(0) << getValue(key) << endl;
			}
		}
	}
}
