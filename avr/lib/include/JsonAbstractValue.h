//*******************************************************************
//    JsonAbstractValue.h
//
//    This file provides definition of a pure virtual base class for
//    Json values. This header is intended to be used as part of 
//    the PICMG IoT library reference code. 
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
#include <string>
#include <map>
#include <iostream>

using namespace std;

class JsonAbstractValue
{
public:
    // deep copy of the value
    virtual JsonAbstractValue* copy() = 0;

    // visualization
    virtual void    dump(ostream& out, bool pretty, int indent, bool useIndent) = 0;
    virtual void    dump(ostream& out, bool pretty) = 0;

    // return the value as a string
    virtual string  getValue(string specifier) = 0;   

    // return the value as a long signed integer
    virtual long    getInteger(string specifier) = 0;

    // return the value as a double-precision floating-point
    virtual double  getDouble(string specifier) = 0;

    // return the value as a boolean
    virtual bool    getBoolean(string specifier) = 0;

    // return the handle value as a string
    virtual string  getHandle(string specifier) = 0;
};
