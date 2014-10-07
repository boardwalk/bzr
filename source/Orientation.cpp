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
#include "Orientation.h"
#include "BinReader.h"
#include <glm/gtx/norm.hpp>

Orientation::Orientation(BinReader& reader)
{
    position.x = reader.readFloat();
    position.y = reader.readFloat();
    position.z = reader.readFloat();

    rotation.w = reader.readFloat();
    rotation.x = reader.readFloat();
    rotation.y = reader.readFloat();
    rotation.z = reader.readFloat();

    assert(glm::length2(rotation) >= 0.99 && glm::length2(rotation) <= 1.01);
}