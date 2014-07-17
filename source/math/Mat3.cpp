#include "math/Mat3.h"
#include "math/Mat4.h"
#include "math/Vec3.h"

Mat3::Mat3()
{
    memset(m, 0, sizeof(m));
}

Mat3::Mat3(const Mat4& other)
{
    m[0] = other.m[0];
    m[1] = other.m[1];
    m[2] = other.m[2];

    m[3] = other.m[4];
    m[4] = other.m[5];
    m[5] = other.m[6];

    m[6] = other.m[8];
    m[7] = other.m[9];
    m[8] = other.m[10];
}

Mat3 Mat3::inverse() const
{
    auto invdet = 1.0 /
         (m[0] * (m[8] * m[4] - m[5] * m[7])
        - m[1] * (m[8] * m[3] - m[5] * m[6])
        + m[2] * (m[7] * m[3] - m[4] * m[6]));

    Mat3 r;

    r.m[0] = invdet *   m[8]* m[4] - m[5] * m[7];
    r.m[1] = invdet * -(m[8]* m[1] - m[2] * m[7]);
    r.m[2] = invdet *   m[5]* m[1] - m[2] * m[4];

    r.m[3] = invdet * -(m[8]* m[3] - m[5] * m[6]);
    r.m[4] = invdet *   m[8]* m[0] - m[2] * m[6];
    r.m[5] = invdet * -(m[5]* m[0] - m[2] * m[3]);

    r.m[6] = invdet *   m[7]* m[3] - m[4] * m[6];
    r.m[7] = invdet * -(m[7]* m[0] - m[1] * m[6]);
    r.m[8] = invdet *   m[4]* m[0] - m[1] * m[3];

    return r;
}

Mat3 Mat3::transpose() const
{
    Mat3 r;

    r.m[0] = m[0];
    r.m[1] = m[3];
    r.m[2] = m[6];

    r.m[3] = m[1];
    r.m[4] = m[4];
    r.m[5] = m[7];

    r.m[6] = m[2];
    r.m[7] = m[5];
    r.m[8] = m[8];

    return r;
}

Vec3 operator*(const Mat3& mat, const Vec3& vec)
{
    Vec3 res;

    res.x = vec.x * mat.m[0] + vec.y * mat.m[3] + vec.z * mat.m[6];
    res.y = vec.x * mat.m[1] + vec.y * mat.m[4] + vec.z * mat.m[7];
    res.z = vec.x * mat.m[2] + vec.y * mat.m[5] + vec.z * mat.m[8];

    return res;
}

