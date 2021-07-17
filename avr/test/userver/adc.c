//    adc.c
//
//    This header file defines functions related to the analog input channel  
//    support.  Much of this code is conditionally compiled
//    based on macro definitions from the configuraiton header file.
//
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
#ifndef __AVR_ATmega328P__ 
#define __AVR_ATmega328P__
#endif

#include "pdrdata.h"
#include "avr/io.h"
#include "adc.h"

//===================================================================
// Initialization Macros

// declarations for digital input channels
#ifdef CHANNEL_AIN2_5V
    #define EN_IH 0
    #define EN_IL 0
    #define PD_B  0
    #define PD_C  0
    #define PD_D  0
    #define USE_ADC
#endif
#ifdef CHANNEL_AIN5V
    #define EN_IH 0
    #define EN_IL 0
    #define PD_B  0
    #define PD_C  0
    #define PD_D  0
    #define USE_ADC
#endif
#ifdef CHANNEL_AIN12V
    #define EN_IH 0
    #define EN_IL 0
    #define PD_B  0
    #define PD_C  0
    #define PD_D  0
    #define USE_ADC
#endif
#ifdef CHANNEL_AIN24V
    #define EN_IH 0
    #define EN_IL 0
    #define PD_B  0
    #define PD_C  0
    #define PD_D  0
    #define USE_ADC
#endif
#ifdef CHANNEL_AIN20MA
    #define EN_IH 1
    #define EN_IL 0
    #define PD_B  0
    #define PD_C  0
    #define PD_D  0
    #define USE_ADC
#endif
#ifdef CHANNEL_AIN6200OHM
    #define EN_IH 0
    #define EN_IL 0
    #define PD_B  0
    #define PD_C  0
    #define PD_D  0
    #define USE_ADC
#endif
#ifdef CHANNEL_AIN4270OHM
    #define EN_IH 0
    #define EN_IL 0
    #define PD_B  1
    #define PD_C  0
    #define PD_D  0
    #define USE_ADC
#endif
#ifdef CHANNEL_AIN2950OHM
    #define EN_IH 0
    #define EN_IL 0
    #define PD_B  0
    #define PD_C  1
    #define PD_D  0
    #define USE_ADC
#endif
#ifdef CHANNEL_AIN2060OHM
    #define EN_IH 0
    #define EN_IL 0
    #define PD_B  0
    #define PD_C  0
    #define PD_D  1
    #define USE_ADC
#endif
#ifdef CHANNEL_AIN1790OHM
    #define EN_IH 0
    #define EN_IL 0
    #define PD_B  1
    #define PD_C  0
    #define PD_D  1
    #define USE_ADC
#endif
#ifdef CHANNEL_AIN1360OHM
    #define EN_IH 0
    #define EN_IL 0
    #define PD_B  1
    #define PD_C  1
    #define PD_D  1
    #define USE_ADC
#endif
#ifdef CHANNEL_AIN625OHM
    #define EN_IH 1
    #define EN_IL 0
    #define PD_B  0
    #define PD_C  0
    #define PD_D  0
    #define USE_ADC
#endif

#ifdef USE_ADC
    // declare the channel-related raw data
    static unsigned int ain_rawdata;

    //===================================================================
    // adc_init()
    //
    // initialize all the channels based on configuration switches
    //
    // parameters: none
    // returns: nothing
    // changes:
    //   The state of all channel hardware will be initialized and set
    // to the "disabled" state.
    void adc_init() 
    {
        // Internal reference to 2.5V (AREF)
        // set the input mux to ain+ (ADC7)
        ADMUX = (1<<MUX2) | (1<<MUX1) | (1<<MUX0);

        // set precaler divisor of 128
        ADCSRA = (1<<ADEN)|(1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);

        #ifdef PE2 
            // if the device is a ATMEGA328PB, it supports all these 
            // control pins - set the control pins to outputs,       
            // otherwise, assume the resistance values are changed   
            // by some other means.                                  
            DDRC |= ((1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3)) 
            DDRE |= ((1<<PE2)); 
            PORTC = (PORTC&(~PC0)) + (EN_IL<<PC0); 
            PORTC = (PORTC&(~PC1)) + (EN_IH<<PC1); 
            PORTC = (PORTC&(~PC2)) + (PD_C<<PC2);  
            PORTC = (PORTC&(~PC3)) + (PD_D<<PC3);  
            PORTE = (PORTE&(~PE2)) + (PD_B<<PE2);  
        #endif \

        // start the first conversion
        ADCSRA |= 0x40;
    }

    /**
     * Sample the adc converter and start the next conversion cycle
     */
    void  adc_sample() { 
        unsigned char lo = ADCL; 
        unsigned char hi = ADCH; 
        ain_rawdata = ((unsigned long)(hi<<8)) | (unsigned long)lo; 

        // start the next conversion
        ADCSRA |= 0x40;
    } 

    /**
     * return the last sampled value
     */
    unsigned int adc_getRawData() { 
        return ain_rawdata; 
    } 
#else
    adc_init() {};
#endif