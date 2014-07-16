#ifndef BZR_GRAPHICS_LANDBLOCKRENDERDATA_H
#define BZR_GRAPHICS_LANDBLOCKRENDERDATA_H

#include "Destructable.h"
#include "Noncopyable.h"

class Landblock;
class Program;

class LandblockRenderData : public Destructable, Noncopyable
{
public:
    LandblockRenderData(const Landblock& landblock);
    ~LandblockRenderData();

    void bind(Program& program);

    GLsizei vertexCount() const;

private:
    void initVAO(const Landblock& landblock);
    void initHeightTexture(const Landblock& landblock);

    GLuint _vertexArray;
    GLuint _vertexBuffer;
    GLsizei _vertexCount;

    GLuint _heightTexture;
    GLfloat _heightBase;
    GLfloat _heightScale;
};

#endif