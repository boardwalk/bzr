#ifndef BZR_GRAPHICS_MESHRENDERDATA_H
#define BZR_GRAPHICS_MESHRENDERDATA_H

#include "Destructable.h"
#include "Noncopyable.h"
#include "Resource.h"
#include <vector>

class Model;
class Structure;
struct Vertex;
struct TriangleFan;

class MeshRenderData : public Destructable, Noncopyable
{
public:
    MeshRenderData(const Model& model);
    MeshRenderData(const Structure& structure);
    ~MeshRenderData();

    void bind();

    GLsizei indexCount() const;

private:
    void initGeometry(const vector<ResourcePtr>& textures, const vector<Vertex>& vertices, const vector<TriangleFan>& triangleFans);
    
    GLuint _vertexArray;
    GLuint _vertexBuffer;
    GLuint _indexBuffer;
    GLsizei _indexCount;
};

#endif