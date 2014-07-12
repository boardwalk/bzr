#ifndef BZR_MATH_MAT4_H
#define BZR_MATH_MAT4_H

struct Vec3;
struct Quat;

struct Mat4
{
	double m[16];

	void makeIdentity();
	void makeTranslation(const Vec3& v);
	void makeRotation(const Quat& q);
    void makePerspective(double fovy, double aspect, double zNear, double zFar);
};

#endif
