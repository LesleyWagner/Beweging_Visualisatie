#include "MotionSensor.h"

void MotionSensor::updateData() {
	// If intPin goes high, all data registers have new data
	// On interrupt, check if data ready interrupt
	if (readByte(MPU9250_ADDRESS_AD0, INT_STATUS) & 0x01) {
		readAccelData(accelCount);  // Read the x/y/z adc values
		readGyroData(gyroCount);  // Read the x/y/z adc values
		readMagData(magCount);  // Read the x/y/z adc values
	}

	// Must be called before updating quaternions!
	updateTime();
}

Components MotionSensor::getAcc() {
	// Now we'll calculate the accleration value into actual g's
	// This depends on scale being set
	ax = (float)accelCount[0] * aRes;
	ay = (float)accelCount[1] * aRes; // - accelBias[1];
	az = (float)accelCount[2] * aRes; // - accelBias[2];
	return Components{ ax, ay, az };
}

Components MotionSensor::getVel() {
	// Calculate the gyro value into actual degrees per second
	// This depends on scale being set
	gx = (float)gyroCount[0] * gRes;
	gy = (float)gyroCount[1] * gRes;
	gz = (float)gyroCount[2] * gRes;
	return Components{ gx, gy, gz };
}

Components MotionSensor::getMag() {
	// Calculate the magnetometer values in milliGauss
	// Include factory calibration per data sheet and user environmental
	// corrections
	// Get actual magnetometer value, this depends on scale being set
	mx = (float)magCount[0] * factoryMagCalibration[0] * mRes * magScale[0] - magBias[0];
	my = (float)magCount[1] * factoryMagCalibration[1] * mRes * magScale[1] - magBias[1];
	mz = (float)magCount[2] * factoryMagCalibration[2] * mRes * magScale[2] - magBias[2];
	return Components{ mx, my, mz };
}

Components MotionSensor::accLPF(Components& currentAcc, Components& prevAccLP) {
	Components currentAccLP;
	// x component
	if (currentAcc.x < prevAccLP.x - NPACC || currentAcc.x > prevAccLP.x + NPACC) {
		currentAccLP.x = currentAcc.x;
	}
	else {
		if (currentAcc.x < prevAccLP.x - NPSACC || currentAcc.x > prevAccLP.x + NPSACC) {
			currentAccLP.x = (currentAcc.x + prevAccLP.x) / 2;
		}
		else {
			currentAccLP.x = (1 - NPLACC) * prevAccLP.x + NPLACC * currentAcc.x;
		}
	}
	// y component
	if (currentAcc.y < prevAccLP.y - NPACC || currentAcc.y > prevAccLP.y + NPACC) {
		currentAccLP.y = currentAcc.y;
	}
	else {
		if (currentAcc.y < prevAccLP.y - NPSACC || currentAcc.y > prevAccLP.y + NPSACC) {
			currentAccLP.y = (currentAcc.y + prevAccLP.y) / 2;
		}
		else {
			currentAccLP.y = (1 - NPLACC) * prevAccLP.y + NPLACC * currentAcc.y;
		}
	}
	// z component
	if (currentAcc.z < prevAccLP.z - NPACC || currentAcc.z > prevAccLP.z + NPACC) {
		currentAccLP.z = currentAcc.z;
	}
	else {
		if (currentAcc.z < prevAccLP.z - NPSACC || currentAcc.z > prevAccLP.z + NPSACC) {
			currentAccLP.z = (currentAcc.z + prevAccLP.z) / 2;
		}
		else {
			currentAccLP.z = (1 - NPLACC) * prevAccLP.z + NPLACC * currentAcc.z;
		}
	}
	return currentAccLP;
}

Components MotionSensor::velLPF(Components& currentVel, Components& prevVelLP) {
	Components currentVelLP;
	// x component
	if (currentVel.x < prevVelLP.x - NPVEL || currentVel.x > prevVelLP.x + NPVEL) {
		currentVelLP.x = currentVel.x;
	}
	else {
		currentVelLP.x = 0;
	}
	// y component
	if (currentVel.y < prevVelLP.y - NPVEL || currentVel.y > prevVelLP.y + NPVEL) {
		currentVelLP.y = currentVel.y;
	}
	else {
		currentVelLP.y = 0;
	}
	// z component
	if (currentVel.z < prevVelLP.z - NPVEL || currentVel.z > prevVelLP.z + NPVEL) {
		currentVelLP.z = currentVel.z;
	}
	else {
		currentVelLP.z = 0;
	}
	return currentVelLP;
}

Components MotionSensor::magLPF(Components& currentMag, Components& prevMagLP) {
	Components currentMagLP;
	// x component
	if (currentMag.x < prevMagLP.x - NPMAG || currentMag.x > prevMagLP.x + NPMAG) {
		prevMagLP.x = currentMag.x;
	}
	else {
		if (currentMag.x < prevMagLP.x - NPSMAG || currentMag.x > prevMagLP.x + NPSMAG) {
			prevMagLP.x = (currentMag.x + prevMagLP.x) / 2;
		}
		else {
			prevMagLP.x = (1 - NPLMAG) * prevMagLP.x + NPLMAG * currentMag.x;
		}
	}
	// y component
	if (currentMag.y < prevMagLP.y - NPMAG || currentMag.y > prevMagLP.y + NPMAG) {
		prevMagLP.y = currentMag.y;
	}
	else {
		if (currentMag.y < prevMagLP.y - NPSMAG || currentMag.y > prevMagLP.y + NPSMAG) {
			prevMagLP.y = (currentMag.y + prevMagLP.y) / 2;
		}
		else {
			prevMagLP.y = (1 - NPLMAG) * prevMagLP.y + NPLMAG * currentMag.y;
		}
	}
	// z component
	if (currentMag.z < prevMagLP.z - NPMAG || currentMag.z > prevMagLP.z + NPMAG) {
		prevMagLP.z = currentMag.z;
	}
	else {
		if (currentMag.z < prevMagLP.z - NPSMAG || currentMag.z > prevMagLP.z + NPSMAG) {
			prevMagLP.z = (currentMag.z + prevMagLP.z) / 2;
		}
		else {
			prevMagLP.z = (1 - NPLMAG) * prevMagLP.z + NPLMAG * currentMag.z;
		}
	}
	return currentMagLP;
}

Components MotionSensor::scaleAcc(Components& acc) {
	// Now we'll calculate the accleration value into actual g's. This depends on scale being set.
	return acc * aRes;
}

Components MotionSensor::scaleVel(Components& vel) {
	// Calculate the gyro value into actual degrees per second. This depends on scale being set.
	return vel * gRes;
}

Components MotionSensor::scaleMag(Components& mag) {
	Components scaledMag;
	scaledMag = mag * mRes; // Calculate the magnetometer values in milliGauss. This depends on scale being set.
	// remove factory bias from the magnetometer values.
	scaledMag.x -= magBias[0];
	scaledMag.y -= magBias[1];
	scaledMag.z -= magBias[2];
	return scaledMag;
}

Components MotionSensor::accHPF(std::list<Components>& accs) {
	Components currentAccHP;
	float accsSumX = 0;
	float accsSumY = 0;
	float accsSumZ = 0;
	for (std::list<Components>::iterator it = accs.begin(); it != accs.end(); it++) {
		accsSumX += (*it).x;
		accsSumY += (*it).y;
		accsSumZ += (*it).z;
	}
	currentAccHP.x = accs.back().x - accsSumX / FIR_SIZE;
	currentAccHP.y = accs.back().y - accsSumY / FIR_SIZE;
	currentAccHP.z = accs.back().z - accsSumZ / FIR_SIZE;
	return currentAccHP;
}
