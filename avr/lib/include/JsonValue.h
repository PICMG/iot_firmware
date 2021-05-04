//*******************************************************************
//    JsonValue.h
//
//    This file provides definitions for a class representation of
//    a concrete JSON value as opposed to an object or array.  This 
//    class represents the all JSON value types as a string.  
//    Getter functions interpret the string as specific data types.
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
class JsonValue :
	public JsonAbstractValue
{
private:
    string value;
public:
    // construction
    JsonValue();
    JsonValue(const JsonValue& val);
    JsonValue(string value);

    // deep copy
    virtual JsonAbstractValue* copy();
    
    // visualization
    virtual void    dump(ostream& out, bool pretty, int indent,bool useIndent);
    virtual void    dump(ostream& out, bool pretty);
    
    // get value in different representations
    virtual string  getValue(string specifier);
    virtual long    getInteger(string specifier);
    virtual double  getDouble(string specifier);
    virtual bool    getBoolean(string specifier);
    virtual string  getHandle(string specifier);
};

