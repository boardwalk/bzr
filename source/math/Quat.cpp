#include "math/Quat.h"
#include "math/Vec3.h"

void Quat::makeFromYawPitchRoll(double yaw, double pitch, double roll)
{
    w = cos(pitch)*cos(yaw)*cos(roll) + sin(pitch)*sin(yaw)*sin(roll);
    x = sin(pitch)*cos(yaw)*cos(roll) - cos(pitch)*sin(yaw)*sin(roll);
    y = cos(pitch)*sin(yaw)*cos(roll) + sin(pitch)*cos(yaw)*sin(roll);
    z = cos(pitch)*cos(yaw)*sin(roll) - sin(pitch)*sin(yaw)*cos(roll);
}

Vec3 operator*(const Quat& q, const Vec3& v)
{
    // http://molecularmusings.wordpress.com/2013/05/24/a-faster-Quat-Vec3-multiplication/
    auto t = 2.0 * Vec3(q.x, q.y, q.z).cross(v);
    return v + q.w * t + Vec3(q.x, q.y, q.z).cross(t);
}