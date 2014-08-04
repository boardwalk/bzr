#ifndef BZR_MODELRENDERER_H
#define BZR_MODELRENDERER_H

#include "graphics/Program.h"
#include "Noncopyable.h"

struct Mat4;

class ModelRenderer : Noncopyable
{
public:
    ModelRenderer();
    ~ModelRenderer();

    void render(const Mat4& projectionMat, const Mat4& viewMat);

private:
    Program _program;
};

#endif