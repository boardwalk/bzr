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
    void initGeometry(const Landblock& landblock);
    void initOffsetTexture(const Landblock& landblock);
    void initNormalTexture(const Landblock& landblock);

    GLuint _vertexArray;
    GLuint _vertexBuffer;
    GLsizei _vertexCount;

    GLuint _offsetTexture;
    GLfloat _offsetBase;
    GLfloat _offsetScale;

    GLuint _normalTexture;
};

#endif
