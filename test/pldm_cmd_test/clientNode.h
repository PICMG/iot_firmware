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
//	unsigned char rxBuffer[512];
//	unsigned char txBuffer[512];
	mctp_struct *mctp;

	unsigned char  pdrCount;
	unsigned int   nextByte;
	unsigned int   maxPDRSize = 64;

	unsigned int pdrSize(unsigned int index);
	void processCommandGetPdr(PldmRequestHeader* rxHeader);
public:
	void init(mctp_struct *);
	clientNode();
	void parseCommand();
	virtual void putCommand(PldmRequestHeader* hdr, unsigned char* command, unsigned int size);
	virtual unsigned char* getResponse(void);

	// PLDM commands
	bool setNumericEffecterValue(unsigned long effecterID, GenericPdr* effecterpdr, double data);
	double getNumericEffecterValue(unsigned long effecterID, GenericPdr* effecterpdr);
	bool setStateEffecterStates(unsigned long effecterID, GenericPdr* effecterpdr, enum8 effecterState);
	enum8 getStateEffecterStates(unsigned long effecterID, GenericPdr* effecterpdr);
	double getSensorReading(unsigned long sensorID, GenericPdr* sensorpdr);
	enum8 getStateSensorReadings(unsigned long sensorID, GenericPdr* sensorpdr);
	bool setNumericEffecterEnable(unsigned long effecterID, GenericPdr* effecterpdr, uint8 enableState);
	bool setStateEffecterEnables(unsigned long effecterID, GenericPdr* effecterpdr, uint8 enableState);
};

