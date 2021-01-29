//*******************************************************************
//    PldmEntity.h
//
//    This file includes definitions for a class representation of a
//    single Pldm entity.  When the pldm client constructs the central 
//    Pdr repository, it extracts entity association information from 
//    the PDRs in order to build the structure of all connected entities.
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
#include <list>
#include <string>
#include "GenericPdr.h"
#include "PdrRepository.h"

using namespace std;

class PldmEntity {
private:
    list<PldmEntity *> children;
    list<GenericPdr*>  pdrs;                 // pdrs associated with this entity
    GenericPdr * pdr;                        // a pointer to the pdr associated with this entity
    string       name;                       // the display name for this entity
    unsigned int tid;                        // the Terminus ID associated with this entity
    unsigned int containerId;                // the ID of the container that contains this entity
    PldmEntity*  parent;                     // a pointer to the parent entity
    bool         logicalAssociation;
    unsigned int containerEntityContainerId;
    unsigned int branchInstance;
    unsigned int containerEntityInstanceNumber;
    unsigned int containerEntityType;
public:
    PldmEntity();
    PldmEntity(GenericPdr & pdr, PdrRepository &repository, uint8 tid);
    ~PldmEntity();
    bool attach(PdrRepository &repository, uint8 tid);
    bool attach(GenericPdr &entity_association, PdrRepository & repository, uint8 tid);
    bool link(PdrRepository &repository, uint8 tid);
    bool link(GenericPdr &entity_association, PdrRepository & repository, uint8 tid);
    bool attachChildren(GenericPdr &entity_association, PdrRepository & repository, uint8 tid);
    bool getPdrFromURI(string uri, GenericPdr *&pdr, unsigned int &tid);

    bool setName(string name);
    bool removeChild();
    void dump();
};