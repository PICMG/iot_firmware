//*******************************************************************
//    vprofiler.h
//
//    This header file declares functions related to the velocity 
//    profiler function and is part of the PICMG reference code for
//    IoT.
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
#ifndef VPROFILER_H_INCLUDED
#define VPROFILER_H_INCLUDED

#define FP16 long
#define TO_FP16(x) ((long)(x*65536))
#define FP16_TO_FLOAT(x) (((float)(x))/((float)65536.0f))
#define FLOAT_TO_FP16(x) ((long)((x)*65536.0f))

void vprofiler_setParameters(long deltax, FP16 velocity, FP16 acceleration, char scurve);
void vprofiler_start();
void vprofiler_update();
void vprofiler_startv();
void vprofiler_updatev();
void vprofiler_stop();
unsigned char vprofiler_isDone();

extern long current_velocity;
#endif // VPROFILER_H_INCLUDED
