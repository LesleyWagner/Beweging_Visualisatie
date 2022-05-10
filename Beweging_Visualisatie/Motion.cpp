#include "Motion.h"
#include "Arduino.h"

Components Motion::calcPosition(const Components currentAcc, const Components orientation, const Components prevPosition, double dt) {
	// remove gravity from the acceleration components, a0 = previous acceleration, a1 = current acceleration
	double yaw = orientation.x;
	double pitch = orientation.y;
	double roll = orientation.z;
	double g_xz = cos(roll);
	double g_x = g_xz * sin(pitch);
	double g_y = sin(roll);
	double g_z = g_xz * cos(pitch);
	double a1_x = currentAcc.x - g_x;
	double a1_y = currentAcc.y - g_y;
	double a1_z = currentAcc.z - g_z;
	/*Serial.print("a.x: ");
	Serial.println(a1_x);
	Serial.print("a.y: ");
	Serial.println(a1_y);
	Serial.print("a.z: ");
	Serial.println(a1_z);*/
	// trapezoidal integration to calculate velocity
	double v1_x = v0_x + (a1_x + a0_x) * dt / 2;
	double v1_y = v0_y + (a1_y + a0_y) * dt / 2;
	double v1_z = v0_z + (a1_z + a0_z) * dt / 2;
	a0_x = a1_x;
	a0_y = a1_y;
	a0_z = a1_z;
	Components position;
	// trapezoidal integration to calculate position
	position.x = prevPosition.x + (v1_x + v0_x) * dt / 2;
	position.y = prevPosition.y + (v1_y + v0_y) * dt / 2;
	position.z = prevPosition.z + (v1_z + v0_z) * dt / 2;
	/*Serial.print("p.x: ");
	Serial.println(position.x);
	Serial.print("p.y: ");
	Serial.println(position.y);
	Serial.print("p.z: ");
	Serial.println(position.z);*/
	v0_x = v1_x;
	v0_y = v1_y;
	v0_z = v1_z;

	return position;
}

Components Motion::calcOrientation(const Components acc, const Components vel, const Components mag, float dt)
{
	Components orientation;

	// Sensors x (y)-axis of the accelerometer is aligned with the y (x)-axis of
	// the magnetometer; the magnetometer z-axis (+ down) is opposite to z-axis
	// (+ up) of accelerometer and gyro! We have to make some allowance for this
	// orientationmismatch in feeding the output to the quaternion filter. For the
	// MPU-9250, we have chosen a magnetic rotation that keeps the sensor forward
	// along the x-axis just like in the LSM9DS0 sensor. This rotation can be
	// modified to allow any convenient orientation convention. This is ok by
	// aircraft orientation standards! Pass gyro rate as rad/s
	MadgwickQuaternionUpdate(acc.x, acc.y, acc.z, vel.x * DEG_TO_RAD,
		vel.y * DEG_TO_RAD, vel.z * DEG_TO_RAD, mag.y,
		mag.x, mag.z, dt);

	// Define output variables from updated quaternion---these are Tait-Bryan
	// angles, commonly used in aircraft orientation. In this coordinate system,
	// the positive z-axis is down toward Earth. Yaw is the angle between Sensor
	// x-axis and Earth magnetic North (or true North if corrected for local
	// declination, looking down on the sensor positive yaw is counterclockwise.
	// Pitch is angle between sensor x-axis and Earth ground plane, toward the
	// Earth is positive, up toward the sky is negative. Roll is angle between
	// sensor y-axis and Earth ground plane, y-axis up is positive roll. These
	// arise from the definition of the homogeneous rotation matrix constructed
	// from quaternions. Tait-Bryan angles as well as Euler angles are
	// non-commutative; that is, the get the correct orientation the rotations
	// must be applied in the correct order which for this configuration is yaw,
	// pitch, and then roll.
	// For more see
	// http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	// which has additional links.
	orientation.x = atan2(2.0f * (*(getQ() + 1) * *(getQ() + 2) + *getQ()
		* *(getQ() + 3)), *getQ() * *getQ() + *(getQ() + 1)
		* *(getQ() + 1) - *(getQ() + 2) * *(getQ() + 2) - *(getQ() + 3)
		* *(getQ() + 3));
	orientation.y = -asin(2.0f * (*(getQ() + 1) * *(getQ() + 3) - *getQ()
		* *(getQ() + 2)));
	orientation.z = atan2(2.0f * (*getQ() * *(getQ() + 1) + *(getQ() + 2)
		* *(getQ() + 3)), *getQ() * *getQ() - *(getQ() + 1)
		* *(getQ() + 1) - *(getQ() + 2) * *(getQ() + 2) + *(getQ() + 3)
		* *(getQ() + 3));
	orientation.y *= RAD_TO_DEG;
	orientation.x *= RAD_TO_DEG;

	// Declination of keizerstraat 76 (51°36'03.4"N 5°19'18.4"E) is
	//    2.19° E  ± 0.39°  changing by  0.19° E per year on 18-01-2022
	// - http://www.ngdc.noaa.gov/geomag-web/#declination
	orientation.x -= 2.2;
	orientation.z *= RAD_TO_DEG;

	return orientation;
}

Motion::Motion() {}
