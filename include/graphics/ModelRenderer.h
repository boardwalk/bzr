#ifndef BZR_GRAPHICS_MODELRENDERER_H
#define BZR_GRAPHICS_MODELRENDERER_H

#include "graphics/Program.h"
#include "math/Mat4.h"
#include "Noncopyable.h"
#include "Resource.h"

class ModelGroup;
class Model;

class ModelRenderer : Noncopyable
{
public:
    struct DepthSortedModel
    {
        Model* model;
        Mat4 worldMat;
    };

    ModelRenderer();
    ~ModelRenderer();

    void render(const Mat4& projectionMat, const Mat4& viewMat);

private:
    void renderOne(ResourcePtr& resource,
        const Mat4& projectionMat,
        const Mat4& viewMat,
        const Mat4& worldMat);

    void renderModelGroup(ModelGroup& modelGroup,
        const Mat4& projectionMat,
        const Mat4& viewMat,
        const Mat4& worldMat);

    void renderModel(Model& model,
        const Mat4& projectionMat,
        const Mat4& viewMat,
        const Mat4& worldMat,
        bool firstPass);

    Program _program;
    vector<DepthSortedModel> _depthSortList;
};

#endif