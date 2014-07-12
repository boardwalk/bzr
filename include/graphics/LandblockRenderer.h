#ifndef BZR_GRAPHICS_LANDBLOCKRENDERER_H
#define BZR_GRAPHICS_LANDBLOCKRENDERER_H

#include "graphics/Buffer.h"
#include "graphics/Texture.h"
#include "IDestructable.h"
#include "Noncopyable.h"

class Landblock;

class LandblockRenderer : public IDestructable, Noncopyable
{
public:
    LandblockRenderer(const Landblock& landblock);
    ~LandblockRenderer();

    void render();

private:
    Buffer _vertexBuffer;
    Buffer _indexBuffer;
    GLsizei _indexCount;

    Texture _texture;
};

#endif
