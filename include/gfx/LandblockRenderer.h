#ifndef BZR_GFX_LANDBLOCKRENDERER_H
#define BZR_GFX_LANDBLOCKRENDERER_H

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
};

#endif
