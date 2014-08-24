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
#ifndef BZR_MATH_VEC3_H
#define BZR_MATH_VEC3_H

struct Vec3
{
    Vec3();
    Vec3(double vx, double vy, double vz);

    double magnitude() const;
    Vec3 normalize() const;
    Vec3 cross(const Vec3& v) const;
    double dot(const Vec3& v) const;
    double squareDist(const Vec3& v) const;

    double x;
    double y;
    double z;
};

Vec3 operator-(const Vec3& a);
Vec3 operator*(double a, const Vec3& b);
Vec3 operator*(const Vec3& a, double b);
Vec3 operator+(const Vec3& a, const Vec3& b);
Vec3 operator-(const Vec3& a, const Vec3& b);

#endif
