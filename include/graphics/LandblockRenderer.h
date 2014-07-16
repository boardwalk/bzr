#ifndef BZR_GRAPHICS_LANDBLOCKRENDERER_H
#define BZR_GRAPHICS_LANDBLOCKRENDERER_H

#include "graphics/Buffer.h"
#include "graphics/Program.h"
#include "Destructable.h"
#include "Noncopyable.h"

class Landblock;
struct Mat4;

class LandblockRenderer : public Destructable, Noncopyable
{
public:
    LandblockRenderer(const Landblock& landblock);
    ~LandblockRenderer();

    void render(const Mat4& projection, const Mat4& modelView);

private:
    void initVAO(const Landblock& landblock);
    void initProgram();
    void initTerrainTexture();
    void initBlendTexture();
    void initHeightTexture(const Landblock& landblock);

    Program _program;

    GLuint _vertexArray;
    GLuint _vertexBuffer;
    GLsizei _vertexCount;

    GLuint _terrainTexture;
    GLuint _blendTexture;
    GLuint _heightTexture;
};

#endif
