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
#ifndef BZR_LANDCELLID_H
#define BZR_LANDCELLID_H

/*
 * A landcell id is a 32-bit value identifying a cell
 * Bits 24:31 are x, the 0-255, east to west position of the landblock container the landcell
 * Bits 16:23 are y, the 0-255, south to north position
 * Bits 0:15 are n, the index of the cell
 * 0x0001 <= n <= 0x0040 specify a cell on land, with an origin in the south-west and column-major format
 * 0x0100 <= n < 0xFFFE specify a structure, either above or below ground
 * n = 0x0000 and 0x0040 < n < 0x0100 are not used
 * n = 0xFFFE is reserved for the landblock doodad file
 * n = 0xFFFF is reserved for the landblock landscape file
 * n = 0xFFFF is also used to identify the landblock (the collection of landcells sharing an x and y)
 */
class LandcellId
{
public:
    LandcellId();
    explicit LandcellId(uint32_t value);
    LandcellId(uint8_t x, uint8_t y);
    LandcellId(uint8_t x, uint8_t y, uint16_t n);

    int calcSquareDistance(LandcellId other) const;

    uint8_t x() const;
    uint8_t y() const;
    uint16_t n() const;
    uint32_t value() const;
    bool isStructure() const;

    bool operator==(LandcellId other) const;
    bool operator!=(LandcellId other) const;

private:
    uint32_t value_;
};

namespace std
{
    template<>
    struct hash<LandcellId>
    {
        size_t operator()(LandcellId id) const
        {
            return id.value();
        }
    };
}

#endif