//*******************************************************************
//    JsonArray.h
//
//    This file provides definition for a class that represents
//    a JSON arrays (json content enclosed in square bracket characters).
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
#pragma once
#include "JsonAbstractValue.h"
#include "JsonObject.h"

class JsonArray :
	public JsonAbstractValue
{
    // internal representation
    typedef map<unsigned int, JsonAbstractValue*> jsonarray;
    jsonarray internal_map;   // the map of array values
public:
    // construction/destruction
    JsonArray();
    JsonArray(const JsonArray&);
    ~JsonArray();

    // array manipulation
    void            add(JsonAbstractValue* val);
    unsigned long   size();  // return the number of elements in the array
    JsonAbstractValue* getElement(unsigned long index); // return a specific element in the array
    
    // deep copy
    virtual JsonAbstractValue* copy();
    
    // visualization
    virtual void    dump(ostream& out, bool pretty, int indent, bool useIndent);
    virtual void    dump(ostream& out, bool pretty);
    
    // get value
    virtual string  getValue(string specifier);
    virtual long    getInteger(string specifier);
    virtual double  getDouble(string specifier);
    virtual bool    getBoolean(string specifier);
    virtual string  getHandle(string specifier);
};

