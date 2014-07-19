#include "math/Quat.h"
#include "math/Vec3.h"

Quat::Quat() :
    w(1.0), x(0.0), y(0.0), z(0.0)
{}

Quat::Quat(double qw, double qx, double qy, double qz) :
    w(qw), x(qx), y(qy), z(qz)
{}

void Quat::makeFromYawPitchRoll(double yaw, double pitch, double roll)
{
    w = cos(pitch)*cos(yaw)*cos(roll) + sin(pitch)*sin(yaw)*sin(roll);
    x = sin(pitch)*cos(yaw)*cos(roll) - cos(pitch)*sin(yaw)*sin(roll);
    y = cos(pitch)*sin(yaw)*cos(roll) + sin(pitch)*cos(yaw)*sin(roll);
    z = cos(pitch)*cos(yaw)*sin(roll) - sin(pitch)*sin(yaw)*cos(roll);
}

Quat Quat::conjugate() const
{
   return Quat(w, -x, -y, -z);
}

Quat operator*(const Quat& a, const Quat& b)
{
    return Quat(
      a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
      a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
      a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
      a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w);
}

Vec3 operator*(const Quat& q, const Vec3& v)
{
    // http://molecularmusings.wordpress.com/2013/05/24/a-faster-Quat-Vec3-multiplication/
    auto t = 2.0 * Vec3(q.x, q.y, q.z).cross(v);
    return v + q.w * t + Vec3(q.x, q.y, q.z).cross(t);
}
