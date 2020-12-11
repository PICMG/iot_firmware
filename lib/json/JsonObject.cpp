#include "JsonObject.h"

JsonObject::JsonObject() {
}

/*
* create a deep clone of the specified json object
*/
JsonObject::JsonObject(const JsonObject &obj) {
    // iterate through each element in the map
    for (jsonmap::iterator it = map.begin(); it != map.end(); ++it) {
        string key = it->first;
        JsonAbstractValue &value = *(it->second);
        
        // erase exsiting entry if it exists (this shouldnt happen)
        jsonmap::iterator it2 = map.find(key);
        if (it2 != map.end()) {
            delete it2->second;
            map.erase(it2);
        }
        // add the new entry
        map.insert(pair<string, JsonAbstractValue*>(key,it->second->copy()));
    }
}

// destructor - delete any dynamically allocated memory associated with
// this object.
JsonObject::~JsonObject() {
    for (jsonmap::iterator it = map.begin(); it != map.end(); ++it)
        delete it->second;
}

// return a pointer to a deep copy of this object (uses copy constructor)
JsonAbstractValue* JsonObject::copy() {
    return (JsonAbstractValue*) new JsonObject(*this);
}

void JsonObject::put(string key, JsonAbstractValue* val) {
    jsonmap::iterator it2 = map.find(key);
    if (it2 != map.end()) {
        delete it2->second;
        map.erase(it2);

        // add the new entry
        map.insert(pair<string, JsonAbstractValue*>(key, val));
    }
    else {
        // add the new entry and map index
        map.insert(pair<string, JsonAbstractValue*>(key, val));
        index.push_back(key);
    }
}

/*
* diagnostic function - dumps the contents of the json object to the system output device
*/
void JsonObject::dump(ostream& out, bool pretty, int indent, bool useIndent) {
    if ((useIndent)&&(pretty))  for (int i = 0;i < indent;i++) out<<" ";
    out << "{";
    if (pretty) out<< endl;

    for (jsonmapindex::iterator it = index.begin(); it != index.end(); ++it) {
        if (pretty) for (int i = 0;i < indent + 3;i++) out<<" ";
        string key = map.find(it->data())->first;
        JsonAbstractValue& value = *map.find(it->data())->second;

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
void JsonObject::dump(ostream& out, bool pretty) {
    dump(out, pretty, 0, true);
}

/*
* returns a string value of the specified element where the specifier
* is of the form:
* 		Empty - return the entire object
* 		key
*/
string JsonObject::getValue(string specifier) {
    if (map.empty()) return "";

    string result = "";
    if (specifier == "") {
        // The specifier is empty - return values for all records in the
        // object (this should not be normal)
        for (jsonmap::iterator it = map.begin(); it != map.end(); ++it) {
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
    jsonmap::iterator it = map.find(specifier);
    if (it != map.end()) return it->second->getValue("");
    return "";
}

    
/*
* returns a bool value of the specified element where the specifier
* is of the form:
* 		Empty - return the entire object
* 		key
* 		key[index].specifier
* 		key.specifier
*/
bool JsonObject::getBoolean(string specifier) {
    if (map.empty()) return "";
    if (specifier == "") return false;

    // the specifier as the key
    jsonmap::iterator it = map.find(specifier);
    if (it != map.end()) return it->second->getBoolean("");
    return false;
}

    
/*
* returns a handle where the specifier is of the form:
* 		key
*/
string JsonObject::getHandle(string specifier) {
    if (map.empty()) return "";
    if (specifier == "") return "";

    // the specifier as the key
    jsonmap::iterator it = map.find(specifier);
    if (it != map.end()) return it->second->getHandle("");
    return "NULL";
}

    
/*
* returns an integer value of the specified element where the specifier
* is of the form:
* 		key
*/
long JsonObject::getInteger(string specifier) {
    if (map.empty()) return 0;
    if (specifier == "") return 0;

    // the specifier as the key
    jsonmap::iterator it = map.find(specifier);
    if (it != map.end()) return it->second->getInteger("");
    return 0;
}

    
/*
* returns a double value of the specified element where the specifier
* is of the form:
* 		key
*/
double JsonObject::getDouble(string specifier) {
    if (map.empty()) return 0.0;
    if (specifier == "") return 0.0;

    // the specifier as the key
    jsonmap::iterator it = map.find(specifier);
    if (it != map.end()) return it->second->getDouble("");
    return 0.0;
}

JsonAbstractValue* JsonObject::find(string key) {
    return map.find(key)->second;
}
    
// return the size of the json object
unsigned long JsonObject::size() {
    return map.size();
}

// return a specific indexed element from the array
string JsonObject::getElementKey(unsigned long idx) {
    if (idx >= index.size()) return NULL;

    // attempt to find the indexed item in the array
    jsonmapindex::iterator it = index.begin();
    for (unsigned int i = 0; i < idx; i++) it++;
    
    // return the result
    return it->data();
}