//    interpolator.c
//
//    This header file defines functions required to interpolate raw
//    data readings.  The code was extensively adapted from code provided
//    by Triple-Ring technology and has been used with permission of the 
//    original author.
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

#include "interpolator.h"
#pragma GCC push_options
#pragma GCC optimize "-O3"

// convert a 16 bit integer to a 16.16 fixed-point integer
long to_fp16(int num) {return ((long)(num))*(65536L);}

// multiply two 16.16 fixed-point numbers and return the result
long fp16_mult(long a, long b) {
	long product = (((((long long)a)*((long long) b)))/65536);
	return product;
}

/*
* interpolator_linearize()
*
* given the raw data, x, with the precision specified (in bits), return the 
* interpolated value using the provided data table.
* 
* the data table and the result are expressed as 16.16 fixed-point numbers
*/
long interpolator_linearize ( int x, LINTABLE_TYPE *table, char precision ) {
	unsigned char   nL;  // the table index for point at the low end of the interval
	int     		xL;  // the value of x at the lower end of the interval
	// the following variables are 16.16 fixed-point precision
	long			nInterval;		// distance into the current interval (in units of x)
	long 			intervalPercent; // distance into current interval in percentage
	long			anLutVal[5];	// lookup tables of interest
	long			anLutDif[3];	// linear differences around the point
	long			nLutDDif;		// second-order linear difference around point
	long			result;			// the result

	long span = (precision<0)?((1L)<<(-precision))/64:((1L)<<(precision))/64;
	char offset = (precision<0)?32:0;

	nL = ( x / span ) + offset;
	xL = (int)( nL * span );  // the x value at the lower interval

	// the distance into the current interval expressed in units of x
	nInterval       = to_fp16( x - xL );

	// the distance into the current interval expressed as a percentage
	intervalPercent = nInterval/span; 

	// lookups are all the fixed point format - no conversion required.
	anLutVal[0] = pgm_read_dword(&table[nL]);	// table value three intervals below our point
	anLutVal[1] = pgm_read_dword(&table[nL+1]);	// table value two intervals below our point
	anLutVal[2] = pgm_read_dword(&table[nL+2]);	// table value at the interval boundary below our point
	anLutVal[3] = pgm_read_dword(&table[nL+3]);  // table value at the interval above our point
	anLutVal[4] = pgm_read_dword(&table[nL+4]);  // table value at two intervals above our point
	
	// calculate differences
	// anLutDif[0] = ( anLutVal[2] - anLutVal[0] ) / 2; // linear difference before the point
	anLutDif[1] = ( anLutVal[3] - anLutVal[1] ) / 2; // linear difference spanning the point
	anLutDif[2] = ( anLutVal[4] - anLutVal[2] ) / 2; // linear difference after the point

	// calculate second difference around the point
	nLutDDif = ( anLutDif[2] - anLutDif[1] );

	result = fp16_mult(nLutDDif,intervalPercent);
	result = fp16_mult(result + anLutDif[1],intervalPercent);
	result += anLutVal[2];

	return result;
}

#pragma GCC pop_options
