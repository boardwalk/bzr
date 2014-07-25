#include "LandblockManager.h"
#include "Core.h"
#include "DatFile.h"

LandblockManager::LandblockManager() : _radius(8)
{}

void LandblockManager::setCenter(LandblockId center)
{
    if(center != _center)
    {
        printf("new center: %02x %02x\n", center.x(), center.y());
        _center = center;
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
    auto sloppyRadius = _radius + 2;

    for(auto x = _center.x() - sloppyRadius; x <= _center.x() + sloppyRadius; x++)
    {
        for(auto y = _center.y() - sloppyRadius; y <= _center.y() + sloppyRadius; y++)
        {
            LandblockId id(x, y);

            if(x < 0x00 || x > 0xFF)
            {
                continue;
            }

            if(y < 0x00 || y > 0xFF)
            {
                continue;
            }

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
