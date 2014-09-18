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

LandcellId::LandcellId(uint8_t x, uint8_t y) :
    value_((uint32_t(x) << 24) | (uint32_t(y) << 16) | 0xFFFF)
{}

LandcellId::LandcellId(uint8_t x, uint8_t y, uint16_t n) :
    value_((uint32_t(x) << 24) | (uint32_t(y) << 16) | n)
{}

int LandcellId::calcSquareDistance(LandcellId other) const
{
    auto dx = other.x() - x();
    auto dy = other.y() - y();
    return dx * dx + dy * dy;
}

uint8_t LandcellId::x() const
{
    return uint8_t(value_ >> 24);
}

uint8_t LandcellId::y() const
{
    return uint8_t(value_ >> 16);
}

uint16_t LandcellId::n() const
{
    return uint16_t(value_);
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
