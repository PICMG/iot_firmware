//*******************************************************************
//    pdrdata.h
//
//    This is an example header file that can be used with code emitted
//    from pdrmaker. 
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
#include <avr/pgmspace.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLE_RATE 4000
typedef signed long FIXEDPOINT_24_8;

#define PDR_BYTE_TYPE const unsigned char
#define PDR_DATA_ATTRIBUTES PROGMEM

extern PDR_BYTE_TYPE __pdr_data[] PDR_DATA_ATTRIBUTES;
extern unsigned int __pdr_total_size;
extern unsigned int __pdr_number_of_records;
extern unsigned int __pdr_max_record_size;

#ifdef __cplusplus
}
#endif
