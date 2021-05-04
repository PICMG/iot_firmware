//*******************************************************************
//    JsonObject.h
//
//    This file provides implementation for a class that represents
//    a JSON object (json content enclosed in curly-brace characters).
//    This header is intended to be used as part of the PICMG IoT 
//    library reference code. 
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
#include "JsonObject.h"

//*******************************************************************
// JsonObject()
//
// default constructor.
JsonObject::JsonObject() {
}

//*******************************************************************
// JsonObject()
//
// Copy constructor - initialize this object as a deep clone of the 
// specified object.
// 
// parameters:
//    val - a reference of the object to clone.
JsonObject::JsonObject(const JsonObject &obj) {
    // iterate through each element in the internal_map
    for (jsonmap::iterator it = internal_map.begin(); it != internal_map.end(); ++it) {
        string key = it->first;
        JsonAbstractValue &value = *(it->second);
        
        // erase exsiting entry if it exists (this shouldnt happen)
        jsonmap::iterator it2 = internal_map.find(key);
        if (it2 != internal_map.end()) {
            delete it2->second;
            internal_map.erase(it2);
        }
        // add the new entry
        internal_map.insert(pair<string, JsonAbstractValue*>(key,it->second->copy()));
    }
}

//*******************************************************************
// ~JsonObject()
//
// destructor - deallocate any memory associated with this object.
JsonObject::~JsonObject() {
    for (jsonmap::iterator it = internal_map.begin(); it != internal_map.end(); ++it)
        delete it->second;
}

//*******************************************************************
// copy()
//
// create a deep clone of this JsonValue and return the result.
// 
// parameters:
//    none
// returns:
//    a pointer to a deep clone of the object
JsonAbstractValue* JsonObject::copy() {
    return (JsonAbstractValue*) new JsonObject(*this);
}

//*******************************************************************
// put()
//
// add a new element into the object.
// 
// parameters:
//    key - the key to associate the new value with
//    val - the new value to add
// returns:
//    void
void JsonObject::put(string key, JsonAbstractValue* val) {
    jsonmap::iterator it2 = internal_map.find(key);
    if (it2 != internal_map.end()) {
        delete it2->second;
        internal_map.erase(it2);

        // add the new entry
        internal_map.insert(pair<string, JsonAbstractValue*>(key, val));
    }
    else {
        // add the new entry and internal_map index
        internal_map.insert(pair<string, JsonAbstractValue*>(key, val));
        index.push_back(key);
    }
}

//*******************************************************************
// dump()
//
// a diagnostic function to dump this object to the 
// specifed output stream.
// 
// parameters:
//    out - the output stream to write to
//    pretty - if true, the output will be indented with fields on
//       separate lines.
//    indent - the indentation level to be used for this value
//    useIndent - true if indentation should be used, otherwise false
// returns:
//    void
void JsonObject::dump(ostream& out, bool pretty, int indent, bool useIndent) {
    if ((useIndent)&&(pretty))  for (int i = 0;i < indent;i++) out<<" ";
    out << "{";
    if (pretty) out<< endl;

    for (jsonmapindex::iterator it = index.begin(); it != index.end(); ++it) {
        if (pretty) for (int i = 0;i < indent + 3;i++) out<<" ";
        string key = internal_map.find(it->data())->first;
        JsonAbstractValue& value = *internal_map.find(it->data())->second;

        out << "\""<<key<<"\":";
        value.dump(out, pretty, indent + 3,false);
        jsonmapindex::iterator itnext = it;
        itnext++;
        if (itnext != index.end()) {
            out << ",";
        }
        if (pretty) out << endl;
    }
    if (pretty) for (int i = 0;i < indent;i++) out<<" ";
    out<<"}";
}

//*******************************************************************
// dump()
//
// a diagnostic function to dump this object to the 
// specifed output stream.
// 
// parameters:
//    out - the output stream to write to
//    pretty - if true, the output will be indented with fields on
//       separate lines.
// returns:
//    void
void JsonObject::dump(ostream& out, bool pretty) {
    dump(out, pretty, 0, true);
}

//*******************************************************************
// getValue()
//
// returns a string value of the specified element where the specifier
// is of the form:
// 		Empty - return the entire object
// 		key - return the value associated with the specific key
// 
// parameters:
//    specifier - the key for the value to return - or empty to return 
//       a string representation of the entire object (not normal)
// returns:
//    a string representation of the requested value
string JsonObject::getValue(string specifier) {
    if (internal_map.empty()) return "";

    string result = "";
    if (specifier == "") {
        // The specifier is empty - return values for all records in the
        // object (this should not be normal)
        for (jsonmap::iterator it = internal_map.begin(); it != internal_map.end(); ++it) {
            string key = it->first;
            JsonAbstractValue& value = *(it->second);
            result.append("\"");
            result.append(key);
            result.append("\":");
            result.append(value.getValue(""));
            result.append("\n");
        }
        return result;
    }
    // the specifier as the key
    jsonmap::iterator it = internal_map.find(specifier);
    if (it != internal_map.end()) return it->second->getValue("");
    return "";
}

    
//*******************************************************************
// getBoolean()
//
// returns a boolean representation of the value specified by the 
// input parameter.
// 
// parameters:
//    specifier - the key for the value to return
// returns:
//    a boolean representation of the requested value (if found), 
//    otherwise, false
bool JsonObject::getBoolean(string specifier) {
    if (internal_map.empty()) return "";
    if (specifier == "") return false;

    // the specifier as the key
    jsonmap::iterator it = internal_map.find(specifier);
    if (it != internal_map.end()) return it->second->getBoolean("");
    return false;
}

    
//*******************************************************************
// getBoolean()
//
// returns a string representation of the handle specified by the 
// input parameter.
// 
// parameters:
//    specifier - the key for the value to return
// returns:
//    a string representation of the requested handle (if found), 
//    otherwise, an empty string
string JsonObject::getHandle(string specifier) {
    if (internal_map.empty()) return "";
    if (specifier == "") return "";

    // the specifier as the key
    jsonmap::iterator it = internal_map.find(specifier);
    if (it != internal_map.end()) return it->second->getHandle("");
    return "NULL";
}

    
//*******************************************************************
// getInteger()
//
// returns an integer representation of the value specified by the 
// input parameter.
// 
// parameters:
//    specifier - the key for the value to return
// returns:
//    an integer representation of the requested value (if found), 
//    otherwise, zero
long JsonObject::getInteger(string specifier) {
    if (internal_map.empty()) return 0;
    if (specifier == "") return 0;

    // the specifier as the key
    jsonmap::iterator it = internal_map.find(specifier);
    if (it != internal_map.end()) return it->second->getInteger("");
    return 0;
}

    
//*******************************************************************
// getDouble()
//
// returns a double representation of the value specified by the 
// input parameter.
// 
// parameters:
//    specifier - the key for the value to return
// returns:
//    a double representation of the requested value (if found), 
//    otherwise, zero
double JsonObject::getDouble(string specifier) {
    if (internal_map.empty()) return 0.0;
    if (specifier == "") return 0.0;

    // the specifier as the key
    jsonmap::iterator it = internal_map.find(specifier);
    if (it != internal_map.end()) return it->second->getDouble("");
    return 0.0;
}

//*******************************************************************
// find()
//
// find and return the value associated with the specified key.
// 
// parameters:
//    specifier - the key for the value to return
// returns:
//    a pointer to a JsonAbstractValue associated with the key, otherwise
//    NULL
JsonAbstractValue* JsonObject::find(string key) {
    map<string,JsonAbstractValue*>::iterator it = internal_map.find(key);
    if (it == internal_map.end()) return NULL;
    return internal_map.find(key)->second;
}
    
//*******************************************************************
// size()
//
// return the number of elements within this object.
// 
// parameters:
//    none.
// returns:
//    the number of fields in this object.
unsigned long JsonObject::size() {
    return internal_map.size();
}

//*******************************************************************
// getElementKey()
//
// returns the key for the nth indext element within the object.
// 
// parameters:
//    idx - the index number for the element to retrieve the key for.
// returns:
//    the key for the nth element, otherwise an empty string.
string JsonObject::getElementKey(unsigned long idx) {
    if (idx >= index.size()) return "";

    // attempt to find the indexed item in the array
    jsonmapindex::iterator it = index.begin();
    for (unsigned int i = 0; i < idx; i++) it++;
    
    // return the result
    return it->data();
}
