#pragma once
#include "MPU9250.h"
#include "Components.h"
#include <list>

class MotionSensor : public MPU9250 {
public:

	/*
	* Constructor, inherited from the base class mpu9250.
	*/
	MotionSensor(uint8_t address = MPU9250_ADDRESS_AD0, TwoWire& wirePort = Wire, uint32_t clock_frequency = 100000) : MPU9250{ address, wirePort, clock_frequency } {}

	/*
	* Reads new data from the sensor if it is available.
	*/
	void updateData();

	/*
	* Gets current acceleration with bias removed. Call updateData() before calling this function.
	* @return - current acceleration
	*/
	Components getAcc();

	/*
	* Gets current angular velocity. Call updateData() before calling this function.
	* @return - current angular velocity
	*/
	Components getVel();

	/*
	* Gets current magnetic force with bias removed. Call updateData() before calling this function.
	* @return - current magnetic force.
	*/
	Components getMag();

	/*
	* Filter noise out of acceleration data.
	* @param currentAcc - the current unfiltered acceleration (x, y, z)
	* @param prevAccLP - previous low pass filtered acceleration (x, y, z)
	* @return - the low pass filtered current acceleration (x, y, z)
	*/
	Components accLPF(Components& currentAcc, Components& prevAccLP);

	/*
	* Filter noise out of velocity data.
	* @param currentVel - the current unfiltered angular velocity (x, y, z)
	* @param prevVelLP - previous low pass filtered angular velocity (x, y, z)
	* @return - the low pass filtered current velocity (x, y, z)
	*/
	Components velLPF(Components& currentVel, Components& prevVelLP);

	/*
	* Filter noise out of magnetic force data.
	* @param currentMag - the current unfiltered magnetic force (x, y, z)
	* @param prevMagLP - previous low pass filtered magnetic force (x, y, z)
	* @return - the low pass filtered current magnetic force (x, y, z)
	*/
	Components magLPF(Components& currentMag, Components& prevMagLP);

	/*
	* Scale acceleration so that the value is in the apprioriate units.
	* @param acc - the current acceleration (x, y, z), low pass filtered.
	* @return the scaled acceleration in g (x, y, z)
	*/
	Components scaleAcc(Components& acc);

	/*
	* Scale angular velocity so that the value is in the apprioriate units.
	* @param vel - the current angular velocity (x, y, z), low pass filtered.
	* @return the scaled angular velocity in rad/s (x, y, z)
	*/
	Components scaleVel(Components& vel);

	/*
	* Scale magnetic force so that the value is in the apprioriate units.
	* @param mag - the current magnetic force (x, y, z), low pass filtered.
	* @return the scaled magnetic force in milligaus (x, y, z)
	*/
	Components scaleMag(Components& mag);

	/*
	* High pass filter for acceleration data.
	* @preconditions - accs is a linked list.
	* - the accelerations should be scaled and low pass filtered.
	* - the front of the list holds the oldest sample.
	* @param accs - a list of the last 5 scaled and low pass filtered accelerations (x, y, z)
	* @return - the high pass filtered current acceleration (x, y, z)
	*/
	Components accHPF(std::list<Components>& accs);

private:
	float NPACC = 200; // noise parameter accelerations
	float NPSACC = 150; // noise parameter sensitivity accelerations
	float NPLACC = 0.025; // noise parameter low accelerations
	float NPMAG = 3; // noise parameter magnetic forces
	float NPSMAG = 3; // noise parameter sensitivity magnetic forces
	float NPLMAG = 0.025; // noise parameter low magnetic forces
	float NPVEL = 30; // noise parameter angular velocity
	const int FIR_SIZE = 5;
	const int BUFFER_SIZE = 100;
};

