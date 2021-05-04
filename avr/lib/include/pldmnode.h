//*******************************************************************
//    PldmNode.h
//
//    This file provides definition for an abstract base class for 
//    PLDM connector nodes.  The node represents an interface between
//    the PLDM protocol and lower network layers.
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
#include "pldm.h"

class PldmNode
{
public:
	// send a pldm request through the connector node
	virtual void putCommand(PldmRequestHeader* hdr, unsigned char* command, unsigned int size) = 0;
	
	// receive a response from the connector node
	virtual unsigned char* getResponse(void) = 0;
};

