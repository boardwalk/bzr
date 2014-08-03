#ifndef BZR_MODELRENDERER_H
#define BZR_MODELRENDERER_H

#include "Destructable.h"
#include "Noncopyable.h"

class ModelRenderer : Noncopyable
{
public:
    ModelRenderer();
    ~ModelRenderer();

    void render();

private:
    shared_ptr<Destructable> _model;
};

#endif