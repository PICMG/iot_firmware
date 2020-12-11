#include <algorithm>
#include "JsonValue.h"

string trimend(const string& str)
{
    unsigned long end = str.find_first_of(" \f\n\r\t\v");
    return str.substr(0, end);
}

static bool match_no_case(string str1, string str2) {
    // make str1 and str2 upper-casae
    transform(str1.begin(), str1.end(), str1.begin(), ::toupper);
    transform(str2.begin(), str2.end(), str2.begin(), ::toupper);
    return (str1.compare(str2)==0)?true:false;
}

JsonValue::JsonValue() {
}

/*
 * create a deep clone of the specified json value
 */
JsonValue::JsonValue(const JsonValue &val) {
    value = val.value;
}

/**
 * constructor - create the value and initialize it from the provided string.
 */
JsonValue::JsonValue(string value) {
    if (match_no_case(value,"null")) {
        this->value = "NULL";
    }
    else {
        this->value = value;
    }
}

/*
 * create a deep clone of this object
 */
JsonAbstractValue * JsonValue::copy() {
    JsonValue* val = new JsonValue;
    val->value = value;
    return val;
}

/**
 * diagnostic function to dump the value to the system output device
 */
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
void JsonValue::dump(ostream& out, bool pretty) {
    dump(out, pretty, 0, true);
}

/**
 * return the value as a string
 */
string JsonValue::getValue(string specifier) {
    if (specifier == "") {
        return value;
    }
    return "";
}

/**
 * return the value as an integer
 */
long JsonValue::getInteger(string specifier) {
    if (specifier == "") {
        return atol(value.c_str());
    }
    return 0;
}

/**
 * return the value as a double
 */
double  JsonValue::getDouble(string specifier) {
    if (specifier == "") {
        return atof(value.c_str());
    }
    return 0.0;
}

/**
 * return the value as a bool
 */
bool JsonValue::getBoolean(string specifier) {
    if (specifier == "") {
        if (match_no_case(value,"true")) return true;
    }
    return false;
}

/**
 * return as a JSON handle represented as a string
 */
string  JsonValue::getHandle(string specifier) {
    return getValue(specifier);
};
