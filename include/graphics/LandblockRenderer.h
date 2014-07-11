#ifndef BZR_GRAPHICS_LANDBLOCKRENDERER_H
#define BZR_GRAPHICS_LANDBLOCKRENDERER_H

#include "graphics/Texture.h"
#include "IDestructable.h"
#include "Noncopyable.h"

class Landblock;

class LandblockRenderer : public IDestructable
{
public:
    LandblockRenderer(const Landblock& landblock);
    ~LandblockRenderer();

    void render();

private:
    GLuint _vertexBuffer;
    GLuint _indexBuffer;
    GLsizei _indexCount;

    Texture _texture;
};

#endif
