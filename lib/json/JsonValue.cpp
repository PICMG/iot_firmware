//*******************************************************************
//    JsonValue.h
//
//    This file provides implementation for a class that represents
//    a concrete JSON value (as opposed to an object or array).  This 
//    class represents the all JSON value types as a string.  
//    Getter functions interpret the string as specific data types.
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
#include <algorithm>
#include "JsonValue.h"

//*******************************************************************
// trimend()
//
// This is a static helper function that trims whitespace from the end
// of a given string.  The new string is returned as a result.
//
// parameters:
//   str - the string to trim
// returns:
//   a new version of the string with whitespace removed from the end
static string trimend(const string& str)
{
    unsigned long end = str.find_first_of(" \f\n\r\t\v");
    return str.substr(0, end);
}

//*******************************************************************
// match_no_case()
//
// This is a static helper returns true if the two specified strings
// match each other without respect to case.  Otherwise the function
// returns false.
//
// parameters:
//   str1, str2 - the strings to compare
// returns:
//   true if equivalent, otherwise, false.
static bool match_no_case(string str1, string str2) {
    // make str1 and str2 upper-casae
    transform(str1.begin(), str1.end(), str1.begin(), ::toupper);
    transform(str2.begin(), str2.end(), str2.begin(), ::toupper);
    return (str1.compare(str2)==0)?true:false;
}

//*******************************************************************
// JsonValue()
//
// default constructor.
JsonValue::JsonValue() {
}

//*******************************************************************
// JsonValue()
//
// Copy constructor - initialize this object as a deep clone of the 
// specified object.
// 
// parameters:
//    val - a reference of the object to clone.
JsonValue::JsonValue(const JsonValue &val) {
    value = val.value;
}

//*******************************************************************
// JsonValue()
//
// Initialization constructor.  Initialize this object from the
// specified string.
//
// parameters:
//    value - the string to initialize this object from
JsonValue::JsonValue(string value) {
    if (match_no_case(value,"null")) {
        this->value = "NULL";
    }
    else {
        this->value = value;
    }
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
JsonAbstractValue * JsonValue::copy() {
    JsonValue* val = new JsonValue;
    val->value = value;
    return val;
}

//*******************************************************************
// dump()
//
// a diagnostic function to dump the value of this object to the 
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
void JsonValue::dump(ostream& out, bool pretty, int indent,bool useIndent) {
    if ((useIndent)&&(pretty)) for (int i = 0;i < indent;i++) out<<" ";
    bool isNumber = false;
    char* endptr;

    string trimmed = trimend(value.c_str());
    strtod(trimmed.c_str(), &endptr);
    if (*endptr == 0) isNumber |= true;
    strtol(trimmed.c_str(), &endptr, 10);
    if (!isNumber) {
        out << "\"" << value << "\"";
    }
    else {
        out << trimmed;
    }
}

//*******************************************************************
// dump()
//
// a diagnostic function to dump the value of this object to the 
// specifed output stream.
// 
// parameters:
//    out - the output stream to write to
//    pretty - if true, the output will be indented with fields on
//       separate lines.
// returns:
//    void
void JsonValue::dump(ostream& out, bool pretty) {
    dump(out, pretty, 0, true);
}

//*******************************************************************
// getValue()
//
// returns the value as a string.
// 
// parameters:
//    specifier - used by other typues of AbstractJsonValue types to
//       select a specific field.  This should be "" for JsonValue objects.
// returns:
//    a string representation of the value.
string JsonValue::getValue(string specifier) {
    if (specifier == "") {
        return value;
    }
    return "";
}

//*******************************************************************
// getInteger()
//
// returns the value as an integer.
// 
// parameters:
//    specifier - used by other typues of AbstractJsonValue types to
//       select a specific field.  This should be "" for JsonValue objects.
// returns:
//    an integer representation of the value.
long JsonValue::getInteger(string specifier) {
    if (specifier == "") {
        return atol(value.c_str());
    }
    return 0;
}

//*******************************************************************
// getDouble()
//
// returns the value as a double-precision floating-point.
// 
// parameters:
//    specifier - used by other typues of AbstractJsonValue types to
//       select a specific field.  This should be "" for JsonValue objects.
// returns:
//    a double representation of the value.
double  JsonValue::getDouble(string specifier) {
    if (specifier == "") {
        return atof(value.c_str());
    }
    return 0.0;
}

//*******************************************************************
// getBool()
//
// returns the value as a bool.
// 
// parameters:
//    specifier - used by other typues of AbstractJsonValue types to
//       select a specific field.  This should be "" for JsonValue objects.
// returns:
//    a bool representation of the value.
bool JsonValue::getBoolean(string specifier) {
    if (specifier == "") {
        if (match_no_case(value,"true")) return true;
    }
    return false;
}

//*******************************************************************
// getHandle()
//
// returns the JSON Handle value as a string.
// 
// parameters:
//    specifier - used by other typues of AbstractJsonValue types to
//       select a specific field.  This should be "" for JsonValue objects.
// returns:
//    a string representation of the handle.
string  JsonValue::getHandle(string specifier) {
    return getValue(specifier);
};
