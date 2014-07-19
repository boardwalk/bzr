#ifndef BZR_MATH_QUAT_H
#define BZR_MATH_QUAT_H

struct Vec3;

struct Quat
{
    Quat();
    Quat(double qw, double qx, double qy, double qz);

    void makeFromYawPitchRoll(double yaw, double pitch, double roll);

    Quat conjugate() const;

    double w;
    double x;
    double y;
    double z;
};

Quat operator*(const Quat& a, const Quat& b);
Vec3 operator*(const Quat& q, const Vec3& v);

#endif