#ifndef BZR_LANDBLOCKMANAGER_H
#define BZR_LANDBLOCKMANAGER_H

#include "Landblock.h"
#include "Noncopyable.h"
#include <unordered_map>

class LandblockManager : Noncopyable
{
public:
    typedef unordered_map<LandblockId, Landblock> container_type;
    typedef container_type::iterator iterator;

    LandblockManager();

    void setCenter(LandblockId center);
    LandblockId center() const;

    void setRadius(int radius);

    iterator find(LandblockId id);
    iterator begin();
    iterator end();

private:
    void load();

    container_type _data;
    LandblockId _center;
    int _radius;
};

#endif
