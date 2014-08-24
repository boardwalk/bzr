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
#include "math/Vec4.h"
#include "math/Mat4.h"

Vec4::Vec4() : x(0.0), y(0.0), z(0.0), w(0.0)
{}

Vec4::Vec4(double vx, double vy, double vz, double vw) : x(vx), y(vy), z(vz), w(vw)
{}

Vec4 operator*(const Mat4& m, const Vec4& v)
{
    return Vec4(
        m.m[0] * v.x + m.m[4] * v.y * m.m[8] * v.z + m.m[12] * v.w,
        m.m[1] * v.x + m.m[5] * v.y * m.m[9] * v.z + m.m[13] * v.w,
        m.m[2] * v.x + m.m[6] * v.y * m.m[10] * v.z + m.m[14] * v.w,
        m.m[3] * v.x + m.m[7] * v.y * m.m[11] * v.z + m.m[15] * v.w);
}
