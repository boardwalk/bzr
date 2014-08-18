#ifndef BZR_STRUCTURE_H
#define BZR_STRUCTURE_H

#include "Noncopyable.h"
#include "Object.h"
#include <vector>

class StructureGeom;

class Structure : Noncopyable
{
public:
    Structure(const void* data, size_t size);
    Structure(Structure&& other);

    const vector<ResourcePtr>& textures() const;
    const vector<Object>& objects() const;
    const StructureGeom& geometry() const;

private:
    vector<ResourcePtr> _textures;
    vector<Object> _objects;
    ResourcePtr _geometry;
};

#endif