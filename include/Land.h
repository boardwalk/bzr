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
#ifndef BZR_LAND_H
#define BZR_LAND_H

#include "physics/Plane.h"
#include "Landcell.h"

struct Scene;

class Land : public Landcell
{
public:
    static const int kGridSize = 9;
    static const fp_t kCellSize;
    static const fp_t kBlockSize;
    static const int kOffsetMapSize = 64;

    Land(const void* data, size_t size);

    void init();

    fp_t getHeight(int gridX, int gridY) const;
    uint8_t getRoad(int gridX, int gridY) const;
    uint8_t getTerrain(int gridX, int gridY) const;
    uint8_t getTerrainScene(int gridX, int gridY) const;
    bool isSplitNESW(int gridX, int gridY) const;

    Plane calcPlane(fp_t x, fp_t y) const;
    fp_t calcHeight(fp_t x, fp_t y) const;
    fp_t calcHeightUnbounded(fp_t x, fp_t y) const;

    LandcellId id() const override;
    uint32_t numStructures() const;
    const uint8_t* normalMap() const;

private:
    // AC: CLandBlock
    PACK(struct Data
    {
        uint32_t fileId;
        uint32_t flags;
        uint16_t styles[kGridSize][kGridSize];
        uint8_t heightIndices[kGridSize][kGridSize];
        uint8_t pad;
    });

    void initStaticObjects();
    void initScenes();
    void initScene(int x, int y, const Scene& scene);

    Data data_;
    fp_t heights_[kGridSize][kGridSize];
    uint32_t numStructures_;

    vector<uint16_t> offsetMap_;
    fp_t offsetMapBase_;
    fp_t offsetMapScale_;

    vector<uint8_t> normalMap_;
};

#endif