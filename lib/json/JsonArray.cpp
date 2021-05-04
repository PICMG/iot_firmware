//*******************************************************************
//    JsonArray.h
//
//    This file provides implementation for a class that represents
//    a JSON arrays (json content enclosed in square bracket characters).
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
#include "JsonArray.h"

//*******************************************************************
// JsonArray()
//
// default constructor.
JsonArray::JsonArray() {

}

//*******************************************************************
// JsonArray()
//
// Copy constructor - initialize this object as a deep clone of the 
// specified object.
// 
// parameters:
//    val - a reference of the object to clone.
JsonArray::JsonArray(const JsonArray &ary) {
    for (jsonarray::iterator it = internal_map.begin(); it != internal_map.end(); ++it) {
        unsigned long key = it->first;
        JsonAbstractValue& value = *(it->second);

        // erase exsiting entry if it exists (this shouldnt happen)
        jsonarray::iterator it2 = internal_map.find(key);
        if (it2 != internal_map.end()) {
            delete it2->second;
            internal_map.erase(it2);
        }
        // add the new entry
        internal_map.insert(pair<unsigned long, JsonAbstractValue*>(key, it->second->copy()));
    }
}

//*******************************************************************
// ~JsonArray()
//
// destructor - deallocate any memory associated with this object.
JsonArray::~JsonArray() {
    for (jsonarray::iterator it = internal_map.begin(); it != internal_map.end(); ++it) 
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
JsonAbstractValue* JsonArray::copy() {
    return (JsonAbstractValue*) new JsonArray(*this);
}

//*******************************************************************
// add()
//
// add a new element into the array.
// 
// parameters:
//    val - the new value to add
// returns:
//    void
void JsonArray::add(JsonAbstractValue *val) {
    internal_map.insert(pair<unsigned int, JsonAbstractValue*>(internal_map.size(), val));
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
void JsonArray::dump(ostream& out, bool pretty, int indent, bool useIndent) {
    out << "[";
    if (pretty) cout<<endl;
    bool firstElement = false;
    for (jsonarray::iterator it = internal_map.begin(); it != internal_map.end(); ++it) {
        it->second->dump(out, pretty, indent+3,true);
        jsonarray::iterator itnext = it;
        itnext++;
        if (itnext != internal_map.end()) {
            out << ",";
        }
        if (pretty) out << endl;
    }
    if (pretty) for (int i = 0;i < indent;i++) out<<" ";
    out<<"]";
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
void JsonArray::dump(ostream& out, bool pretty) {
    dump(out, pretty, 0, true);
}

//*******************************************************************
// getValue()
//
// returns a string value of the specified element where the specifier
// is of the form:
// 		[index].key
// 
// parameters:
//    specifier - the key for the value to return - or empty to return 
//       a string representation of the entire object (not normal)
// returns:
//    a string representation of the requested value
string JsonArray::getValue(string specifier) {
    if (internal_map.empty()) return "";

    if (specifier=="") {
        // The specifier is empty - return values for all records in the
        // object (this should not be normal)
        string result;
        for (jsonarray::iterator it = internal_map.begin(); it != internal_map.end(); ++it) {
            result.append(it->second->getValue(""));
            result.append(", ");
        }
        return result;
    }
    // use the leftmost part of the specifier as the key
    string index = specifier.substr(1, specifier.find("]") - 1);
    string spec2 = specifier.substr(specifier.find(".") + 1, specifier.length() - specifier.find(".") - 1);
    jsonarray::iterator it = internal_map.find(atol(index.c_str()));
    if (it != internal_map.end()) {
        return it->second->getValue(spec2);
    }
    return "";
}

//*******************************************************************
// getBoolean()
//
// returns a boolean value of the specified element where the specifier
// is of the form:
// 		[index].key
// 
// parameters:
//    specifier - the key for the value to return - or empty to return 
//       a string representation of the entire object (not normal)
// returns:
//    a boolean representation of the requested value
bool JsonArray::getBoolean(string specifier) {
    if (specifier == "") return false;
    // use the leftmost part of the specifier as the key
    string index = specifier.substr(1, specifier.find("]") - 1);
    string spec2 = specifier.substr(specifier.find(".") + 1, specifier.length() - specifier.find(".") - 1);
    jsonarray::iterator it = internal_map.find(atol(index.c_str()));
    if (it != internal_map.end()) {
        return it->second->getBoolean(spec2);
    }
    return "";
}

//*******************************************************************
// getHandle()
//
// returns a string value of the specified handle element where the 
// specifier is of the form:
// 		[index].key
// 
// parameters:
//    specifier - the key for the value to return - or empty to return 
//       a string representation of the entire object (not normal)
// returns:
//    a string representation of the requested handle
string JsonArray::getHandle(string specifier) {
    if (specifier == "") return "";
    // use the leftmost part of the specifier as the key
    string index = specifier.substr(1, specifier.find("]") - 1);
    string spec2 = specifier.substr(specifier.find(".") + 1, specifier.length() - specifier.find(".") - 1);
    jsonarray::iterator it = internal_map.find(atol(index.c_str()));
    if (it != internal_map.end()) {
        return it->second->getHandle(spec2);
    }
    return "";
}

//*******************************************************************
// getInteger()
//
// returns an integer value of the specified element where the specifier
// is of the form:
// 		[index].key
// 
// parameters:
//    specifier - the key for the value to return - or empty to return 
//       a string representation of the entire object (not normal)
// returns:
//    an integer representation of the requested value
long JsonArray::getInteger(string specifier) {
    if (specifier == "") return 0;
    // use the leftmost part of the specifier as the key
    string index = specifier.substr(1, specifier.find("]") - 1);
    string spec2 = specifier.substr(specifier.find(".") + 1, specifier.length() - specifier.find(".") - 1);
    jsonarray::iterator it = internal_map.find(atol(index.c_str()));
    if (it != internal_map.end()) {
        return it->second->getInteger(spec2);
    }
    return 0;
}


//*******************************************************************
// getDouble()
//
// returns a double value of the specified element where the specifier
// is of the form:
// 		[index].key
// 
// parameters:
//    specifier - the key for the value to return - or empty to return 
//       a string representation of the entire object (not normal)
// returns:
//    a double representation of the requested value
double JsonArray::getDouble(string specifier) {
    if (specifier == "") return 0.0;
    // use the leftmost part of the specifier as the key
    string index = specifier.substr(1, specifier.find("]") - 1);
    string spec2 = specifier.substr(specifier.find(".") + 1, specifier.length() - specifier.find(".") - 1);
    jsonarray::iterator it = internal_map.find(atol(index.c_str()));
    if (it != internal_map.end()) {
        return it->second->getDouble(spec2);
    }
    return 0.0;
}

//*******************************************************************
// size()
//
// return the number of elements within this array.
// 
// parameters:
//    none.
// returns:
//    the number of fields in this array.
unsigned long JsonArray::size() {
    return internal_map.size();
}

//*******************************************************************
// getElement()
//
// return the value of the indexed element.
// 
// parameters:
//    none.
// returns:
//    a pointer to the JsonAbstractValue indexed by the input parameter.
//    NULL if the element does not exist.
JsonAbstractValue* JsonArray::getElement(unsigned long index) {
    // attempt to find the indexed item in the array
    jsonarray::iterator it = internal_map.find(index);

    // return the result if found, otherwise, null
    if (it != internal_map.end()) return it->second;
    return NULL;
}
