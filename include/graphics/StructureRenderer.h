#ifndef BZR_GRAPHICS_STRUCTURERENDERER_H
#define BZR_GRAPHICS_STRUCTURERENDERER_H

#include "graphics/Program.h"
#include "Noncopyable.h"

struct Mat4;
struct Vec3;
struct Quat;
class Structure;

class StructureRenderer : Noncopyable
{
public:
    StructureRenderer();
    ~StructureRenderer();

    void render(const Mat4& projectionMat, const Mat4& viewMat);

private:
    void renderStructure(Structure& structure, const Mat4& projectionMat, const Mat4& view, const Vec3& position, const Quat& rotation);

    Program _program;
};

#endif