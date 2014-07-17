#ifndef BZR_LANDBLOCKMANAGER_H
#define BZR_LANDBLOCKMANAGER_H

#include "Landblock.h"
#include "Noncopyable.h"
#include <unordered_map>

class LandblockManager : Noncopyable
{
public:
	using container_type = unordered_map<LandblockId, Landblock>;
	using iterator = container_type::iterator;

	LandblockManager();

	void setCenter(LandblockId center);
	LandblockId getCenter() const;

	void setRadius(int radius);

	iterator begin();
	iterator end();

private:
	void load();

	container_type _data;
	LandblockId _center;
	int _radius;
};

#endif