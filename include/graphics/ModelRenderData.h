#ifndef BZR_MODELRENDERDATA_H
#define BZR_MODELRENDERDATA_H

#include "Destructable.h"
#include "Noncopyable.h"

class SimpleModel;

class ModelRenderData : public Destructable, Noncopyable
{
public:
    ModelRenderData(const SimpleModel& model);
    ~ModelRenderData();

    void render();

private:
    GLuint _vertexArray;
    GLuint _vertexBuffer;
    GLuint _indexBuffer;
    GLsizei _indexCount;
};

#endif