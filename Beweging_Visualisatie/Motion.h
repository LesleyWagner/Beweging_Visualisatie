#pragma once

#include "Components.h"
#include "math.h"
#include "quaternionFilters.h"

class Motion {
public:
	/*
	* Calculates x, y, z coordinates of the position of an object given its previous and current acceleration, orientation, previous position and time interval.
	* @param currentAcc - current acceleration of the object (x, y, z)
	* @param orientation - orientation of the object (yaw, pitch, roll)
	* @param prevPosition - previous position of the object (x, y, z)
	* @param dt - time interval between acceleration data / position calculations
	* @return x, y, z components of the position
	*/
	Components calcPosition(const Components currentAcc, const Components orientation, const Components prevPosition, double dt);

	/*
	* Calculates x, y, z coordinates of the orientation of an object.
	* @param acc - current acceleration of the object (x, y, z)
	* @param vel - current velocity of the object (x, y, z)
	* @param mag - current magnetic force of the object (x, y, z)
	* @param dt - time interval between samples
	* @return x, y, z components of the orientation (yaw, pitch, roll)
	*/
	Components calcOrientation(const Components acc, const Components vel, const Components mag,
		float dt);

	Motion();

private:
	// previous acceleration
	double a0_x = 0;
	double a0_y = 0;
	double a0_z = 0;
	// previous velocity
	double v0_x = 0;
	double v0_y = 0;
	double v0_z = 0;
};