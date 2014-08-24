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
#include "math/Vec3.h"
#include <algorithm>

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

double Vec3::dot(const Vec3& v) const
{
    return x * v.x + y * v.y + z * v.z;
}

double Vec3::squareDist(const Vec3& v) const
{
    auto xd = abs(v.x - x);
    auto yd = abs(v.y - y);
    auto zd = abs(v.z - z);
    return xd * xd + yd * yd + zd * zd;
}

Vec3 operator-(const Vec3& a)
{
    return Vec3(-a.x, -a.y, -a.z);
}

Vec3 operator*(double a, const Vec3& b)
{
    return Vec3(a * b.x, a * b.y, a * b.z);
}

Vec3 operator*(const Vec3& a, double b)
{
    return b * a;
}

Vec3 operator+(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 operator-(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}
