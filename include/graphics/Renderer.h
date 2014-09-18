/*
 * Bael'Zharon's Respite
 * Copyright (C) 2014 Daniel Skorupski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef BZR_GRAPHICS_RENDERER_H
#define BZR_GRAPHICS_RENDERER_H

#include "Noncopyable.h"
#ifdef OCULUSVR
#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>
#endif

class LandRenderer;
class ModelRenderer;
class SkyRenderer;
class StructureRenderer;

class Renderer : Noncopyable
{
public:
    Renderer();
    ~Renderer();

    void init();
    void render(fp_t interp);

    GLenum textureMinFilter() const;
    GLfloat textureMaxAnisotropy() const;
    bool renderHitGeometry() const;

private:
    void createWindow();
#ifdef OCULUSVR
    void initOVR();
    void cleanupOVR();
    void renderOVR(fp_t interp);
#endif

    fp_t fieldOfView_;
    GLenum textureMinFilter_;
    GLfloat textureMaxAnisotropy_;
    bool renderHitGeometry_;

    bool videoInit_;
    SDL_Window* window_;
    SDL_GLContext context_;

#ifdef OCULUSVR
    ovrHmd hmd_;
    ovrSizei renderTexSize_;
    ovrRecti eyeViewport_[ovrEye_Count];
    ovrGLTexture eyeTexture_[ovrEye_Count];
    ovrEyeRenderDesc eyeRenderDesc_[ovrEye_Count];
    GLuint renderTex_;
    GLuint depthTex_;
    GLuint framebuffer_;
#endif

    unique_ptr<SkyRenderer> skyRenderer_;
    unique_ptr<LandRenderer> landRenderer_;
    unique_ptr<StructureRenderer> structureRenderer_;
    unique_ptr<ModelRenderer> modelRenderer_;
};

#endif
