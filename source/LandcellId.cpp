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
    _value(0)
{}

LandcellId::LandcellId(uint8_t x, uint8_t y, uint16_t n) :
    _value((uint32_t(x) << 24) | (uint32_t(y) << 16) | n)
{}

uint8_t LandcellId::x() const
{
    return uint8_t(_value >> 24);
}

uint8_t LandcellId::y() const
{
    return uint8_t(_value >> 16);
}

uint16_t LandcellId::n() const
{
    return uint16_t(_value);
}

uint8_t LandcellId::operator[](int axis) const
{
    return uint8_t(_value >> (24 - axis * 8));
}

uint32_t LandcellId::value() const
{
    return _value;
}

bool LandcellId::operator==(LandcellId other) const
{
    return _value == other._value;
}

bool LandcellId::operator!=(LandcellId other) const
{
    return _value != other._value;
}
