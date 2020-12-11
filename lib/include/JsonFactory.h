#pragma once
#include "JsonAbstractValue.h"
#include "JsonValue.h"
#include "JsonObject.h"
#include "JsonArray.h"

class JsonFactory
{
private:
    unsigned long strpos;
    string str;


    void   skipWhitespace();
    string getstring();
    string getRaw();
    JsonAbstractValue* builder();
public:
    JsonFactory();
    JsonAbstractValue* build(string str);
};

