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
#include "Location.h"
#include "BinReader.h"
#include <glm/gtx/norm.hpp>

void read(BinReader& reader, Location& loc)
{
    loc.position.x = reader.readFloat();
    loc.position.y = reader.readFloat();
    loc.position.z = reader.readFloat();

    loc.rotation.w = reader.readFloat();
    loc.rotation.x = reader.readFloat();
    loc.rotation.y = reader.readFloat();
    loc.rotation.z = reader.readFloat();

    assert(glm::length2(loc.rotation) >= 0.99 && glm::length2(loc.rotation) <= 1.01);
}
