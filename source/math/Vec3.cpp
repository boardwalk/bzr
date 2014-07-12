#include "math/Vec3.h"

Vec3::Vec3() : x(0.0), y(0.0), z(0.0)
{}

Vec3::Vec3(double vx, double vy, double vz) : x(vx), y(vy), z(vz)
{}

Vec3 operator+(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 operator*(double a, const Vec3& b)
{
    return Vec3(a * b.x, a * b.y, a * b.z);
}

Vec3 operator*(const Vec3& a, double b)
{
    return b * a;
}

Vec3 cross(const Vec3& a, const Vec3& b)
{
    return Vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);
}
