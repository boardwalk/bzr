/*
 * Bael'Zharon's Respite
 * Copyright (C) 2014 Daniel Skorupski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "math/Mat4.h"
#include "math/Vec3.h"
#include "math/Quat.h"

Mat4::Mat4()
{
    memset(m, 0, sizeof(m));
}

void Mat4::makeIdentity()
{
    memset(m, 0, sizeof(m));

    m[0] = 1.0;
    m[5] = 1.0;
    m[10] = 1.0;
    m[15] = 1.0;
}

void Mat4::makeTranslation(const Vec3& v)
{
    makeIdentity();

    m[12] = v.x;
    m[13] = v.y;
    m[14] = v.z;
}

void Mat4::makeScale(const Vec3& v)
{
    memset(m, 0, sizeof(m));

    m[0] = v.x;
    m[5] = v.y;
    m[10] = v.z;
    m[15] = 1.0;
}

void Mat4::makeRotation(const Quat& q)
{
    m[0] = 1.0 - 2.0 * (q.y * q.y + q.z * q.z);
    m[1] = 2.0 * (q.x * q.y + q.w * q.z);
    m[2] = 2.0 * (q.x * q.z - q.w * q.y);
    m[3] = 0.0;

    m[4] = 2.0 * (q.x * q.y - q.w * q.z);
    m[5] = 1.0 - 2.0 * (q.x * q.x + q.z * q.z);
    m[6] = 2.0 * (q.y * q.z + q.w * q.x);
    m[7] = 0.0;

    m[8] = 2.0 * (q.x * q.z + q.w * q.y);
    m[9] = 2.0 * (q.y * q.z - q.w * q.x);
    m[10] = 1.0 - 2.0 * (q.x * q.x + q.y * q.y);
    m[11] = 0.0;

    m[12] = 0.0;
    m[13] = 0.0;
    m[14] = 0.0;
    m[15] = 1.0;
}

//
// on coordinates
// our coordinate system is:
// +x goes right
// +y goes up
// +z goes out of the screen
// this is "right handed"
// http://www.songho.ca/opengl/gl_projectionmatrix.html
//
void Mat4::makePerspective(double fovy, double aspect, double zNear, double zFar)
{
    memset(m, 0, sizeof(double) * 16);

    auto f = 1.0 / tan(fovy * PI / 360.0);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = -(zFar + zNear) / (zFar - zNear);
    m[11] = -1.0;
    m[14] = -(2.0 * zFar * zNear) / (zFar - zNear);
}

Mat4 operator*(const Mat4& a, const Mat4& b)
{
    Mat4 res;
   
    for(auto row = 0; row < 4; row++)
    {
        for(auto col = 0; col < 4; col++)
        {
            for(auto i = 0; i < 4; i++)
            {
                res.m[col * 4 + row] = res.m[col * 4  + row] + a.m[i * 4 + row] * b.m[col * 4 + i];
            }
        }
    }

    return res;
}
