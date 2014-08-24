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
#ifndef BZR_MATH_MAT4_H
#define BZR_MATH_MAT4_H

struct Quat;
struct Vec3;

struct Mat4
{
    Mat4();

    void makeIdentity();
    void makeTranslation(const Vec3& v);
    void makeRotation(const Quat& q);
    void makeScale(const Vec3& v);
    void makePerspective(double fovy, double aspect, double zNear, double zFar);

    double m[16];
};

Mat4 operator*(const Mat4& a, const Mat4& b);

#endif
