//*******************************************************************
//    fcs.h
//
//    This file provides definitions for a Frame Check Sequence used
//    as part of the PICMG pldm library reference code. 
//    
//    Portions of this code are based on the IETF RFC 1662 
//    as required by the DMTF MCTP and PLDM protocols.
//    More information about PLDM and MCTP can be found on the DMTF
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

//constant values used as checks by the FCS
#define  INITFCS 0xffff
#define  GOODFCS 0xf0b8

//FCS value generator
unsigned int fcs_calcFcs(unsigned int fcs, unsigned char* cp, unsigned int len);

