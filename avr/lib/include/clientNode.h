//*******************************************************************
//    clientNode.h
//
//    This file includes definitions for a simulated PLDM connector node.
//    This node behaves like an extenal embedded device, that communicates
//    with PLDM commands and responses.   This is only intended for use 
//    in PLDM testing.
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
#include "mctp.h"
#include "pldm.h"
#include "pldmnode.h"
#include "PdrRepository.h"

class clientNode : public PldmNode
{

	mctp_struct *mctp;

	unsigned int   maxPDRSize = 64;

public:
	void init(mctp_struct *);
	clientNode();
	virtual void putCommand(PldmRequestHeader* hdr, unsigned char* command, unsigned int size);
	virtual unsigned char* getResponse(void);

	// PLDM commands (shortcuts)
	bool setNumericEffecterValue(GenericPdr* effecterpdr, double data);
	double getNumericEffecterValue(GenericPdr* effecterpdr);
	bool setStateEffecterStates(GenericPdr* effecterpdr, enum8 effecterState);
	enum8 getStateEffecterStates(GenericPdr* effecterpdr);
	double getSensorReading(GenericPdr* sensorpdr);
	enum8 getStateSensorReadings(GenericPdr* sensorpdr);
	bool setNumericEffecterEnable(GenericPdr* effecterpdr, uint8 enableState);
	bool setStateEffecterEnables(GenericPdr* effecterpdr, uint8 enableState);

	// complete PLDM commands
	map<int,string> setNumericEffecterValue(GenericPdr* pdr, map<int,string> &params);
	map<int,string> getNumericEffecterValue(GenericPdr* pdr, map<int,string> &params);
	map<int,string> setStateEffecterStates(GenericPdr* pdr, map<int,string> &params, PdrRepository &repo);
	map<int,string> getStateEffecterStates(GenericPdr* pdr, map<int,string> &params, PdrRepository &repo);
	map<int,string> getSensorReading(GenericPdr* pdr, map<int,string> &params);
	map<int,string> getStateSensorReadings(GenericPdr* pdr, map<int,string> &params, PdrRepository &repo);
	map<int,string> setNumericEffecterEnable(GenericPdr* pdr, map<int,string> &params);
	map<int,string> setStateEffecterEnables(GenericPdr* pdr, map<int,string> &params);
};

