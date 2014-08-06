#ifndef BZR_MODELRENDERDATA_H
#define BZR_MODELRENDERDATA_H

#include "math/Vec2.h"
#include "Destructable.h"
#include "Noncopyable.h"
#include <vector>

class Model;

class ModelRenderData : public Destructable, Noncopyable
{
public:
    ModelRenderData(const Model& model);
    ~ModelRenderData();

    void bind();

    GLsizei indexCount() const;

private:
    void initTexture(const Model& model, vector<Vec2>& texCoordScales);
    void initGeometry(const Model& model, const vector<Vec2>& texCoordScales);

    GLuint _vertexArray;
    GLuint _vertexBuffer;
    GLuint _indexBuffer;
    GLsizei _indexCount;
    GLuint _texture;
};

#endif