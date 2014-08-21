#ifndef BZR_GRAPHICS_MODELRENDERER_H
#define BZR_GRAPHICS_MODELRENDERER_H

#include "graphics/Program.h"
#include "math/Quat.h"
#include "math/Vec3.h"
#include "Noncopyable.h"
#include "Resource.h"

struct Mat4;
class ModelGroup;
class Model;

class ModelRenderer : Noncopyable
{
public:
    struct DepthSortedModel
    {
        Model* model;
        Vec3 worldPos;
        Quat worldRot;
    };

    ModelRenderer();
    ~ModelRenderer();

    void render(const Mat4& projectionMat, const Mat4& viewMat);

private:
    void renderOne(ResourcePtr& resource, const Mat4& projectionMat, const Mat4& viewMat, const Vec3& position, const Quat& rotation);
    void renderModelGroup(ModelGroup& modelGroup, uint32_t parent, const Mat4& projectionMat, const Mat4& viewMat, const Vec3& position, const Quat& rotation);
    void renderModel(Model& model, const Mat4& projectionMat, const Mat4& viewMat, const Vec3& position, const Quat& rotation, bool firstPass);

    Program _program;
    vector<DepthSortedModel> _depthSortList;
};

#endif