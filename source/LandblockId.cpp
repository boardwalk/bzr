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
#include "LandblockId.h"

LandblockId::LandblockId() : _x(0), _y(0)
{}

LandblockId::LandblockId(uint8_t lx, uint8_t ly) : _x(lx), _y(ly)
{}

uint8_t LandblockId::x() const
{
    return _x;
}

uint8_t LandblockId::y() const
{
    return _y;
}

uint32_t LandblockId::fileId() const
{
    return (uint32_t)_x << 24 | (uint32_t)_y << 16 | 0xFFFF;
}

int LandblockId::calcSquareDistance(LandblockId other) const
{
    auto dx = other.x() - x();
    auto dy = other.y() - y();
    return dx * dx + dy * dy;
}

bool LandblockId::operator==(LandblockId other) const
{
    return _x == other._x && _y == other._y;
}

bool LandblockId::operator!=(LandblockId other) const
{
    return _x != other._x || _y != other._y;
}
