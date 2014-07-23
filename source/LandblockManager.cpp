#include "LandblockManager.h"
#include "Core.h"
#include "Config.h"
#include "DatFile.h"

LandblockManager::LandblockManager()
{
    _radius = Core::get().config().getInt("LandblockManager.radius", 8);
}

void LandblockManager::setCenter(LandblockId c)
{
    if(c != _center)
    {
        printf("new center: %02x %02x\n", c.x(), c.y());
        _center = c;
        load();
    }
}

LandblockId LandblockManager::center() const
{
    return _center;
}

void LandblockManager::setRadius(int radius)
{
    assert(radius >= 0);

    if(radius != _radius)
    {
        _radius = radius;
        load();
    }
}

LandblockManager::iterator LandblockManager::find(LandblockId id)
{
    return _data.find(id);
}

LandblockManager::iterator LandblockManager::begin()
{
    return _data.begin();
}

LandblockManager::iterator LandblockManager::end()
{
    return _data.end();
}

void LandblockManager::load()
{
    // we grab more landblocks than we need so the ones that actually
    // get initialized have all their neighbors
    uint8_t sloppyRadius = uint8_t(_radius) + 2;

    for(uint8_t x = _center.x() - sloppyRadius; x <= _center.x() + sloppyRadius; x++)
    {
        for(uint8_t y = _center.y() - sloppyRadius; y <= _center.y() + sloppyRadius; y++)
        {
            LandblockId id(x, y);

            if(_center.calcSquareDistance(id) > sloppyRadius * sloppyRadius)
            {
                continue;
            }

            if(_data.find(id) != _data.end())
            {
                continue;
            }

            auto data = Core::get().cellDat().read(id.fileId());

            if(data.empty())
            {
                 continue;
            }

            auto pair = container_type::value_type(id, Landblock(data.data(), data.size()));
            _data.emplace(move(pair));
        }
    }

    for(auto it = _data.begin(); it != _data.end(); ++it)
    {
        if(_center.calcSquareDistance(it->first) <= _radius * _radius)
        {
            it->second.init();
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
