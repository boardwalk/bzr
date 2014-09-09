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
#include "LandcellManager.h"
#include "Core.h"
#include "Config.h"
#include "DatFile.h"
#include "Land.h"
#include "Structure.h"

LandcellManager::LandcellManager()
{
    _radius = Core::get().config().getInt("LandcellManager.radius", 5);
}

void LandcellManager::setCenter(LandcellId c)
{
    if(c != _center)
    {
        printf("new center: %02x %02x\n", c.x(), c.y());
        _center = c;
        load();
    }
}

LandcellId LandcellManager::center() const
{
    return _center;
}

void LandcellManager::setRadius(int radius)
{
    assert(radius >= 0);

    if(radius != _radius)
    {
        _radius = radius;
        load();
    }
}

LandcellManager::iterator LandcellManager::find(LandcellId id)
{
    return _data.find(id);
}

LandcellManager::iterator LandcellManager::begin()
{
    return _data.begin();
}

LandcellManager::iterator LandcellManager::end()
{
    return _data.end();
}

void LandcellManager::load()
{
    // we grab more landblocks than we need so the ones that actually
    // get initialized have all their neighbors
    uint8_t sloppyRadius = uint8_t(_radius) + 2;

    for(uint8_t x = _center.x() - sloppyRadius; x <= _center.x() + sloppyRadius; x++)
    {
        for(uint8_t y = _center.y() - sloppyRadius; y <= _center.y() + sloppyRadius; y++)
        {
            LandcellId landId(x, y);

            if(_center.calcSquareDistance(landId) > sloppyRadius * sloppyRadius)
            {
                continue;
            }

            if(_data.find(landId) != _data.end())
            {
                continue;
            }

            auto data = Core::get().cellDat().read(landId.value());

            if(data.empty())
            {
                 continue;
            }

            _data[landId].reset(new Land(data.data(), data.size()));
        }
    }

    for(auto it = _data.begin(); it != _data.end(); ++it)
    {
        if(_center.calcSquareDistance(it->first) <= _radius * _radius)
        {
            if(it->first.isStructure())
            {
                continue;
            }

            Land& land = static_cast<Land&>(*it->second);

            land.init();

            for(auto i = 0u; i < land.numStructures(); i++)
            {
                auto structId = LandcellId(land.id().x(), land.id().y(), (uint16_t)(0x0100 + i));

                auto data = Core::get().cellDat().read(structId.value());

                if(data.empty())
                {
                    throw runtime_error("Structure not found");
                }

                _data[structId].reset(new Structure(data.data(), data.size()));
            }
        }
    }

    for(auto it = _data.begin(); it != _data.end(); /**/)
    {
        if(_center.calcSquareDistance(it->first) > _radius * _radius)
        {
            it = _data.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
