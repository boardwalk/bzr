#ifndef BZR_GRAPHICS_LANDBLOCKRENDERER_H
#define BZR_GRAPHICS_LANDBLOCKRENDERER_H

#include "graphics/LandblockRenderData.h"
#include "graphics/Program.h"
#include "math/Vec3.h"
#include "Noncopyable.h"

class Landblock;
struct Mat4;

class LandblockRenderer : Noncopyable
{
public:
    LandblockRenderer();
    ~LandblockRenderer();

    void render(const Mat4& projectionMat, const Mat4& viewMat);

    void setLightPosition(const Vec3& lightPosition);

private:
    void initProgram();
    void initTerrainTexture();
    void initBlendTexture();

    Program _program;

    GLuint _terrainTexture;
    GLuint _blendTexture;

    Vec3 _lightPosition;
};

#endif
