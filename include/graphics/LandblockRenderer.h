#ifndef BZR_GRAPHICS_LANDBLOCKRENDERER_H
#define BZR_GRAPHICS_LANDBLOCKRENDERER_H

#include "graphics/LandblockRenderData.h"
#include "graphics/Buffer.h"
#include "graphics/Program.h"

#include "Noncopyable.h"

class Landblock;
struct Mat4;

class LandblockRenderer : public Noncopyable
{
public:
    LandblockRenderer();
    ~LandblockRenderer();

    void render(LandblockRenderData& renderData, const Mat4& projection, const Mat4& modelView);

private:
    void initProgram();
    void initTerrainTexture();
    void initBlendTexture();

    Program _program;

    GLuint _terrainTexture;
    GLuint _blendTexture;
};

#endif
