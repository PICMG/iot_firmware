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
#include "config.h"
#include "avr/io.h"
#include "adc.h"

//===================================================================
// Initialization Macros

// declarations for digital input channels - these should configure the external
// resistance network.  They are not used for this architecture
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
        // Configure the resolution by writing to the Resolution Selection (RESSEL) bit in the Control A (ADCn.CTRLA)
        // register.
        ADC0_CTRLA = ADC_RESSEL_10BIT_gc;

        // Configure a voltage reference by writing to the Reference Selection (REFSEL) bit in the Control C
        // (ADCn.CTRLC) register. The default is the internal voltage reference of the device (VREF, as configured there).
        // Configure the CLK_ADC by writing to the Prescaler (PRESC) bit field in the Control C (ADCn.CTRLC) register.
        // Reduce the sampling capacitance for better performance with higher voltage references.
        ADC0_CTRLC = ADC_REFSEL_VDDREF_gc + ADC_PRESC_DIV32_gc + ADC_SAMPCAP_bm;

        // Configure an input by writing to the MUXPOS bit field in the MUXPOS (ADCn.MUXPOS) register.
        // This configures for AIN5 which is connected to A7 on the Arduino Every header
        ADC0_MUXPOS = ADC_MUXPOS_AIN5_gc;

        // Enable the ADC by writing a ‘1’ to the ENABLE bit in ADCn.CTRLA.
        ADC0_CTRLA |= ADC_ENABLE_bm;

        // disable the digital input on the analog pin
        PORTD_PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;

        // start the first conversion
        ADC0_COMMAND = ADC_STCONV_bm;
    }

    /**
     * Sample the adc converter and start the next conversion cycle
     */
    void  adc_sample() { 
        // get the new sample reading
        ain_rawdata = ADC0_RES; 

        // start the next conversion
        ADC0_COMMAND = ADC_STCONV_bm;
    } 

    /**
     * return the last sampled value
     */
    unsigned int adc_getRawData() { 
        return ain_rawdata; 
    } 
#else
    void adc_init() {};
#endif