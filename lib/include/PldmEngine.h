//*******************************************************************
//    PldmEngine.h
//
//    This file implements a client-side pldm engine that manages
//    a collection of endpoints and allows interaction with the 
//    endpoints through sending commands and receiving responses.
//
//    Portions of this code are based on the Platform Level Data Model
//    (PLDM) specifications from the Distributed Management Task Force 
//    (DMTF).  More information about PLDM can be found on the DMTF
//    web site (www.dmtf.org).
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
#include <map>
#include "JsonAbstractValue.h"
#include "JsonValue.h"
#include "JsonObject.h"
#include "JsonValue.h"
#include "GenericPdr.h"
#include "pldm.h"
#include "clientNode.h"
#include "PldmEntity.h"
#include "pldmnode.h"
#include "Terminus.h"

class PldmEngine {
    // internal representation
    PldmEntity pldmRoot;                 // root node of system entity map
    map<int, Terminus *> termini; // a map of all known termini in the system

    void sendGetTidCommand(clientNode &pldm_node);
    void sendSetTidCommand(clientNode &pldm_node, uint8 newTid);
    int waitForResponse(mctp_struct *mctp, unsigned char **response);
public:
    // construction and destruction
    PldmEngine();
    ~PldmEngine();

    // initialize the pldm engine and enumerate the endpoints
    bool init(string porttype);

    // display a map of PLDM endpoints to the specified output stream
    void showMap(ostream &out);

    // send a pldm command and await a response
    map<int, string> sendPldmCommand(string command, string address, map<int, string> &parameters);
};