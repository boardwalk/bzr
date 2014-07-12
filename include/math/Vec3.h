#ifndef BZR_MATH_VEC3_H
#define BZR_MATH_VEC3_H

struct Vec3
{
    Vec3();
    Vec3(double vx, double vy, double vz);
   
    double x;
    double y;
    double z;
};

Vec3 operator*(double a, const Vec3& b);
Vec3 operator*(const Vec3& a, double b);
Vec3 operator+(const Vec3& a, const Vec3& b);
Vec3 cross(const Vec3& a, const Vec3& b);

#endif
