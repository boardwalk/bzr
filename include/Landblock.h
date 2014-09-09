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
#ifndef BZR_LANDBLOCK_H
#define BZR_LANDBLOCK_H

#include "Destructable.h"
#include "LandblockId.h"
#include "Noncopyable.h"
#include "Structure.h"

class Landblock : Noncopyable
{
public:
    static const int GRID_SIZE = 9;
    static constexpr fp_t SQUARE_SIZE = 24.0;
    static constexpr fp_t LANDBLOCK_SIZE = 192.0;
    static const int OFFSET_MAP_SIZE = 64;

    PACK(struct Data
    {
        uint32_t fileId;
        uint32_t flags;
        uint16_t styles[GRID_SIZE][GRID_SIZE];
        uint8_t heights[GRID_SIZE][GRID_SIZE];
        uint8_t pad;
    });

    Landblock(const void* data, size_t length);
    Landblock(Landblock&& other);

    void init();

    fp_t calcHeight(fp_t x, fp_t y) const;
    fp_t calcHeightUnbounded(fp_t x, fp_t y) const;

    LandblockId id() const;

    const Data& data() const;
    const vector<Doodad>& doodads() const;
    const vector<Structure>& structures() const;

    const uint8_t* normalMap() const;

    bool isSplitNESW(int x, int y) const;

    unique_ptr<Destructable>& renderData();

private:
    void initDoodads();

    Data _data;
    vector<Doodad> _doodads;
    vector<Structure> _structures;

    vector<uint16_t> _offsetMap;
    fp_t _offsetMapBase;
    fp_t _offsetMapScale;

    vector<uint8_t> _normalMap;

    unique_ptr<Destructable> _renderData;
};

#endif
