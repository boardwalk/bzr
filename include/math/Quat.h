#ifndef BZR_MATH_QUAT_H
#define BZR_MATH_QUAT_H

#include "math/Vec3.h"

struct Quat
{
	double w;
	double x;
	double y;
	double z;

	void makeFromYawPitchRoll(double yaw, double pitch, double roll);
};

Vec3 operator*(const Quat& q, const Vec3& v);

#endif