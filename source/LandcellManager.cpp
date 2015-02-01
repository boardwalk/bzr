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
#include "Config.h"
#include "Core.h"
#include "DatFile.h"
#include "Land.h"
#include "Log.h"
#include "Structure.h"
#include <algorithm>

LandcellManager::LandcellManager()
{
    radius_ = Core::get().config().getInt("LandcellManager.radius", 5);
}

void LandcellManager::setCenter(LandcellId c)
{
    if(c != center_)
    {
        LOG(Misc, Info) << "new center=" << c.x() << ", " << c.y() << "\n";
        center_ = c;
        load();
    }
}

LandcellId LandcellManager::center() const
{
    return center_;
}

LandcellManager::iterator LandcellManager::find(LandcellId id)
{
    return data_.find(id);
}

LandcellManager::iterator LandcellManager::begin()
{
    return data_.begin();
}

LandcellManager::iterator LandcellManager::end()
{
    return data_.end();
}

void LandcellManager::load()
{
    // we grab more landblocks than we need so the ones that actually
    // get initialized have all their neighbors
    int sloppyRadius = radius_ + 2;

    for(int x = max(center_.x() - sloppyRadius, 0); x <= min(center_.x() + sloppyRadius, 0xFF); x++)
    {
        for(int y = max(center_.y() - sloppyRadius, 0); y <= min(center_.y() + sloppyRadius, 0xFF); y++)
        {
            LandcellId landId(x, y);

            if(center_.calcSquareDistance(landId) > sloppyRadius * sloppyRadius)
            {
                continue;
            }

            if(data_.find(landId) != data_.end())
            {
                continue;
            }

            vector<uint8_t> data = Core::get().cellDat().read(landId.value());

            if(data.empty())
            {
                continue;
            }

            data_[landId].reset(new Land(data.data(), data.size()));
        }
    }

    for(auto it = data_.begin(); it != data_.end(); ++it)
    {
        if(center_.calcSquareDistance(it->first) <= radius_ * radius_)
        {
            if(it->first.isStructure())
            {
                continue;
            }

            Land& land = static_cast<Land&>(*it->second);

            land.init();

            for(uint32_t i = 0; i < land.numStructures(); i++)
            {
                LandcellId structId(land.id().x(), land.id().y(), (uint16_t)(0x0100 + i));

                if(data_.find(structId) != data_.end())
                {
                    continue;
                }

                vector<uint8_t> data = Core::get().cellDat().read(structId.value());

                if(data.empty())
                {
                    throw runtime_error("Structure not found");
                }

                data_[structId].reset(new Structure(data.data(), data.size()));
            }
        }
    }

    for(auto it = data_.begin(); it != data_.end(); /**/)
    {
        if(center_.calcSquareDistance(it->first) > radius_ * radius_)
        {
            it = data_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
