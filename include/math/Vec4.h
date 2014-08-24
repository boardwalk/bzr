#ifndef BZR_MATH_VEC4_H
#define BZR_MATH_VEC4_H

struct Mat4;

struct Vec4
{
    Vec4();
    Vec4(double vx, double vy, double vz, double vw);

    double x;
    double y;
    double z;
    double w;
};

Vec4 operator*(const Mat4& m, const Vec4& v);

#endif