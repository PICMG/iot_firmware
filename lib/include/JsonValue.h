#pragma once
#include "JsonAbstractValue.h"
class JsonValue :
	public JsonAbstractValue
{
private:
    string value;
public:
    JsonValue();
    JsonValue(const JsonValue& val);
    JsonValue(string value);

    virtual JsonAbstractValue* copy();
    virtual void    dump(ostream& out, bool pretty, int indent,bool useIndent);
    virtual void    dump(ostream& out, bool pretty);
    virtual string  getValue(string specifier);
    virtual long    getInteger(string specifier);
    virtual double  getDouble(string specifier);
    virtual bool    getBoolean(string specifier);
    virtual string  getHandle(string specifier);
};

