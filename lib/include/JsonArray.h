#pragma once
#include "JsonAbstractValue.h"
#include "JsonObject.h"

class JsonArray :
	public JsonAbstractValue
{
    typedef map<unsigned int, JsonAbstractValue*> jsonarray;
    jsonarray map;
public:
    JsonArray();
    JsonArray(const JsonArray&);
    ~JsonArray();

    void            add(JsonAbstractValue* val);
    virtual JsonAbstractValue* copy();
    virtual void    dump(ostream& out, bool pretty, int indent, bool useIndent);
    virtual void    dump(ostream& out, bool pretty);
    virtual string  getValue(string specifier);
    virtual long    getInteger(string specifier);
    virtual double  getDouble(string specifier);
    virtual bool    getBoolean(string specifier);
    virtual string  getHandle(string specifier);

    unsigned long   size();  // return the number of elements in the array
    JsonAbstractValue* getElement(unsigned long index); // return a specific element in the array
};

