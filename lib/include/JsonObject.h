#pragma once
#include <map>
#include <list>
#include "JsonAbstractValue.h"

class JsonObject :
    public JsonAbstractValue
{
    typedef map<string, JsonAbstractValue*> jsonmap;
    typedef list<string> jsonmapindex;
    jsonmap map;
    jsonmapindex index;
public:
    JsonObject();
    JsonObject(const JsonObject& obj);
    ~JsonObject();

    void            put(string key, JsonAbstractValue* val);
    virtual JsonAbstractValue* copy();
    virtual void    dump(ostream& out, bool pretty, int indent, bool useIndent);
    virtual void    dump(ostream& out, bool pretty);
    virtual string  getValue(string specifier);
    virtual long    getInteger(string specifier);
    virtual double  getDouble(string specifier);
    virtual bool    getBoolean(string specifier);
    virtual string  getHandle(string specifier);

    unsigned long size();
    JsonAbstractValue* find(string key);
    string getElementKey(unsigned long index);
};

