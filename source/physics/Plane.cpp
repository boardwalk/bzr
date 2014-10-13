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
#include "physics/Plane.h"
#include "BinReader.h"
#include "util.h"

static const fp_t kEpsilon = fp_t(0.0002);

Plane::Plane()
{}

Plane::Plane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    normal = glm::normalize(glm::cross(b - a, c - a));
    dist = -(a.x * normal.x + a.y * normal.y + a.z * normal.z);
}

fp_t Plane::calcZ(fp_t x, fp_t y)
{
    if(normal.z <= kEpsilon)
    {
        return fp_t(0.0);
    }

    return -(x * normal.x + y * normal.y + dist) / normal.z;
}

void read(BinReader& reader, Plane& plane)
{
    read(reader, plane.normal);
    plane.dist = reader.readFloat();
}
