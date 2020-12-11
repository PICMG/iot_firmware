#include "JsonArray.h"

// Default constructor
JsonArray::JsonArray() {

}

/*
 * Copy Constructor: create a deep clone of the specified object
 */
JsonArray::JsonArray(const JsonArray &ary) {
    for (jsonarray::iterator it = map.begin(); it != map.end(); ++it) {
        unsigned long key = it->first;
        JsonAbstractValue& value = *(it->second);

        // erase exsiting entry if it exists (this shouldnt happen)
        jsonarray::iterator it2 = map.find(key);
        if (it2 != map.end()) {
            delete it2->second;
            map.erase(it2);
        }
        // add the new entry
        map.insert(pair<unsigned long, JsonAbstractValue*>(key, it->second->copy()));
    }
}

// destructor
JsonArray::~JsonArray() {
    for (jsonarray::iterator it = map.begin(); it != map.end(); ++it) 
        delete it->second;
}

// return a pointer to a deep copy of this object (uses copy constructor)
JsonAbstractValue* JsonArray::copy() {
    return (JsonAbstractValue*) new JsonArray(*this);
}

void JsonArray::add(JsonAbstractValue *val) {
    map.insert(pair<unsigned int, JsonAbstractValue*>(map.size(), val));
}

/*
* diagnostic function - dumps the contents of the json array to the system output device
*/
void JsonArray::dump(ostream& out, bool pretty, int indent, bool useIndent) {
    out << "[";
    if (pretty) cout<<endl;
    bool firstElement = false;
    for (jsonarray::iterator it = map.begin(); it != map.end(); ++it) {
        it->second->dump(out, pretty, indent+3,true);
        jsonarray::iterator itnext = it;
        itnext++;
        if (itnext != map.end()) {
            out << ",";
        }
        if (pretty) out << endl;
    }
    if (pretty) for (int i = 0;i < indent;i++) out<<" ";
    out<<"]";
}

void JsonArray::dump(ostream& out, bool pretty) {
    dump(out, pretty, 0, true);
}

/*
* returns a string value of the specified element where the specifier
* is of the form [index].specifier
*/
string JsonArray::getValue(string specifier) {
    if (map.empty()) return "";

    if (specifier=="") {
        // The specifier is empty - return values for all records in the
        // object (this should not be normal)
        string result;
        for (jsonarray::iterator it = map.begin(); it != map.end(); ++it) {
            result.append(it->second->getValue(""));
            result.append(", ");
        }
        return result;
    }
    // use the leftmost part of the specifier as the key
    string index = specifier.substr(1, specifier.find("]") - 1);
    string spec2 = specifier.substr(specifier.find(".") + 1, specifier.length() - specifier.find(".") - 1);
    jsonarray::iterator it = map.find(atol(index.c_str()));
    if (it != map.end()) {
        return it->second->getValue(spec2);
    }
    return "";
}

/*
* returns a bool value of the specified element where the specifier
* is of the form [index].specifier
*/
bool JsonArray::getBoolean(string specifier) {
    if (specifier == "") return false;
    // use the leftmost part of the specifier as the key
    string index = specifier.substr(1, specifier.find("]") - 1);
    string spec2 = specifier.substr(specifier.find(".") + 1, specifier.length() - specifier.find(".") - 1);
    jsonarray::iterator it = map.find(atol(index.c_str()));
    if (it != map.end()) {
        return it->second->getBoolean(spec2);
    }
    return "";
}


/*
* returns the object handle of the specified element where the specifier
* is of the form [index].specifier
*/
string JsonArray::getHandle(string specifier) {
    if (specifier == "") return "";
    // use the leftmost part of the specifier as the key
    string index = specifier.substr(1, specifier.find("]") - 1);
    string spec2 = specifier.substr(specifier.find(".") + 1, specifier.length() - specifier.find(".") - 1);
    jsonarray::iterator it = map.find(atol(index.c_str()));
    if (it != map.end()) {
        return it->second->getHandle(spec2);
    }
    return "";
}


/*
* returns the integer value of the specified element where the specifier
* is of the form [index].specifier
*/
long JsonArray::getInteger(string specifier) {
    if (specifier == "") return 0;
    // use the leftmost part of the specifier as the key
    string index = specifier.substr(1, specifier.find("]") - 1);
    string spec2 = specifier.substr(specifier.find(".") + 1, specifier.length() - specifier.find(".") - 1);
    jsonarray::iterator it = map.find(atol(index.c_str()));
    if (it != map.end()) {
        return it->second->getInteger(spec2);
    }
    return 0;
}


/*
* returns the double value of the specified element where the specifier
* is of the form [index].specifier
*/
double JsonArray::getDouble(string specifier) {
    if (specifier == "") return 0.0;
    // use the leftmost part of the specifier as the key
    string index = specifier.substr(1, specifier.find("]") - 1);
    string spec2 = specifier.substr(specifier.find(".") + 1, specifier.length() - specifier.find(".") - 1);
    jsonarray::iterator it = map.find(atol(index.c_str()));
    if (it != map.end()) {
        return it->second->getDouble(spec2);
    }
    return 0.0;
}

// return the size of the array
unsigned long JsonArray::size() {
    return map.size();
}

// return a specific indexted element from the array
JsonAbstractValue* JsonArray::getElement(unsigned long index) {
    // attempt to find the indexed item in the array
    jsonarray::iterator it = map.find(index);

    // return the result if found, otherwise, null
    if (it != map.end()) return it->second;
    return NULL;
}

#ifdef NOT_DEFINED
/*
* returns true if the array contains any of the elements in the
* specified parameter
*/
bool JsonArray::containsAny(JsonArray ary) {
    if (ary == null) return false;
    if (ary.isEmpty()) return false;
    for (int i = 0;i < ary.size();i++) {
        for (int j = 0;j < this.size();j++) {
            if (this.get(j).getValue("").equals(ary.get(i).getValue(""))) return true;
        }
    }
    return false;
}

/*
* returns true if the array values contain any elements whose values
* match the specified string array
*/
bool JsonArray::containsAny(ArrayList<string> ary) {
    if (ary == null) return false;
    if (ary.isEmpty()) return false;
    for (int i = 0;i < ary.size();i++) {
        for (int j = 0;j < this.size();j++) {
            if (this.get(j).getValue("").equals(ary.get(i))) return true;
        }
    }
    return false;
}

/*
* find an object in the array that contains a key value pair
* that matches the specified parameters.
*/
JsonObject JsonArray::findMatching(string key, string value) {
    for (int i = 0;i < this.size();i++) {
        if (this.get(i).getClass().isAssignableFrom(JsonObject.class)) {
            // here if the current array element is a json object
            JsonObject jo = (JsonObject)this.get(i);
            if (jo.containsKey(key) && (jo.getValue(key).equals(value))) {
                // return the matching object
                return jo;
            }
        }
    }
    return null;
}
#endif
