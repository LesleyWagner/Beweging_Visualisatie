#pragma once

/*
* Represents any property that can have directional x, y and z components.
*/
struct Components {
	float x;
	float y;
	float z;

	friend constexpr Components operator*(const Components& lhs, const Components& rhs);
	friend constexpr Components operator+(const Components& lhs, const Components& rhs);
	friend constexpr Components operator-(const Components& lhs, const Components& rhs);
	friend constexpr Components operator*(const Components& lhs, float scalar);
	friend constexpr Components operator*(float scalar, const Components& rhs);
};

constexpr Components operator*(const Components& lhs, const Components& rhs) {
	double x = lhs.x * rhs.x;
	double y = lhs.y * rhs.y;
	double z = lhs.z * rhs.z;
	return Components{ (float)x, (float)y, (float)z };
}

constexpr Components operator+(const Components& lhs, const Components& rhs) {
	double x = lhs.x + rhs.x;
	double y = lhs.y + rhs.y;
	double z = lhs.z + rhs.z;
	return Components{ (float)x, (float)y, (float)z };
}

constexpr Components operator-(const Components& lhs, const Components& rhs) {
	double x = lhs.x - rhs.x;
	double y = lhs.y - rhs.y;
	double z = lhs.z - rhs.z;
	return Components{ (float)x, (float)y, (float)z };
}

constexpr Components operator*(const Components& lhs, float scalar) {
	double x = lhs.x * scalar;
	double y = lhs.y * scalar;
	double z = lhs.z * scalar;
	return Components{ (float)x, (float)y, (float)z };
}

constexpr Components operator*(float scalar, const Components& rhs) {
	double x = rhs.x * scalar;
	double y = rhs.y * scalar;
	double z = rhs.z * scalar;
	return Components{ (float)x, (float)y, (float)z };
}