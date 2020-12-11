#pragma once
#include <string>
#include <map>
#include <iostream>

using namespace std;

class JsonAbstractValue
{
public:
    virtual JsonAbstractValue* copy() = 0;
    virtual void    dump(ostream& out, bool pretty, int indent, bool useIndent) = 0;
    virtual void    dump(ostream& out, bool pretty) = 0;
    virtual string  getValue(string specifier) = 0;
    virtual long    getInteger(string specifier) = 0;
    virtual double  getDouble(string specifier) = 0;
    virtual bool    getBoolean(string specifier) = 0;
    virtual string  getHandle(string specifier) = 0;
};
