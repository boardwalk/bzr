#include "LandblockManager.h"
#include "Core.h"
#include "DatFile.h"

LandblockManager::LandblockManager() : _center(0x31, 0xD6), _radius(3)
{
	load();
}

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
	// cull existing landblocks
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

	// load missing landblocks
	for(auto x = _center.x() - _radius; x <= _center.x() + _radius; x++)
	{
		for(auto y = _center.y() - _radius; y <= _center.y() + _radius; y++)
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

			if(_center.calcSquareDistance(id) > _radius * _radius)
			{
				continue;
			}

			if(_data.find(id) != _data.end())
			{
				continue;
			}

			auto data = Core::get().cellDat().read(id.fileId());
			auto pair = container_type::value_type(id, Landblock(data.data(), data.size()));
			_data.emplace(move(pair));
		}
	}
}
