#ifndef BZR_STRUCTURE_H
#define BZR_STRUCTURE_H

#include "Destructable.h"
#include "Noncopyable.h"
#include "Object.h"
#include <vector>

class StructureGeom;

class Structure : Noncopyable
{
public:
    Structure(const void* data, size_t size);
    Structure(Structure&& other);

    const Vec3& position() const;
    const Quat& rotation() const;
    const vector<ResourcePtr>& textures() const;
    const vector<Object>& objects() const;
    const StructureGeom& geometry() const;
    uint16_t pieceNum() const;

    unique_ptr<Destructable>& renderData();

private:
    Vec3 _position;
    Quat _rotation;
    vector<ResourcePtr> _textures;
    vector<Object> _objects;
    ResourcePtr _geometry;
    uint16_t _pieceNum;
    unique_ptr<Destructable> _renderData;
};

#endif