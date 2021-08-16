//    adc.h
//
//    This header file decleres functions related to the analog interface
//    channels.
//    This code is intended to be used as part of the PICMG reference code 
//    for IoT.
//    
//    More information on the PICMG IoT data model can be found within
//    the PICMG family of IoT specifications.  For more information,
//    please visit the PICMG web site (www.picmg.org)
//
//    Copyright (C) 2021,  PICMG
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
#include "config.h"

//===================================================================
// Function Declarations
void adc_sample(); 
void adc_init(); 
unsigned int adc_getRawData();


// declarations for digital input channels
#ifdef CHANNEL_AIN2_5V
    #define ain2_5v_sample adc_sample
    #define ain2_5v_init   adc_init
    #define ain2_5v_getRawData adc_getRawData    
#endif
#ifdef CHANNEL_AIN5V
    #define ain5v_sample adc_sample
    #define ain5v_init   adc_init
    #define ain5v_getRawData adc_getRawData    
#endif
#ifdef CHANNEL_AIN12V
    #define ain12v_sample adc_sample
    #define ain12v_init   adc_init
    #define ain12v_getRawData adc_getRawData    
#endif
#ifdef CHANNEL_AIN24V
    #define ain24v_sample adc_sample
    #define ain24v_init   adc_init
    #define ain24v_getRawData adc_getRawData    
#endif
#ifdef CHANNEL_AIN20MA
    #define ain20ma_sample adc_sample
    #define ain20ma_init   adc_init
    #define ain20ma_getRawData adc_getRawData    
#endif
#ifdef CHANNEL_AIN6200OHM
    #define ain6200ohm_sample adc_sample
    #define ain6200ohm_init   adc_init
    #define ain6200ohm_getRawData adc_getRawData    
#endif
#ifdef CHANNEL_AIN4270OHM
    #define ain4270ohm_sample adc_sample
    #define ain4270ohm_init   adc_init
    #define ain4270ohm_getRawData adc_getRawData    
#endif
#ifdef CHANNEL_AIN2950OHM
    #define ain2950ohm_sample adc_sample
    #define ain2950ohm_init   adc_init
    #define ain2950ohm_getRawData adc_getRawData    
#endif
#ifdef CHANNEL_AIN2060OHM
    #define ain2060ohm_sample adc_sample
    #define ain2060ohm_init   adc_init
    #define ain2060ohm_getRawData adc_getRawData    
#endif
#ifdef CHANNEL_AIN1790OHM
    #define ain1790ohm_sample adc_sample
    #define ain1790ohm_init   adc_init
    #define ain1790ohm_getRawData adc_getRawData    
#endif
#ifdef CHANNEL_AIN1360OHM
    #define ain1360ohm_sample adc_sample
    #define ain1360ohm_init   adc_init
    #define ain1360ohm_getRawData adc_getRawData    
#endif
#ifdef CHANNEL_AIN625OHM
    #define ain625ohm_sample adc_sample
    #define ain625ohm_init   adc_init
    #define ain625ohm_getRawData adc_getRawData    
#endif

