#ifndef BZR_MODELRENDERER_H
#define BZR_MODELRENDERER_H

#include "graphics/Program.h"
#include "Noncopyable.h"
#include "Resource.h"

struct Mat4;
class ModelGroup;
class Model;
struct Quat;
struct Vec3;

class ModelRenderer : Noncopyable
{
public:
    ModelRenderer();
    ~ModelRenderer();

    void render(const Mat4& projectionMat, const Mat4& viewMat);

private:
	void renderOne(ResourcePtr& resource, const Mat4& projectionMat, const Mat4& viewMat, const Vec3& position, const Quat& rotation);
	void renderModelGroup(ModelGroup& modelGroup, uint32_t parent, const Mat4& projectionMat, const Mat4& viewMat, const Vec3& position, const Quat& rotation);
	void renderModel(Model& model, const Mat4& projectionMat, const Mat4& viewMat, const Vec3& position, const Quat& rotation);

    Program _program;
};

#endif