//    vprofiler.h
//
//    This header file defines functions related to the velocity 
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
#include <math.h>
#include <stdlib.h>
#include "uart.h"
#include "vprofiler.h"

static long active_position;
static long active_velocity;
static long active_acceleration;
static char active_scurve;

static long current_t;
long current_velocity;      // fixed point 16 fractional bits
static long current_acceleration;  // fixed point 16 fractional bits
static long current_position;      // fixed point 16 fractional bits
static long current_jerk;
static long current_jerk6;

static long gt1, gt2, gt3, gt4, gt5;
static long gdx1, gdx2, gdx3, gdx4, gdx5;
static long gv1, gv2, gv3, gv4;
static long ga1, ga2, ga3, ga4;
static long gj1, gj2, gj3, gj4;
static long gj1_6, gj2_6, gj3_6, gj4_6;

static char estop = 0;
static char phase = 0x7F;
static long dwell = 10;            // the dwell time after the profile
								   // completes before it registers as done.

/********************************************************************
* setParameters
*
* set the motion parameters for the next move.  The new parameters are
* not used until the next motion is begun.
*
* parameters:
*   deltax - a signed integer, that specifies the number of encoder
*      steps that the motor should move.
*   velocity - a 15.16 signed floating point number for the target
*      steady-state velocity of the profile. Although this number is
*      signed, assume it is positive only.
*   acceleration - a 15.16 signed floating point number that specifies
*      the average acceleration during the transient phase of the
*      motion.  Acceleration must be more than 1/32000 times the velocity
*      to avoid numeric overflow.
*   scurve - nonzero if the profile should be an s-curve.  zero
*      if the motion should be a trapezoid.
*/
void vprofiler_setParameters(long deltax, FP16 velocity, FP16 acceleration, char scurve) {
	active_position = deltax;
	active_velocity = (deltax > 0) ? labs(velocity) : -1 * labs(velocity);
	active_acceleration = (deltax > 0) ? labs(acceleration) : -1 * labs(acceleration);
	active_scurve = scurve;
}

static FP16 calc_v(float v0, float a, float j, float dt) {
	return FLOAT_TO_FP16(v0 + a * dt + j * dt * dt / 2.0f);
}

static FP16 calc_dx(float v0, float a, float j, float dt) {
	return FLOAT_TO_FP16(v0*dt + a * dt*dt/2.0f + j * dt * dt * dt / 6.0f);
}

/********************************************************************
* try3()
*
*/
void vprofiler_start()
{
	float t2 = FP16_TO_FLOAT(active_velocity) / FP16_TO_FLOAT(active_acceleration);
	// for scurve, average_acceleration = (1/2) peak_acceleration

	// calculate the distance traveled if the the trajectory accelerates fully to
	// the plateau velocity.
	float xt2 = 0.5f * FP16_TO_FLOAT(active_acceleration) * t2 * t2;
	float t3 = t2 + ((float)active_position-2.0f*xt2) / FP16_TO_FLOAT(active_velocity);
	float jerk = 0.0f;
	float vplateau = FP16_TO_FLOAT(active_velocity);

	if (active_scurve) jerk = FP16_TO_FLOAT(active_acceleration) * 4.0f / t2;
	if (fabs(xt2) >= fabs((float)active_position) / 2.0f) {
		// truncated - average acceleration remains as requested transient and
		// plateau times change
		t2 = sqrt((float)active_position / FP16_TO_FLOAT(active_acceleration));
		t3 = t2;
		xt2 = 0.5f * FP16_TO_FLOAT(active_acceleration) * t2 * t2;
		// calculate the plateau velocity
		if (active_scurve) jerk = FP16_TO_FLOAT(active_acceleration)*4.0f / t2;
		vplateau = FP16_TO_FLOAT(active_acceleration) * t2;
	}
	// float dxt3 = (float)active_position - 2.0 * xt2;

	// calculate the time points for each transition
	// half way through the acceleration transient
	gt1 = (long)(t2 / 2.0 + 1.0);
	gt2 = (long)(t2 + 1.0);
	gt3 = (long)(t3)+1;
	gt4 = (long)(t3 + t2 / 2.0 + 1.0);
	gt5 = (long)(t3 + t2 + 1.0);

	float deltat;
	deltat = (float)gt1 - (t2 / 2.0f);
	gdx1 = (active_scurve) ?
		calc_dx(vplateau/2.0, +2.0f * FP16_TO_FLOAT(active_acceleration), -jerk, deltat) :
		calc_dx(vplateau / 2.0, +FP16_TO_FLOAT(active_acceleration), 0.0f, deltat);
	gdx1 -= (active_scurve) ?
		calc_dx(vplateau / 2.0, 2.0f * FP16_TO_FLOAT(active_acceleration), jerk, (deltat - 1.0f)) :
		calc_dx(vplateau / 2.0, FP16_TO_FLOAT(active_acceleration), 0.0f, (deltat - 1.0f));
	gv1 = (active_scurve) ?
		calc_v(vplateau / 2.0, + 2.0f * FP16_TO_FLOAT(active_acceleration),-jerk,deltat) :
		calc_v(vplateau / 2.0, + FP16_TO_FLOAT(active_acceleration), 0.0f, deltat);
	ga1 = FLOAT_TO_FP16((active_scurve) ?
		2.0f * FP16_TO_FLOAT(active_acceleration) - jerk * deltat :
		FP16_TO_FLOAT(active_acceleration));
	gj1 = FLOAT_TO_FP16((active_scurve) ?-jerk : 0.0f);
	gj1_6 = gj1 / 6;

	deltat = (float)gt2 - t2;
	gdx2 = calc_dx(vplateau, 0, 0, deltat);
	gdx2 -= (active_scurve) ?
		calc_dx(vplateau, 0, -jerk, (deltat - 1.0f)) :
		calc_dx(vplateau, FP16_TO_FLOAT(active_acceleration), 0.0f, (deltat - 1.0f));
	gv2 = FLOAT_TO_FP16(vplateau);
	ga2 = 0;
	gj2 = 0;
	gj2_6 = gj2 / 6;

	deltat = (float)gt3 - t3;
	gdx3 = (active_scurve) ?
		calc_dx(vplateau, 0, -jerk, deltat) :
		calc_dx(vplateau, FP16_TO_FLOAT(-active_acceleration), 0.0f, deltat);
	if (gt2 == gt3) {
		// truncated waveform
		gdx3 -= (active_scurve) ?
			calc_dx(vplateau, 0, -jerk, (deltat - 1.0f)) :
			calc_dx(vplateau, FP16_TO_FLOAT(active_acceleration), 0.0f, (deltat - 1.0f));
	}
	else {
		// full waveform
		gdx3 -= (active_scurve) ?
			calc_dx(vplateau, 0, 0, (deltat - 1.0)) :
			calc_dx(vplateau, 0, 0.0f, (deltat - 1.0));
	}
	gv3 = (active_scurve) ?
		calc_v(vplateau, 0, -jerk, deltat) :
		calc_v(vplateau, FP16_TO_FLOAT(-active_acceleration), 0.0f, deltat);
	ga3 = FLOAT_TO_FP16((active_scurve) ?- jerk * deltat : FP16_TO_FLOAT(-active_acceleration));
	gj3 = FLOAT_TO_FP16((active_scurve) ? -jerk : 0.0f);
	gj3_6 = gj3 / 6;

	deltat = (float)gt4 - (t3 + t2 / 2.0f);
	gdx4 = (active_scurve) ?
		calc_dx(vplateau / 2.0f, 2.0f * FP16_TO_FLOAT(-active_acceleration), +jerk, deltat) :
		calc_dx(vplateau / 2.0f, FP16_TO_FLOAT(-active_acceleration), 0.0f, deltat);
	gdx4 -= (active_scurve) ?
		calc_dx(vplateau / 2.0f, 2.0f * FP16_TO_FLOAT(-active_acceleration), -jerk, (deltat-1.0f)) :
		calc_dx(vplateau / 2.0f, FP16_TO_FLOAT(-active_acceleration), 0.0f, (deltat-1.0f));
	gv4 = (active_scurve) ?
		calc_v(vplateau/2.0f, 2.0f * FP16_TO_FLOAT(-active_acceleration), +jerk, deltat) :
		calc_v(vplateau/2.0f, FP16_TO_FLOAT(-active_acceleration), 0.0f, deltat);
	ga4 = FLOAT_TO_FP16((active_scurve) ?
		2.0f * FP16_TO_FLOAT(-active_acceleration) + jerk * deltat :
		FP16_TO_FLOAT(-active_acceleration));
	gj4 = FLOAT_TO_FP16((active_scurve) ? +jerk : 0.0f);
	gj4_6 = gj4 / 6;

	deltat = (float)gt5 - (t2 + t3);
	gdx5 = (active_scurve) ?
		-calc_dx(0.0f, 0.0f, jerk, (deltat - 1.0f)) :
		-calc_dx(0.0f, FP16_TO_FLOAT(-active_acceleration), 0.0f, (deltat - 1.0f));

	current_position = 0;
	current_velocity = 0;
	current_jerk = FLOAT_TO_FP16(jerk);
	current_jerk6 = current_jerk / 6;
	current_acceleration = (active_scurve) ? 0 : active_acceleration;
	current_t = gt1;
	phase = 0;
}

void vprofiler_stop()
{
    estop = 1;
}

/********************************************************************
* update
*
* this function should be called once per sample period.  It updates
* the motion trajectory based on trajectory paramters that were
* previously set.
*/
void vprofiler_update() {
    if (estop) {
        current_velocity = 0;
        current_acceleration = 0;
        current_jerk = 0;
        current_jerk6 = 0;
        phase = 5;
        estop = 0;
        current_t = dwell;
    }
	if (current_t == 1) {
		switch (phase) {
			case 0:
				// second half of acceleration transient
				current_position = gdx1;
				current_velocity = gv1;
				current_acceleration = ga1;
				current_jerk = gj1;
				current_jerk6 = gj1_6;
				current_t = gt2 - gt1;
				phase = (gt2 == gt3) ? 2 : 1;
				break;
			case 1:
				// start of velocity plateau
				current_position = gdx2;
				current_velocity = gv2;
				current_acceleration = ga2;
				current_jerk = gj2;
				current_jerk6 = gj2_6;
				current_t = gt3 - gt2;
				phase = 2;
				break;
			case 2:
				// end of velocity plateau
				current_position = gdx3;
				current_velocity = gv3;
				current_acceleration = ga3;
				current_jerk = gj3;
				current_jerk6 = gj3_6;
				current_t = gt4 - gt3;
				phase = 3;
				break;
			case 3:
				// second half of deceleration transient
				current_position = gdx4;
				current_velocity = gv4;
				current_acceleration = ga4;
				current_jerk = gj4;
				current_jerk6 = gj4_6;
				phase = 4;
				current_t = gt5 - gt4;
				break;
			case 4:
				// completion of motion
				current_position = gdx5;
				current_velocity = 0;
				current_acceleration = 0;
				current_jerk = 0;
				current_jerk6 = 0;
				phase = 5;
				current_t = dwell;
				break;
			default:
				// dwell time expired
				current_t = 0xFFFFFFFF;
				phase = 0x7F;
				break;
		}
	}
	else {
		current_position = current_velocity + current_acceleration / 2 + current_jerk6;
		current_velocity += current_acceleration + current_jerk / 2;
		current_acceleration += current_jerk;
		current_t--;
	}
}

unsigned char vprofiler_isDone() {
	return phase == 0x7f;
}
