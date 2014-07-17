#ifndef BZR_GRAPHICS_SKYRENDERER_H
#define BZR_GRAPHICS_SKYRENDERER_H

#include "graphics/Program.h"
#include "Noncopyable.h"

struct Mat4;

class SkyRenderer : Noncopyable
{
public:
    SkyRenderer();
    ~SkyRenderer();

    void render(const Mat4& projMat, const Mat4& viewMat);

private:
    void initProgram();
    void initGeometry();
    void initTexture();

    Program _program;
    GLuint _vertexArray;
    GLuint _vertexBuffer;
    GLsizei _vertexCount;
    GLuint _texture;
};

#endif
