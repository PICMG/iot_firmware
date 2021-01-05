//    control_servo.h
//
//    This header file declares functions related to the the servo 
//    controller mode of operation for a PICMG IoT device node.  This
//    code is intended to be used as part of the PICMG reference code for IoT.
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
#ifndef CONTROL_SERVO_H_INCLUDED
#define CONTROL_SERVO_H_INCLUDED
#include "vprofiler.h"

#define SERVO_CMD_NONE   0
#define SERVO_CMD_RUN    1
#define SERVO_CMD_STOP   2
#define SERVO_CMD_DONE   3
#define SERVO_CMD_ERR    4
#define SERVO_CMD_COND   5

#define SERVO_MODE_NOWAIT  0
#define SERVO_MODE_WAIT    1

#define SERVO_FLAGS_ERROR      0x80
#define SERVO_FLAGS_TRIGGER    0x40

extern unsigned char servo_cmd;   // the servo command to execute
extern unsigned char servo_mode;  // the servo mode (wait/nowait)
extern unsigned char servo_flags; // set by synchronization/error conditions

extern long requested_position;
extern FP16 requested_velocity;
extern FP16 requested_acceleration;
extern FP16 requested_kffa;

void control_update();  // servo update loop (priority mode)

#endif