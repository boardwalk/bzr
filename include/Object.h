#ifndef BZR_OBJECT_H
#define BZR_OBJECT_H

#include "math/Quat.h"
#include "math/Vec3.h"
#include "Resource.h"

struct Object
{
    ResourcePtr resource;
    Vec3 position;
    Quat rotation;
};

#endif