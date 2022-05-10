#pragma once
#include "Components.h"

/*
* A data package for orientation data: position and orientation
*/
struct OrientationPacket {
	Components position; // position data (x, y, z)
	Components orientation; // orientation data (roll, pitch, yaw)
};