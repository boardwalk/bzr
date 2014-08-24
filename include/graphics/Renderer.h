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

class LandblockRenderer;
class ModelRenderer;
class SkyRenderer;
class StructureRenderer;

class Renderer : Noncopyable
{
public:
    Renderer();
    ~Renderer();

    void init();
    void render(double interp);

    GLenum textureMinFilter() const;
    GLfloat textureMaxAnisotropy() const;

private:
    void createWindow();
#ifdef OCULUSVR
    void initOVR();
    void cleanupOVR();
    void renderOVR(double interp);
#endif

    double _fieldOfView;
    GLenum _textureMinFilter;
    GLfloat _textureMaxAnisotropy;

    bool _videoInit;
    SDL_Window* _window;
    SDL_GLContext _context;

#ifdef OCULUSVR
    ovrHmd _hmd;
    ovrSizei _renderTexSize;
    ovrRecti _eyeViewport[ovrEye_Count];
    ovrGLTexture _eyeTexture[ovrEye_Count];
    ovrEyeRenderDesc _eyeRenderDesc[ovrEye_Count];
    GLuint _renderTex;
    GLuint _depthTex;
    GLuint _framebuffer;
#endif

    unique_ptr<SkyRenderer> _skyRenderer;
    unique_ptr<LandblockRenderer> _landblockRenderer;
    unique_ptr<StructureRenderer> _structureRenderer;
    unique_ptr<ModelRenderer> _modelRenderer;
};

#endif
