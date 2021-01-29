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
#include "PldmEntity.h"

//===================================================================
// PldmEntity
//
// Constructor - initialize internal representation
PldmEntity::PldmEntity() {
    pdr  = 0;
    name = "System";
    tid  = 0;
    containerId = 0;
    parent = 0;
    logicalAssociation = false;
    containerEntityContainerId = 0;
    branchInstance = 0;
    containerEntityInstanceNumber = 1;
    containerEntityType = 0;
} 

//===================================================================
// PldmEntity
//
// Constructor - construct a PldmEntity from the given PDR.  The
// PDR must be an EntityAssociation PDR for this function to work
//
// parameters:
//   pdr - the pdr to initialize from
//
PldmEntity::PldmEntity(GenericPdr &pdr, PdrRepository &repository, uint8 tid) {
    if (pdr.getPdrType()!=PDR_TYPE_ENTITY_ASSOCIATION) return;
    
    this->pdr  = &pdr;
    name = "";
    this->tid  = tid;
    containerId = atoi(pdr.getValue("containerID").c_str());
    parent = 0;
    containerEntityContainerId = atoi(pdr.getValue("containerEntityContainerID").c_str());
    containerEntityInstanceNumber = atoi(pdr.getValue("containerEntityInstanceNumber").c_str());
    branchInstance = containerEntityInstanceNumber;
    containerEntityType = atoi(pdr.getValue("containerEntityType").c_str());
    logicalAssociation = (pdr.getValue("associationType")=="logicalContainment");
    name = repository.getEntityTypeString(containerEntityType&0x7fff);

    // create any children 
    int containedEntityCount = atoi(pdr.getValue("containedEntityCount").c_str());
    for (int i = 1;i <= containedEntityCount; i++) {
        PldmEntity *child = new PldmEntity;
        child->tid  = tid;
        parent = this;
        string key = "containedEntityContainerID[" + to_string(i) + "]";
        child->containerEntityContainerId =    atoi(pdr.getValue(key.c_str()).c_str());
        child->containerId = child->containerEntityContainerId;
        key = "containedEntityInstanceNumber[" + to_string(i) + "]";
        child->containerEntityInstanceNumber = atoi(pdr.getValue(key.c_str()).c_str());
        child->branchInstance = child->containerEntityInstanceNumber;
        key = "containedEntityType[" + to_string(i) + "]";
        child->containerEntityType =           atoi(pdr.getValue(key.c_str()).c_str());
        child->name = repository.getEntityTypeString(child->containerEntityType&0x7fff);
        children.push_back(child);
    }
} 

//===================================================================
// ~PldmEntity()
//
// Destructor - free memory associated with this entity
PldmEntity::~PldmEntity() {
    // delete memory associated with the children, do not delete memory
    // from PDR entries because these are deallocated with the PdrRepository
    while (children.size()!=0) {
        PldmEntity * child = *children.begin();
        children.pop_front();
        delete child;
    }
}

//===================================================================
// attach()
//
// attempt to attach the entities associated with the specific PDR 
// repository.
//
// parameters:
//   repository - the pdr repository to build from
//   tid - the terminus id that this PDR was read from
// returns:
//   true if the pdr was attached, otherwise false 
bool PldmEntity::attach(PdrRepository &repository, uint8 tid) {
    bool added = true;
    while (added == true) {
        added = false;
        unsigned int recordNumber = 1;
        GenericPdr *pdr = repository.getPdrFromRecordHandle(recordNumber);
        while (pdr) {
            added |= attach(*pdr, repository, tid);
            recordNumber++;
            pdr = repository.getPdrFromRecordHandle(recordNumber);
        }
    }
    return added;
}

//===================================================================
// link()
//
// attempt to link the pdrs associated with the specific PDR 
// repository.
//
// parameters:
//   repository - the pdr repository to build from
//   tid - the terminus id that this PDR was read from
// returns:
//   true if pdrs were attached, otherwise false 
bool PldmEntity::link(PdrRepository &repository, uint8 tid) {
    bool result = false;
    unsigned int recordNumber = 1;
    GenericPdr *pdr = repository.getPdrFromRecordHandle(recordNumber);
    while (pdr) {
        result |= link(*pdr, repository, tid);
        recordNumber++;
        pdr = repository.getPdrFromRecordHandle(recordNumber);
    }
}

//===================================================================
// link()
//
// link the association of a given pdr with this entity
//
// parameters:
//   pdr - the pdr to attempt to link
//   tid - the terminus id that this PDR was read from
// returns:
//   true if the pdr was linked, otherwise false 
bool PldmEntity::link(GenericPdr &pdr, PdrRepository &repository, uint8 tid) {
    // make sure the pdr can be linked - it must be a sensor/effecter/fru record
    if ((pdr.getPdrType()!=PDR_TYPE_STATE_SENSOR) && 
        (pdr.getPdrType()!=PDR_TYPE_NUMERIC_SENSOR) &&
        (pdr.getPdrType()!=PDR_TYPE_NUMERIC_EFFECTER) &&
        (pdr.getPdrType()!=PDR_TYPE_STATE_EFFECTER)) return false;

    uint16 containerId = atoi(pdr.getValue("containerID").c_str());
    uint16 entityInstanceNumber = atoi(pdr.getValue("entityInstanceNumber").c_str());
    uint16 entityType = atoi(pdr.getValue("entityType").c_str());
    if ((atoi(pdr.getValue("containerID").c_str())==this->containerId) && 
        (atoi(pdr.getValue("entityInstanceNumber").c_str())==this->containerEntityInstanceNumber) &&
        (atoi(pdr.getValue("entityType").c_str())== this->containerEntityType) && (this->tid == tid) ) {
        // here if the pdr is associated with this entity - add it
        pdrs.push_back(&pdr);
        return true;        
    }

    // the pdr does not attach to this entity - check the children
    for (list<PldmEntity *>::iterator it = children.begin(); it != children.end(); ++it) {
        if ((*it)->link(pdr,repository,tid)) return true;
    }
    return false; 
}

//===================================================================
// attach()
//
// attempt to attach the entity associated with the specific PDR to
// this entity.
//
// parameters:
//   pdr - the pdr to attempt to attach
//   tid - the terminus id that this PDR was read from
// returns:
//   true if the pdr was attached, otherwise false 
bool PldmEntity::attach(GenericPdr &pdr, PdrRepository &repository, uint8 tid) {
    // make sure the pdr is an entity association.  If not, return
    // without attempting to attach 
    if (pdr.getPdrType()!=PDR_TYPE_ENTITY_ASSOCIATION) return false;

    // Special case if this node is the system node and the entity association connects
    // to the system node 
    if (((atoi(pdr.getValue("containerEntityContainerID").c_str()))==0)&&(this->containerId==0)) {
        // this is the system node an the containerEntity of the entity association connects
        // to the system - add the entity as a child if it does not already exist.
        int instanceNumber = 1;
        int type = atoi(pdr.getValue("containerEntityType").c_str())&0x7fff;
        for (list<PldmEntity *>::iterator it = children.begin(); it != children.end(); ++it) {
            PldmEntity * child = (*it);
            if (type == child->containerEntityType&0x7fff) instanceNumber++;
            if (child->tid != tid) continue;
            string val = pdr.getValue("containerEntityContainerID");
            if (child->containerEntityContainerId != atoi(pdr.getValue("containerEntityContainerID").c_str())) continue;
            if (child->containerEntityInstanceNumber != atoi(pdr.getValue("containerEntityInstanceNumber").c_str())) continue;
            if (child->containerEntityType != atoi(pdr.getValue("containerEntityType").c_str())) continue;
            if (child->logicalAssociation != (pdr.getValue("associationType")=="logicalContainment")) continue;

            child->name = repository.getEntityTypeString(child->containerEntityType&0x7fff);
            
            // a match has been found - add children to the node
            return child->attachChildren(pdr,repository, tid);
        }

        // a match has not been found - create a new child entity for this association       
        PldmEntity * child = new PldmEntity(pdr,repository,tid);
        child->parent = this;
        child->tid = tid;
        child->branchInstance = instanceNumber;
        this->children.push_back(child);        

        // children were automatically created, so just return
        return true;
    }

    // all other cases, associations should make a match between the contianer information
    // already in the tree, and the container information in the entity association.
    // This is because, the node should have already been added to the tree as a child of
    // a previous node.
    if (this->tid != tid) return false;   // the terminus id does not even match no need to check further
    uint16 containerId = atoi(pdr.getValue("containerEntityContainerID").c_str());
    uint16 entityInstanceNumber = atoi(pdr.getValue("containerEntityInstanceNumber").c_str());
    uint16 entityType = atoi(pdr.getValue("containerEntityType").c_str());
    if ((atoi(pdr.getValue("containerEntityContainerID").c_str())==this->containerEntityContainerId)&&
        (atoi(pdr.getValue("containerEntityInstanceNumber").c_str())== this->containerEntityInstanceNumber)&&
        (atoi(pdr.getValue("containerEntityType").c_str())== this->containerEntityType)) {

        // add missing entity information
        this->pdr = &pdr;
        this->containerId = atoi(pdr.getValue("containerID").c_str());

        // add children of this association
        return attachChildren(pdr,repository,tid);
    }

    // final case - the association does not directly relate to this entity - recurse through
    // the children to see if it applies there
    for (list<PldmEntity *>::iterator it = children.begin(); it!=children.end(); ++it) {
        if ((*it)->attachChildren(pdr,repository, tid)) return true;
    }
    return false; 
}

//===================================================================
// attachChildren()
//
// examine the entity association in the given GenericPdr.  If child
// entities are found that are not in this entity's current list of
// children, create and add them to this entity's list of children
//
// parameters:
//   pdr - the pdr to attempt to attach
//   tid - the terminus id that this PDR was read from
// returns:
//   true if any children from the pdr were attached, otherwise false 
bool PldmEntity::attachChildren(GenericPdr &pdr, PdrRepository &repository, uint8 tid) {
    bool result = false;
    int containedEntityCount = atoi(pdr.getValue("containedEntityCount").c_str());
    for (int i = 1;i <= containedEntityCount; i++) {
        string key = "containedEntityContainerID[" + to_string(i) + "]";
        unsigned int child_containerId = atoi(pdr.getValue(key.c_str()).c_str());
        key = "containedEntityInstanceNumber[" + to_string(i) + "]";
        unsigned int child_InstanceNumber = atoi(pdr.getValue(key.c_str()).c_str());
        key = "containedEntityType[" + to_string(i) + "]";
        unsigned int child_EntityType =           atoi(pdr.getValue(key.c_str()).c_str());         
        
        // loop to check each child for a match
        bool matchFound = false;
        for (list<PldmEntity *>::iterator it = children.begin(); it!=children.end(); ++it) {
            PldmEntity *child = (*it);
            if ((child_containerId == child->containerEntityContainerId)&&
               (child_InstanceNumber == child->containerEntityInstanceNumber)&&
               (child_EntityType == child->containerEntityType)) {
                // a match has been found for this child.
                matchFound = true;
                break;
            }
        }
        if (!matchFound) {
            // here if there was no match - create a new child and add it
            // to the list
            PldmEntity * child = new PldmEntity();
            child->containerEntityContainerId = child_containerId;
            child->containerEntityInstanceNumber = child_InstanceNumber;
            child->branchInstance = child->containerEntityInstanceNumber;
            child->containerEntityType = child_EntityType;
            child->parent = this;
            child->tid = tid;
            child->name = repository.getEntityTypeString(child->containerEntityType&0x7fff);
            children.push_back(child);
            result = true;
        }
    }
    return result;
}

static string getPdrURIPart(GenericPdr *pdr) {
    string name="";
    switch (pdr->getPdrType()) {
    case PDR_TYPE_STATE_SENSOR:
        name = "StateSensor_";
        name += pdr->getValue("sensorID");
        break;
    case PDR_TYPE_NUMERIC_SENSOR:
        name = "NumericSensor_";
        name += pdr->getValue("sensorID");
        break;
    case PDR_TYPE_NUMERIC_EFFECTER:
        name = "NumericEffecter_";
        name += pdr->getValue("effecterID");
        break;
    case PDR_TYPE_STATE_EFFECTER:
        name = "StateEffecter_";
        name += pdr->getValue("effecterID");
        break;
    default:
        break;
    }
    return name;
}

//*******************************************************************
// getPdrFromURI()
//
// given a dot notation of an object, get the associated PDR and
// terminus id.  If a match is found, return true, otherwise,
// return false.
//
// uri parameters should be of the form:
//   part1/part2/part3 ...
//
// parameters:
//    uri - a string that contains the URI
//    pdr - on return the pointer will be updaed to point to the 
//          pdr that matches the uri.  If no match is found, the 
//          pointer will not be altered.
//    tid - on return this will be updated to point to the
//          terminus number that matches the uri.  If no match
//          is found, the value will not be altered.
// returns:
//    true if a match is found, otherwise false.
bool PldmEntity::getPdrFromURI(string uri, GenericPdr *&pdr, unsigned int & tid ) {
    // extract the first part and remaining part of the uri
    std::size_t found = uri.find_first_of("/");
    string first_part = uri.substr(0,found);
    string remaining = uri.substr(found+1, string::npos);

    // check to see if the first part of the uri matches this node
    if ((this->containerId == 0)&&(this->containerEntityType==0)) {
        if (first_part != "System") return false;
    } 
    else {
        string node_name = name + "_" + to_string(branchInstance);
        if (first_part != node_name) return false;
    }

    // here if the node name matches - check children if there
    // there is still a backslash character in the remaining uri
    found = (remaining.find_first_of("/"));
    if (found != string::npos) {
        // there are still more parts to the URI - recurse the
        // children to find a match
        for (list<PldmEntity*>::iterator it = children.begin(); it != children.end(); ++it) {
            if ((*it)->getPdrFromURI(remaining, pdr, tid)) return true;
        }
        // here if no match is found
        return false;
    }

    // here if there are no more slashes in the URI - check 
    // the remaining name against the names of the children 
    for (list<GenericPdr *>::iterator it = pdrs.begin(); it != pdrs.end(); ++it) {
        GenericPdr *childpdr = *it;
        string name = getPdrURIPart(childpdr);
        if (remaining == name) {
            pdr = childpdr;
            tid = this->tid;
            return true;
        }
    }
    return false;
}


void PldmEntity::dump() {
    static string indentstring = "";
    const char* elbow  = "\u255a";
    const char* tee    = "\u2560";
    const char* updown = "\u2551";
    const char* leftright = "\u2550";

    if ((this->containerId == 0)&&(this->containerEntityType==0)) {  
        // root node
        cout<<this->name<<endl;
        indentstring = "   ";
    }

    // for each child in the list
    string indentsave = indentstring;
    for (list<PldmEntity *>::iterator it = children.begin(); it != children.end(); ++it) {
        PldmEntity *child = *it;
        list<PldmEntity *>::iterator it2 = it;
        if ((++it2)==children.end()) {
            // last entity in the list
            cout<<indentstring<<elbow<<leftright;
            cout<<child->name<<"_"<<child->branchInstance<<", tid "<<child->tid<<endl;
            indentstring = indentstring + "    ";
        } 
        else {
            cout<<indentstring<<tee<<leftright;
            cout<<child->name<<"_"<<child->branchInstance<<", tid "<<child->tid<<endl;
            indentstring = indentstring + updown + "   ";
        }
        child->dump();
        indentstring = indentsave;
    }

    // for each pldm linked to this node
    indentstring = indentstring;
    for (list<GenericPdr *>::iterator it = pdrs.begin(); it != pdrs.end(); ++it) {
        GenericPdr *pdr = *it;
        list<GenericPdr*>::iterator it2 = it;
        // get the information to print
        string name = getPdrURIPart(pdr);
        if ((++it2)==pdrs.end()) {
            // last pdr in the list
            cout<<indentstring<<elbow<<leftright;
        } 
        else {
            cout<<indentstring<<tee<<leftright;
        }
        cout<<name<<endl;
    }
    indentstring = indentsave;
    
}
