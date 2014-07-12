#include "math/Mat3.h"
#include "math/Mat4.h"

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

// 11 12 13
// 21 22 23
// 31 32 33

// 0 3 6
// 1 4 7 
// 2 5 8

Mat3 Mat3::inverse() const
{
    auto invdet = 1.0 /
         (m[0] * (m[8] * m[4] - m[5] * m[7])
        - m[1] * (m[8] * m[3] - m[5] * m[6])
        + m[2] * (m[7] * m[3] - m[4] * m[6]));

    Mat3 r;

    r.m[0] = invdet *   m[8]* m[4] - m[5] * m[7];//  33,22 32,23
    r.m[1] = invdet * -(m[8]* m[1] - m[2] * m[7]);//- 33,21 31,23
    r.m[2] = invdet *   m[5]* m[1] - m[2] * m[4];//  32,21 31,22

    r.m[3] = invdet * -(m[8]* m[3] - m[5] * m[6]);//- 33,12 32,13
    r.m[4] = invdet *   m[8]* m[0] - m[2] * m[6];//  33,11 31,13
    r.m[5] = invdet * -(m[5]* m[0] - m[2] * m[3]);//- 32,11 31,12

    r.m[6] = invdet *   m[7]* m[3] - m[4] * m[6];//  23,12 22,13
    r.m[7] = invdet * -(m[7]* m[0] - m[1] * m[6]);//- 23,11 21,13
    r.m[8] = invdet *   m[4]* m[0] - m[1] * m[3];//  22,11 21,12

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
