#include "graphics/ModelRenderer.h"
#include "Core.h"
#include "ResourceCache.h"

ModelRenderer::ModelRenderer()
{
    _model = Core::get().resourceCache().get(0x01000007);
}

ModelRenderer::~ModelRenderer()
{}

void ModelRenderer::render()
{
}
