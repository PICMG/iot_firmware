//*******************************************************************
//    PdrMaker.cpp
//
//    This file implements code that builds a byte table for a given
//    JSON-based pdr table.  The byte table emitted is formatted in 
//    such a way that it can be built directly into IoT device firmware.
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
#include <iomanip>
#include <fstream>
#include "JsonFactory.h"

#define COMMON_HEADER_SIZE 10

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
JsonAbstractValue* loadJsonFile(const char* filename) {
    JsonFactory jf;

    ifstream jsonfile(filename, std::ifstream::binary);
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
// getMatchingPdrTemplate()
//
// Given a JsonArray of PDR templates, and a string specifying a PDR
// name, return the template JSON definition matching the PDR name
//
// parameters:
//    pdrs - an array of pdr tempates to check
//    type - the pdr type to find in the teplate array.
// returns:
//    a pointer to json tempate with a type matching the type
//    parameters, otherwise NULL.
JsonObject* getMatchingPdrTemplate(JsonArray& pdrs, string type) {
    JsonAbstractValue* av;
    if (type.length() == 0) return NULL;

    for (int i = 0;i < pdrs.size();i++) {
        // get the template
        av = pdrs.getElement(i);
        if ((!av) || (typeid(*av) != typeid(JsonObject))) continue;
        JsonObject& tmplt = *((JsonObject*)av);

        // find the value of the PDRType field
        av = tmplt.find("PDRType");
        if ((!av) || (typeid(*av) != typeid(JsonObject))) continue;            
        JsonObject& pdrtype = *((JsonObject*)av);

        // find the default value of the PDRType for this template PDR
        av = pdrtype.find("default");
        if ((!av) || (typeid(*av) != typeid(JsonValue))) continue;
        JsonValue& dflt = *((JsonValue *)av);
        
        // check to see if the types match
        if (dflt.getValue("").compare(type) == 0) return &tmplt;
    }
    return NULL;
}

//*******************************************************************
// findEnumerationValue()
//
// find an enumeration entry that matches the specified value and 
// return the value as a result, return "" if no match is found
//
// parameters:
//    enums - an array of enumeration choices
//    matchkey - the enumeration key to match.
// returns:
//    a string of the matching enumeration value, otherwise ""
string findEnumerationValue(JsonArray* enums, string matchkey) {
    if (matchkey == "") return "";

    // if the matchkey appears to be a number, dont perform the lookup
    if ((matchkey[0]>='0') && (matchkey[0] <= '9')) return matchkey;

    // loop for each element in the enumeration definition
    JsonAbstractValue* av;
    string result = "";
    for (int i = 0;i < enums->size();i++) {
        av = enums->getElement(i);
        if ((!av) || (typeid(*av) != typeid(JsonObject))) continue;
        if (((JsonObject*)av)->getValue("key") == matchkey) 
            return ((JsonObject*)av)->getValue("value");
    }
    return "";
}

//*******************************************************************
// emitByte()
//
// emit a single byte to the specified output stream.  If more than
// the specified number of bytes will be written on the current
// line, send a carriage return so the next bytes will be written to
// the next line.
//
// parameters:
//    out - the output stream to write to
//    byte - the byte to write
//    bytes_on_line - the number of bytes on the current line (updated
//       on exit)
//    bytecount - the total bytes emitted (updated on exit)
// returns:
//    noting
void emitByte(ostream& out, unsigned char byte, unsigned char& bytes_on_line, unsigned long& bytecount) {
    out << "0x" << hex << setw(2) << setfill('0') << (0x0ff & (unsigned int) byte) << ", ";
    bytes_on_line++; bytecount++;
    if (bytes_on_line == 0x10) {
        bytes_on_line = 0;
        out << endl << "   ";
    }
}

//*******************************************************************
// calculatePdrBytes()
//
// calculate the total number of bytes that will be required to 
// represent the specified pdr.  This function is similar to main()
// except bytes are not actually emitted.
//
// parameters:
//    tmplt - the template metadata for this pdr
//    pdr - the pdr to calculate byte size for
// returns:
//    the size (in bytes) required to represent the pdr
unsigned long calculatePdrBytes(JsonObject *tmplt, JsonObject &pdr ) {
    JsonAbstractValue* av;
    string type;
    unsigned long bytecount = 0;

    // for each field in the template (in order)
    for (int j = 0; j < tmplt->size(); j++) {
        // get the field name that should be matched
        string templateFieldName = tmplt->getElementKey(j);

        // get the field type (indirecting if required)
        av = tmplt->find(templateFieldName);
        if (!av) continue;
        JsonObject* meta = NULL;
        string defaultValueStr = "";
        if (typeid(*av) == typeid(JsonObject)) {
            // here if there is additional metadata for this item.
            meta = (JsonObject*)av;
            av = meta->find("type");
            if ((!av) || (typeid(*av) != typeid(JsonValue))) continue;
            type = av->getValue("");
            // perform indirection if required
            if (type[0] == '@') {
                av = pdr.find(type.substr(1, string::npos));
                if ((!av) || (typeid(*av) != (typeid(JsonValue)))) continue;
                type = av->getValue("");
            }

            defaultValueStr = meta->getValue("default");
        }
        else if (typeid(*av) == typeid(JsonValue)) {
            // here if the type is a concrete value or indirection key
            type = av->getValue("");
            if (type.length() == 0) continue;

            // perform indirection if required
            if (type[0] == '@') {
                av = pdr.find(type.substr(1, string::npos));
                if ((!av) || (typeid(*av) != (typeid(JsonValue)))) continue;
                type = av->getValue("");
            }
        }
        else {
            // here for jsonarray - this is unexpected
            continue;
        }

        // Get the value string from the pdr, or use the default if no
        // keyword exists
        string pdrValue = pdr.getValue(templateFieldName);
        if (pdrValue == "") pdrValue = defaultValueStr;

        // if the value is still blank, use a default of "0", unless the 
        // key contains [*] (which are optional fields). 
        if ((pdrValue == "") && (templateFieldName.find('[') == string::npos)) {
            pdrValue = "0";
        }
        else if (pdrValue == "") continue;

        // compute the length contribution based on type
        if ((type == "uint8") || (type == "enum8") || (type == "bitfield8") || (type == "sint8") || (type == "bool8")) {
            bytecount++;
        }
        else if ((type == "uint16") || (type == "sint16")) {
            bytecount += 2;
        }
        else if ((type == "uint32") || (type == "sint32") || (type == "real32")) {
            bytecount += 4;
        }
        else if (type == "ASCII") {
            bytecount += pdrValue.length() + 1;
        }
        else if ((type == "UTF16-BE") || (type == "UTF16")) {
            bytecount += pdrValue.length()*2 + 2;
        }
        else if (type == "UTF16-LE") {
            bytecount += pdrValue.length() * 2 + 2;
        }
        else {
            return 0;
        }
    }
    return (bytecount < COMMON_HEADER_SIZE)?0:bytecount-COMMON_HEADER_SIZE;
}

/////////////////////////////////////////////////////////////////////
// main()
//
// main program entry point.  This program takes two arguments:
// the full path to the json file to convert, and the full path 
// to the output file.  A third file (pldm_definitions.json) holds
// the pldm definitions that are used to perform the conversion.
//
int main(int argc, char *argv[]) {
    JsonAbstractValue* av;

    if (argc != 3) {
        cerr << "Wrong number of arguments.  Syntax: " << endl;
        cerr << "   PdrMaker infile.json outfile.c" << endl;
        return 0;
    }

    // load the pdr template file
    JsonAbstractValue *tmplt_defs = loadJsonFile("pldm_definitions.json");
    if ((!tmplt_defs) || (typeid(*tmplt_defs) != typeid(JsonObject))) {
        cerr << "Unable to open PLDM template Json file" << endl;
        return 0;
    }
    
    // find the PDR description array within the file
    av = ((JsonObject*)tmplt_defs)->find("pdr_defs");
    if ((!av) || (typeid(*av) != typeid(JsonArray))) {
        cerr << "Couldn't locate PDR templates in PLDM template Json file" << endl;
        return 0;
    }
    JsonArray &template_pdrs = *((JsonArray*)av);

    // load the pdr file
    JsonAbstractValue* pdrjson = loadJsonFile(argv[1]);
    if ((!pdrjson)||(typeid(*pdrjson) != typeid(JsonArray))) {
        cerr << "Invalid input Json file " <<argv[1]<< endl;
        return 0;
    }

    ofstream outfile(argv[2]);
    if (!outfile.is_open()) {
        cerr << "error opening output file " << argv[2] << endl;
        return 0;
    }

    unsigned long bytecount = 0;
    unsigned long max_record_size = 0;
    unsigned long number_of_records = 0;
    // for each value in the pdrjson
    outfile << "#include \"pdrdata.h\""<<endl<<endl;
    outfile << "PDR_BYTE_TYPE __pdr_data[] = { ";
    for (int i = 0;i < ((JsonArray*)pdrjson)->size();i++) {
        unsigned char bytes_on_line = 0;
        unsigned long record_start_byte = bytecount;
        // get the json value, skip if not a json object.
        av = ((JsonArray*)pdrjson)->getElement(i);
        if ((!av)||(typeid(*av) != typeid(JsonObject))) continue;
        JsonObject& pdr = *((JsonObject*)av);

        // get the type of the pdr - skip if not defined
        av = pdr.find("PDRType");
        if ((!av) || (typeid(*av) != typeid(JsonValue))) continue;
        string type = av->getValue("");

        // find the corresponding pdr in the pdr template
        JsonObject* tmplt = getMatchingPdrTemplate(template_pdrs, type);
        if (!tmplt) {
            cerr << "Unable to find matching PDR type: " << type << endl;
            continue;
        }

        outfile << endl << "   /* PDR TYPE : " << type<<" */"<<endl<<"   ";
        // for each field in the template (in order)
        for (int j = 0; j < tmplt->size(); j++) {
            // get the field name that should be matched
            string templateFieldName = tmplt->getElementKey(j);

            // get the field type (indirecting if required)
            av = tmplt->find(templateFieldName);
            if (!av) continue;
            JsonObject* meta = NULL;
            string defaultValueStr = "";
            if (typeid(*av) == typeid(JsonObject)) {
                meta = (JsonObject *)av;
                av = meta->find("type");
                if ((!av) || (typeid(*av) != typeid(JsonValue))) continue;
                type = av->getValue("");

                // perform indirection if required
                if (type[0] == '@') {
                    av = pdr.find(type.substr(1, string::npos));
                    if ((!av) || (typeid(*av) != (typeid(JsonValue)))) continue;
                    type = av->getValue("");
                }
                defaultValueStr = meta->getValue("default");
            }
            else if (typeid(*av)==typeid(JsonValue)) {
                // here if the type is a concrete value or indirection key
                type = av->getValue("");
                if (type.length() == 0) continue;
                
                // perform indirection if required
                if (type[0]=='@') {
                    string tname = type.substr(1, string::npos);
                    av = pdr.find(type.substr(1,string::npos));
                    if ((!av)||(typeid(*av)!=(typeid(JsonValue)))) continue;
                    type = av->getValue("");
                }
            }
            else {
                // here for jsonarray - this is unexpected
                continue;
            }

            // Get the value string from the pdr, or use the default if no
            // keyword exists
            string pdrValue = pdr.getValue(templateFieldName);
            if (pdrValue == "") pdrValue = defaultValueStr;
            
            // if the value is still blank, use a default of "0", unless the 
            // key contains [*] (which are optional fields). 
            if ((pdrValue == "") && (templateFieldName.find('[') == string::npos)) {
                pdrValue = "0";
            } else if (pdrValue=="") continue;
            
            // if this is an enumerated type, perform the lookup
            if (type == "enum8") {
                av = meta->find("values");
                if ((!av) || (typeid(*av) != typeid(JsonArray))) {
                    cerr<< "cannot find enumeration values for key: "<<templateFieldName<<endl;
                    outfile.close();
                    return 0;
                }
                string newValue = findEnumerationValue((JsonArray*)av, pdrValue);
                if (newValue == "") {
                    cerr << "cannot find matching enumeration '" << pdrValue << "' for key: " << templateFieldName << endl;
                    outfile.close();
                    return 0;
                }
                pdrValue = newValue;
            }

            // special case - if the field name is "DataLength", use the data length for this record
            if (templateFieldName == "dataLength") pdrValue = to_string(calculatePdrBytes(tmplt, pdr));

            // emit the value based on the type
            if ((type == "uint8") || (type == "enum8") || (type == "bitfield8") || (type=="sint8") || (type=="bool8")) {
                emitByte(outfile,(unsigned char)(0x0ff&atol(pdrValue.c_str())),bytes_on_line,bytecount);
            } 
            else if ((type == "uint16")||(type == "sint16")) {
                unsigned long val = (unsigned long)(atol(pdrValue.c_str()));
                emitByte(outfile, (unsigned char)(0x0ff & val), bytes_on_line, bytecount);
                val = val >> 8;
                emitByte(outfile, (unsigned char)(0x0ff & val), bytes_on_line, bytecount);
            }
            else if ((type == "uint32") || (type == "sint32")) {
                unsigned long val = (unsigned long)(atol(pdrValue.c_str()));
                emitByte(outfile, (unsigned char)(0x0ff & val), bytes_on_line, bytecount);
                val = val >> 8;
                emitByte(outfile, (unsigned char)(0x0ff & val), bytes_on_line, bytecount);
                val = val >> 8;
                emitByte(outfile, (unsigned char)(0x0ff & val), bytes_on_line, bytecount);
                val = val >> 8;
                emitByte(outfile, (unsigned char)(0x0ff & val), bytes_on_line, bytecount);
            }
            else if (type == "real32") {
                float f = atof(pdrValue.c_str());
                unsigned long val = *((unsigned long*)&f);
                emitByte(outfile, (unsigned char)(0x0ff & val), bytes_on_line, bytecount);
                val = val >> 8;
                emitByte(outfile, (unsigned char)(0x0ff & val), bytes_on_line, bytecount);
                val = val >> 8;
                emitByte(outfile, (unsigned char)(0x0ff & val), bytes_on_line, bytecount);
                val = val >> 8;
                emitByte(outfile, (unsigned char)(0x0ff & val), bytes_on_line, bytecount);
            }
            else if (type == "ASCII") {
                for (int chnm = 0;chnm < pdrValue.length();chnm++) {
                    emitByte(outfile, (unsigned char)(pdrValue[chnm]), bytes_on_line, bytecount);
                }
                emitByte(outfile, 0, bytes_on_line, bytecount);
            } 
            else if ((type == "UTF16-BE")||(type =="UTF16")) {
                // convert ascii input string to utf16 big endian format
                for (int chnm = 0;chnm < pdrValue.length();chnm++) {
                    unsigned char ch = pdrValue[chnm];
                    if (ch < 0x80) { 
                        emitByte(outfile, 0, bytes_on_line, bytecount);
                        emitByte(outfile, (unsigned char)(pdrValue[chnm]), bytes_on_line, bytecount);
                    } 
                    else {
                        emitByte(outfile, 0xC0 + ((unsigned char)(pdrValue[chnm])>>6), bytes_on_line, bytecount);
                        emitByte(outfile, 0x80 + ((unsigned char)(pdrValue[chnm]) & 0x3F), bytes_on_line, bytecount);
                    }
                }
                // null terminator
                emitByte(outfile, 0, bytes_on_line, bytecount);
                emitByte(outfile, 0, bytes_on_line, bytecount);
            }
            else if (type == "UTF16-LE") {
                // convert ascii input string to utf16 little endian format
                for (int chnm = 0;chnm < pdrValue.length();chnm++) {
                    unsigned char ch = pdrValue[chnm];
                    if (ch < 0x80) {
                        emitByte(outfile, (unsigned char)(pdrValue[chnm]), bytes_on_line, bytecount);
                        emitByte(outfile, 0, bytes_on_line, bytecount);
                    }
                    else {
                        emitByte(outfile, 0x80 + ((unsigned char)(pdrValue[chnm]) & 0x3F), bytes_on_line, bytecount);
                        emitByte(outfile, 0xC0 + ((unsigned char)(pdrValue[chnm]) >> 6), bytes_on_line, bytecount);
                    }
                }
                // null terminator
                emitByte(outfile, 0, bytes_on_line, bytecount);
                emitByte(outfile, 0, bytes_on_line, bytecount);
            }
            else {
                // unknown type
                cerr << "Unknown field type : " << type << endl;
                outfile.close();
                return 0;
            }
        }
        if (bytecount - record_start_byte > max_record_size) max_record_size = bytecount - record_start_byte;
        number_of_records++;
    }
    outfile << endl << "   /* END OF RECORDS */" << endl;
    outfile << "   0x00 " << endl << "};" << endl << endl;
    outfile << "unsigned int __pdr_total_size = " << dec << bytecount << ";" << endl;
    outfile << "unsigned int __pdr_number_of_records = " << number_of_records << ";" << endl;
    outfile << "unsigned int __pdr_max_record_size = " << max_record_size << ";" << endl;
    outfile.close();
    return 0;
}