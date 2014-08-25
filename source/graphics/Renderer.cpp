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
#include "graphics/Renderer.h"
#include "graphics/LandblockRenderer.h"
#include "graphics/ModelRenderer.h"
#include "graphics/SkyRenderer.h"
#include "graphics/StructureRenderer.h"
#include "Camera.h"
#include "Config.h"
#include "Core.h"
#include "util.h"
#ifdef OCULUSVR
#include <SDL_syswm.h>
#include <algorithm>
#endif

#define UNITS_PER_METER 1.0

Renderer::Renderer() : _videoInit(false), _window(nullptr), _context(nullptr)
#ifdef OCULUSVR
    , _hmd(nullptr), _renderTex(0), _depthTex(0), _framebuffer(0)
#endif
{
    if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
        throwSDLError();
    }

    _videoInit = true;

#ifdef __APPLE__
    // Apple's drivers don't support the compatibility profile on GL >v2.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

    auto& config = Core::get().config();

    auto multisamples = config.getInt("Renderer.multisamples", 16);

    if(multisamples != 0)
    {
        // Enable MSAA
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisamples);
    }

    createWindow();

    _context = SDL_GL_CreateContext(_window);

    if(_context == nullptr)
    {
        throwSDLError();
    }

#ifdef _MSC_VER
	auto glewErr = glewInit();

	if(glewErr != GLEW_OK)
	{
        string err("Unable to initialize GLEW: ");
        err.append((const char*)glewGetErrorString(glewErr));
        throw runtime_error(err);
	}
#endif

    _fieldOfView = config.getDouble("Renderer.fieldOfView", 90.0);

    auto textureFiltering = config.getString("Renderer.textureFiltering", "trilinear");

    if(textureFiltering == "bilinear")
    {
        _textureMinFilter = GL_LINEAR_MIPMAP_NEAREST;
    }
    else if(textureFiltering == "trilinear")
    {
        _textureMinFilter = GL_LINEAR_MIPMAP_LINEAR;
    }
    else
    {
        throw runtime_error("Bad value for Renderer.textureFiltering");
    }

    _textureMaxAnisotropy = (GLfloat)config.getDouble("Renderer.anisotropyLevel", 0.0);

    if(_textureMaxAnisotropy != 0.0f)
    {
#ifdef _MSC_VER
        if(!GLEW_EXT_texture_filter_anisotropic)
        {
            throw runtime_error("Anisotropic filtering not supported");
        }
#endif

        GLfloat driverMaxAnisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &driverMaxAnisotropy);

        if(_textureMaxAnisotropy < 0.0f || _textureMaxAnisotropy > driverMaxAnisotropy)
        {
            throw runtime_error("Bad value for Renderer.maxAnisotropyLevel");
        }
    }
}

Renderer::~Renderer()
{
    _modelRenderer.reset();
    _landblockRenderer.reset();
    _structureRenderer.reset();
    _skyRenderer.reset();

#ifdef OCULUSVR
    cleanupOVR();
#endif

    if(_context != nullptr)
    {
        SDL_GL_DeleteContext(_context);
    }

    if(_window != nullptr)
    {
        SDL_DestroyWindow(_window);
    }

    if(_videoInit)
    {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
}

void Renderer::init()
{
#ifdef OCULUSVR
    initOVR();
#endif

    SDL_GL_SetSwapInterval(1); // vsync
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // the default is 4
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _skyRenderer.reset(new SkyRenderer());
    _landblockRenderer.reset(new LandblockRenderer());
    _structureRenderer.reset(new StructureRenderer());
    _modelRenderer.reset(new ModelRenderer());

    _landblockRenderer->setLightPosition(_skyRenderer->sunVector() * 1000.0);
}

void Renderer::render(double interp)
{
    (void)interp;

#ifdef OCULUSVR
    if(_hmd != nullptr)
    {
        return renderOVR(interp);
    }
#endif

    int windowWidth, windowHeight;
    SDL_GetWindowSize(_window, &windowWidth, &windowHeight);

    // projection * view * model * vertex
    Mat4 projectionMat;
    projectionMat.makePerspective(_fieldOfView, double(windowWidth)/double(windowHeight), 0.1, 1000.0);

    const Mat4& viewMat = Core::get().camera().viewMatrix();

    glClear(GL_DEPTH_BUFFER_BIT);

    _skyRenderer->render();
    _landblockRenderer->render(projectionMat, viewMat);
    _structureRenderer->render(projectionMat, viewMat);
    _modelRenderer->render(projectionMat, viewMat);

    SDL_GL_SwapWindow(_window);
}

// TEMPORARY
void Renderer::setModel(ResourcePtr model)
{
    _modelRenderer->_theModel = model;
}

// TEMPORARY
void Renderer::setSubmodelNum(int submodelNum)
{
    _modelRenderer->_submodelNum = submodelNum;
}

GLenum Renderer::textureMinFilter() const
{
    return _textureMinFilter;
}

GLfloat Renderer::textureMaxAnisotropy() const
{
    return _textureMaxAnisotropy;
}

void Renderer::createWindow()
{
    auto& config = Core::get().config();
    auto displayNum = config.getInt("Renderer.displayNum", 0);
    auto windowMode = config.getString("Renderer.windowMode", "windowed");
    auto width = config.getInt("Renderer.width", 1024);
    auto height = config.getInt("Renderer.height", 768);

    if(displayNum < 0 || displayNum >= SDL_GetNumVideoDisplays())
    {
        throw runtime_error("Bad value for Renderer.displayNum");
    }

    SDL_Rect displayBounds;
    SDL_GetDisplayBounds(displayNum, &displayBounds);

    SDL_Rect windowBounds;
    Uint32 windowFlags = SDL_WINDOW_OPENGL;

    if(windowMode == "fullscreen")
    {
        windowBounds.x = displayBounds.x;
        windowBounds.y = displayBounds.y;
        windowBounds.w = width;
        windowBounds.h = height;
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }
    else if(windowMode == "fullscreenDesktop")
    {
        windowBounds = displayBounds;
        windowFlags |= SDL_WINDOW_BORDERLESS;
    }
    else if(windowMode == "windowed")
    {
        windowBounds.x = displayBounds.x + (displayBounds.w - width) / 2;
        windowBounds.y = displayBounds.y + (displayBounds.h - height) / 2;
        windowBounds.w = width;
        windowBounds.h = height;
    }
    else
    {
        throw runtime_error("Bad value for Renderer.windowMode");
    }

    _window = SDL_CreateWindow("Bael'Zharon's Respite",
        windowBounds.x, windowBounds.y, windowBounds.w, windowBounds.h, windowFlags);

    if(_window == nullptr)
    {
        throwSDLError();
    }
}

#ifdef OCULUSVR
static Quat convertOvrQuatf(const ovrQuatf& quat)
{
    return Quat(quat.w, quat.x, quat.y, quat.z);
}

static Vec3 convertOvrVector3f(const ovrVector3f& vec)
{
    // ovr has +x right, +y up, and +z back
    // we have +x right, +y forward, +z up,
    return Vec3(vec.x, -vec.z, vec.y) * UNITS_PER_METER;
}

static Mat4 convertOvrMatrix4f(const ovrMatrix4f& mat)
{
    Mat4 result;

    for(auto j = 0; j < 4; j++)
    {
        for(auto i = 0; i < 4; i++)
        {
            result.m[i + j * 4] = mat.M[i][j];
        }
    }

    return result;
}

void Renderer::initOVR()
{
    ovr_Initialize();

    auto& config = Core::get().config();

    if(!config.getBool("Renderer.OVR", false))
    {
        return;
    }

    _hmd = ovrHmd_Create(0);

    if(!_hmd)
    {
        fprintf(stderr, "Failed to create OVR HMD, falling back to fake one\n");
        _hmd = ovrHmd_CreateDebug(ovrHmd_DK2);
    }

    auto leftEyeTexSize = ovrHmd_GetFovTextureSize(_hmd, ovrEye_Left, _hmd->DefaultEyeFov[ovrEye_Left], 1.0f);
    auto rightEyeTexSize = ovrHmd_GetFovTextureSize(_hmd, ovrEye_Right, _hmd->DefaultEyeFov[ovrEye_Right], 1.0f);

    _renderTexSize.w = leftEyeTexSize.w + rightEyeTexSize.w;
    _renderTexSize.h = max(leftEyeTexSize.h, rightEyeTexSize.h);

    glGenTextures(1, &_renderTex);
    glBindTexture(GL_TEXTURE_2D, _renderTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _renderTexSize.w, _renderTexSize.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &_renderTexSize.w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &_renderTexSize.h);

    glGenTextures(1, &_depthTex);
    glBindTexture(GL_TEXTURE_2D, _depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, _renderTexSize.w, _renderTexSize.h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

    _eyeViewport[ovrEye_Left].Pos.x = 0;
    _eyeViewport[ovrEye_Left].Pos.y = 0;
    _eyeViewport[ovrEye_Left].Size.w = _renderTexSize.w / 2;
    _eyeViewport[ovrEye_Left].Size.h = _renderTexSize.h;

    _eyeViewport[ovrEye_Right].Pos.x = _renderTexSize.w / 2;
    _eyeViewport[ovrEye_Right].Pos.y = 0;
    _eyeViewport[ovrEye_Right].Size.w = _renderTexSize.w / 2;
    _eyeViewport[ovrEye_Right].Size.h = _renderTexSize.h;

    _eyeTexture[ovrEye_Left].OGL.Header.API = ovrRenderAPI_OpenGL;
    _eyeTexture[ovrEye_Left].OGL.Header.TextureSize = _renderTexSize;
    _eyeTexture[ovrEye_Left].OGL.Header.RenderViewport = _eyeViewport[ovrEye_Left];
    _eyeTexture[ovrEye_Left].OGL.TexId = _renderTex;

    _eyeTexture[ovrEye_Right].OGL.Header.API = ovrRenderAPI_OpenGL;
    _eyeTexture[ovrEye_Right].OGL.Header.TextureSize = _renderTexSize;
    _eyeTexture[ovrEye_Right].OGL.Header.RenderViewport = _eyeViewport[ovrEye_Right];
    _eyeTexture[ovrEye_Right].OGL.TexId = _renderTex;

    ovrSizei targetSize;
    SDL_GetWindowSize(_window, &targetSize.w, &targetSize.h);

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);

    if(!SDL_GetWindowWMInfo(_window, &wmInfo))
    {
        throw runtime_error("Failed to get window info");
    }

    ovrGLConfig cfg;
    cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
    cfg.OGL.Header.RTSize = targetSize;
    cfg.OGL.Header.Multisample = 1; // yes?
#if defined(OVR_OS_WIN32)
    cfg.OGL.Window = wmInfo.info.win.window;
    cfg.OGL.DC = GetDC(wmInfo.info.win.window);
#elif defined(OVR_OS_MAC)
    // Mac does not have any fields
#else
    #error Implement for this OS.
#endif

    unsigned int distortionCaps = ovrDistortionCap_Chromatic|ovrDistortionCap_TimeWarp|ovrDistortionCap_Overdrive;

    if(!ovrHmd_ConfigureRendering(_hmd, &cfg.Config, distortionCaps, _hmd->DefaultEyeFov, _eyeRenderDesc))
    {
        throw runtime_error("Failed to configure HMD rendering");
    }

#ifdef OVR_OS_WIN32
    if(!ovrHmd_AttachToWindow(_hmd, wmInfo.info.win.window, nullptr, nullptr))
    {
        throw runtime_error("Failed to attach HMD to window");
    }
#endif

    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _renderTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTex, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw runtime_error("Framebuffer not complete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int trackingCaps = ovrTrackingCap_Orientation|ovrTrackingCap_Position;

    if(!ovrHmd_ConfigureTracking(_hmd, trackingCaps, 0))
    {
        throw runtime_error("Failed to configure HMD tracking");
    }

    // warning will disappear as soon as the timeout expires
    ovrHmd_DismissHSWDisplay(_hmd);
}

void Renderer::cleanupOVR()
{
    if(_hmd)
    {
        glDeleteFramebuffers(1, &_framebuffer);
        glDeleteTextures(1, &_renderTex);
        glDeleteTextures(1, &_depthTex);

        ovrHmd_Destroy(_hmd);
    }

    ovr_Shutdown();
}

void Renderer::renderOVR(double interp)
{
    (void)interp;

    ovrHmd_BeginFrame(_hmd, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glClear(GL_DEPTH_BUFFER_BIT);

    ovrPosef eyePose[ovrEye_Count];

    for(auto i = 0; i < ovrEye_Count; i++)
    {
        auto eye = _hmd->EyeRenderOrder[i];

        glViewport(_eyeViewport[eye].Pos.x, _eyeViewport[eye].Pos.y,
                   _eyeViewport[eye].Size.w, _eyeViewport[eye].Size.h);

        auto projectionMat = convertOvrMatrix4f(ovrMatrix4f_Projection(_eyeRenderDesc[eye].Fov, 0.1f, 1000.0f, /*rightHanded*/ true));

        eyePose[eye] = ovrHmd_GetEyePose(_hmd, eye);
        Core::get().camera().setHeadOrientation(convertOvrQuatf(eyePose[eye].Orientation).conjugate());
        Core::get().camera().setHeadPosition(convertOvrVector3f(eyePose[eye].Position));

        Mat4 viewMat;
        viewMat.makeTranslation(convertOvrVector3f(_eyeRenderDesc[eye].ViewAdjust));
        viewMat = viewMat * Core::get().camera().viewMatrix();

        _skyRenderer->render();
        _landblockRenderer->render(projectionMat, viewMat);
        _structureRenderer->render(projectionMat, viewMat);
        _modelRenderer->render(projectionMat, viewMat);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ovrHmd_EndFrame(_hmd, eyePose, (ovrTexture*)_eyeTexture);    
}
#endif
