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
#include "LandcellId.h"

LandcellId::LandcellId() :
    value_(0)
{}

LandcellId::LandcellId(uint32_t value) :
    value_(value)
{}

LandcellId::LandcellId(int x, int y) :
    value_((x << 24) | (y << 16) | 0xFFFFu)
{
    assert(x >= 0 && x <= 0xFF);
    assert(y >= 0 && y <= 0xFF);
}

LandcellId::LandcellId(int x, int y, int n) :
    value_((x << 24) | (y << 16) | n)
{
    assert(x >= 0 && x <= 0xFF);
    assert(y >= 0 && y <= 0xFF);
    assert(n >= 0 && n <= 0xFFFF);
}

int LandcellId::calcSquareDistance(LandcellId other) const
{
    int dx = other.x() - x();
    int dy = other.y() - y();
    return dx * dx + dy * dy;
}

int LandcellId::x() const
{
    return value_ >> 24;
}

int LandcellId::y() const
{
    return (value_ >> 16) & 0xFF;
}

int LandcellId::n() const
{
    return value_ & 0xFFFF;
}

uint32_t LandcellId::value() const
{
    return value_;
}

bool LandcellId::isStructure() const
{
    return n() >= 0x0100 && n() < 0xFFFE;
}

bool LandcellId::operator==(LandcellId other) const
{
    return value_ == other.value_;
}

bool LandcellId::operator!=(LandcellId other) const
{
    return value_ != other.value_;
}
