#include "math/Vec3.h"

Vec3::Vec3() : x(0.0), y(0.0), z(0.0)
{}

Vec3::Vec3(double vx, double vy, double vz) : x(vx), y(vy), z(vz)
{}

double Vec3::magnitude() const
{
    return sqrt(x * x + y * y + z * z);
}

Vec3 Vec3::normalize() const
{
    auto invMag = 1.0 / magnitude();
    return Vec3(x * invMag, y * invMag, z * invMag);
}

Vec3 Vec3::cross(const Vec3& v) const
{
    return Vec3(
        y * v.z - z * v.y,
        z * v.x - x * v.z,
        x * v.y - y * v.x);
}

Vec3 operator-(const Vec3& a)
{
    return Vec3(-a.x, -a.y, -a.z);
}

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

