//*******************************************************************
//    platform_defs.h
//
//    This header file adds platform-specific definitions for the build
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

#define SAMPLE_RATE 4000
typedef signed long FIXEDPOINT_24_8;

#define PDR_BYTE_TYPE const unsigned char
#define FRU_BYTE_TYPE const unsigned char
#define LINTABLE_TYPE const long
#define PDR_DATA_ATTRIBUTES PROGMEM
#define FRU_DATA_ATTRIBUTES PROGMEM
#define LINTABLE_DATA_ATTRIBUTES PROGMEM
