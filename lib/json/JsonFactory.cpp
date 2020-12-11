#include "JsonFactory.h"

// return a string that is equivalent to the starting string with leading whitespace removed
string trim(const string & str)
{
    unsigned long start = str.find_first_not_of(" \f\n\r\t\v");
    return str.substr(start, str.length() - start);
}

/* helper function to skip past white space */
void JsonFactory::skipWhitespace() {
    while (strpos < str.length()) {
        if (str[strpos] <= ' ') {
            strpos++;
        }
        else {
            break;
        }
    }
}

/*
* helper function used by builder to extract a double quoted string
*/
string JsonFactory::getstring() {
    if (str[strpos] != '\"') return "";
    strpos++;
    int start = strpos;
    bool ignoreNext = false;
    while (strpos < str.length()) {
        if ((str[strpos] == '\"') && (!ignoreNext)) {
            string result = str.substr(start, strpos-start);
            strpos++;
            return result;
        }
        ignoreNext = false;
        if (str[strpos] == '\\') ignoreNext = true;
        strpos++;
    }
    return "";
}

/*
* helper function used by builder to extract a string that is delimited by
* JSON ending delimiters.
*/
string JsonFactory::getRaw() {
    int start = strpos;
    while (strpos < str.length()) {
        if ((str[strpos] == ',') || (str[strpos] == '}') ||
            (str[strpos] == ']')) {
            string result = str.substr(start, strpos-start);
            return result;
        }
        strpos++;
    }
    return "";
}

JsonFactory::JsonFactory() : strpos(0) {
}

/**
* entry point for the builder.  Builds a JsonAbstractValue based on the input string
* @param str - the JSON formatted string that specifies the structure to build
* @return A JsonAbstractValue structure that matches the input string
*/
JsonAbstractValue *JsonFactory::build(string str) {
    // trim leading and trailing whitespace
    this->str = trim(str);
    strpos = 0;
    return builder();
}

/*
* helper class to build the JsonAbstractValue
*/
JsonAbstractValue * JsonFactory::builder() {
    if (str == "") return NULL;

    if (str[strpos] == '[') {
        // here if the string represents a json array
        strpos++;
        skipWhitespace();

        // here if we need to create a value set (json array)
        JsonArray *cs = new JsonArray();
        while (strpos < str.length()) {
                // create and build the object or string
            JsonAbstractValue* obj = builder();
            if (obj == NULL) {
                cerr<<"null object returned at " <<strpos<<endl;
                return NULL;
            }
            cs->add(obj);

            // next character should either be a comma or an end brace
            skipWhitespace();
            if (strpos >= str.length()) {
                cerr<<"unexpected end of string"<<endl;
                return NULL;
            }
            if (str[strpos] == ']') break;
            if (str[strpos] == ',') strpos++;
            skipWhitespace();
        }
        skipWhitespace();
        if (str[strpos] != ']') {
            cerr<<"']' expected but none found at " << strpos<<endl;
            return NULL;
        }
        strpos++;
        return cs;
    }

    if (str[strpos] == '{') {
        // here if we need to create a json object
        strpos++;
        JsonObject *co = new JsonObject();

        skipWhitespace();

        // check for an empty object.
        if ((strpos < str.length() + 1) && (str[strpos] == '}')) {
            strpos += 2;
            return co;
        }

        while (strpos < str.length()) {
            string key = getstring();
            if (key == "") return NULL;
            if (strpos >= str.length()) {
                cerr<<"unexpected end of string"<<endl;
                return NULL;
            }

            skipWhitespace();
            if (str[strpos] != ':') {
                cerr<<"keyword separator expected.  None found"<<endl;
                return NULL;
            }
            strpos++;

            // create and build the value
            skipWhitespace();
            JsonAbstractValue *obj = builder();
            if (obj == NULL) return NULL;
            co->put(key, obj);

            // next character should either be a comma or an end brace
            skipWhitespace();
            if (strpos >= str.length()) {
                cerr<<"Unexpected end of string"<<endl;
                return NULL;
            }
            if (str[strpos] == '}') break;
            if (str[strpos] == ',') strpos++;
            skipWhitespace();
        }
        skipWhitespace();
        if (str[strpos] != '}') return NULL;
        strpos++;
        return co;
    }

    // here if the line is a value primitive
    if (str[strpos] == '"') {
        string s = getstring();
        if (s == "") {
            cerr<<"Null string returned"<<endl;
            return NULL;
        }
        JsonValue *cv = new JsonValue(s);
        skipWhitespace();
        return cv;
    }

    // here if the value primitive is not quoted
    string s = getRaw();
    if (s == "") {
        cerr << "Null raw value returned" << endl;
        return NULL;
    }
    JsonValue *cv = new JsonValue(s);
    skipWhitespace();
    return cv;
}

