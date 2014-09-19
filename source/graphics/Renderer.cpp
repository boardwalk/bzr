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
#include "graphics/LandRenderer.h"
#include "graphics/ModelRenderer.h"
#include "graphics/SkyRenderer.h"
#include "graphics/StructureRenderer.h"
#include "Camera.h"
#include "Config.h"
#include "Core.h"
#include "util.h"
#include <glm/gtc/matrix_transform.hpp>
#ifdef OCULUSVR
#include <SDL_syswm.h>
#include <algorithm>
#endif

Renderer::Renderer() : videoInit_(false), window_(nullptr), context_(nullptr)
#ifdef OCULUSVR
    , hmd_(nullptr), renderTex_(0), depthTex_(0), framebuffer_(0)
#endif
{
    if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
        throwSDLError();
    }

    videoInit_ = true;

#ifdef __APPLE__
    // Apple's drivers don't support the compatibility profile on GL >v2.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    // 32 crashes mysteriously on my laptop
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    Config& config = Core::get().config();

    int multisamples = config.getInt("Renderer.multisamples", 16);

    if(multisamples != 0)
    {
        // Enable MSAA
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisamples);
    }

    createWindow();

    context_ = SDL_GL_CreateContext(window_);

    if(context_ == nullptr)
    {
        throwSDLError();
    }

#ifdef _MSC_VER
    GLenum glewErr = glewInit();

    if(glewErr != GLEW_OK)
    {
        string err("Unable to initialize GLEW: ");
        err.append((const char*)glewGetErrorString(glewErr));
        throw runtime_error(err);
    }
#endif

    fieldOfView_ = config.getFloat("Renderer.fieldOfView", 90.0);

    string textureFiltering = config.getString("Renderer.textureFiltering", "trilinear");

    if(textureFiltering == "bilinear")
    {
        textureMinFilter_ = GL_LINEAR_MIPMAP_NEAREST;
    }
    else if(textureFiltering == "trilinear")
    {
        textureMinFilter_ = GL_LINEAR_MIPMAP_LINEAR;
    }
    else
    {
        throw runtime_error("Bad value for Renderer.textureFiltering");
    }

    textureMaxAnisotropy_ = static_cast<GLfloat>(config.getFloat("Renderer.anisotropyLevel", 0.0));

    if(textureMaxAnisotropy_ != 0.0f)
    {
#ifdef _MSC_VER
        if(!GLEW_EXT_texture_filter_anisotropic)
        {
            throw runtime_error("Anisotropic filtering not supported");
        }
#endif

        GLfloat driverMaxAnisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &driverMaxAnisotropy);

        if(textureMaxAnisotropy_ < 0.0f || textureMaxAnisotropy_ > driverMaxAnisotropy)
        {
            throw runtime_error("Bad value for Renderer.maxAnisotropyLevel");
        }
    }

    renderHitGeometry_ = config.getBool("Renderer.renderHitGeometry", false);
}

Renderer::~Renderer()
{
    modelRenderer_.reset();
    landRenderer_.reset();
    structureRenderer_.reset();
    skyRenderer_.reset();

#ifdef OCULUSVR
    cleanupOVR();
#endif

    if(context_ != nullptr)
    {
        SDL_GL_DeleteContext(context_);
    }

    if(window_ != nullptr)
    {
        SDL_DestroyWindow(window_);
    }

    if(videoInit_)
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

    skyRenderer_.reset(new SkyRenderer());
    landRenderer_.reset(new LandRenderer());
    structureRenderer_.reset(new StructureRenderer());
    modelRenderer_.reset(new ModelRenderer());

    landRenderer_->setLightPosition(skyRenderer_->sunVector() * fp_t(1000.0));
}

void Renderer::render(fp_t interp)
{
    (void)interp;

#ifdef OCULUSVR
    if(hmd_ != nullptr)
    {
        return renderOVR(interp);
    }
#endif

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window_, &windowWidth, &windowHeight);

    // projection * view * model * vertex
    glm::mat4 projectionMat = glm::perspective(fieldOfView_ / fp_t(180.0) * pi(), static_cast<fp_t>(windowWidth) / static_cast<fp_t>(windowHeight), fp_t(0.1), fp_t(1000.0));

    const glm::mat4& viewMat = Core::get().camera().viewMatrix();

    glClear(GL_DEPTH_BUFFER_BIT);

    skyRenderer_->render();
    landRenderer_->render(projectionMat, viewMat);
    structureRenderer_->render(projectionMat, viewMat);
    modelRenderer_->render(projectionMat, viewMat);

    SDL_GL_SwapWindow(window_);
}

GLenum Renderer::textureMinFilter() const
{
    return textureMinFilter_;
}

GLfloat Renderer::textureMaxAnisotropy() const
{
    return textureMaxAnisotropy_;
}

bool Renderer::renderHitGeometry() const
{
    return renderHitGeometry_;
}

void Renderer::createWindow()
{
    Config& config = Core::get().config();
    int displayNum = config.getInt("Renderer.displayNum", 0);
    string windowMode = config.getString("Renderer.windowMode", "windowed");
    int width = config.getInt("Renderer.width", 1024);
    int height = config.getInt("Renderer.height", 768);

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

    window_ = SDL_CreateWindow("Bael'Zharon's Respite",
        windowBounds.x, windowBounds.y, windowBounds.w, windowBounds.h, windowFlags);

    if(window_ == nullptr)
    {
        throwSDLError();
    }
}

#ifdef OCULUSVR
static glm::quat convertOvrQuatf(const ovrQuatf& quat)
{
    return glm::quat{quat.w, quat.x, quat.y, quat.z};
}

static glm::vec3 convertOvrVector3f(const ovrVector3f& vec)
{
    // ovr has +x right, +y up, and +z back
    // we have +x right, +y forward, +z up,
    return glm::vec3{vec.x, -vec.z, vec.y};
}

static glm::mat4 convertOvrMatrix4f(const ovrMatrix4f& mat)
{
    glm::mat4 result;

    for(int j = 0; j < 4; j++)
    {
        for(int i = 0; i < 4; i++)
        {
            result[i][j] = mat.M[i][j];
        }
    }

    return result;
}

void Renderer::initOVR()
{
    ovr_Initialize();

    Config& config = Core::get().config();

    if(!config.getBool("Renderer.OVR", false))
    {
        return;
    }

    hmd_ = ovrHmd_Create(0);

    if(!hmd_)
    {
        fprintf(stderr, "Failed to create OVR HMD, falling back to fake one\n");
        hmd_ = ovrHmd_CreateDebug(ovrHmd_DK2);
    }

    ovrSizei leftEyeTexSize = ovrHmd_GetFovTextureSize(hmd_, ovrEye_Left, hmd_->DefaultEyeFov[ovrEye_Left], 1.0f);
    ovrSizei rightEyeTexSize = ovrHmd_GetFovTextureSize(hmd_, ovrEye_Right, hmd_->DefaultEyeFov[ovrEye_Right], 1.0f);

    renderTexSize_.w = leftEyeTexSize.w + rightEyeTexSize.w;
    renderTexSize_.h = max(leftEyeTexSize.h, rightEyeTexSize.h);

    glGenTextures(1, &renderTex_);
    glBindTexture(GL_TEXTURE_2D, renderTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, renderTexSize_.w, renderTexSize_.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &renderTexSize_.w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &renderTexSize_.h);

    glGenTextures(1, &depthTex_);
    glBindTexture(GL_TEXTURE_2D, depthTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, renderTexSize_.w, renderTexSize_.h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

    eyeViewport_[ovrEye_Left].Pos.x = 0;
    eyeViewport_[ovrEye_Left].Pos.y = 0;
    eyeViewport_[ovrEye_Left].Size.w = renderTexSize_.w / 2;
    eyeViewport_[ovrEye_Left].Size.h = renderTexSize_.h;

    eyeViewport_[ovrEye_Right].Pos.x = renderTexSize_.w / 2;
    eyeViewport_[ovrEye_Right].Pos.y = 0;
    eyeViewport_[ovrEye_Right].Size.w = renderTexSize_.w / 2;
    eyeViewport_[ovrEye_Right].Size.h = renderTexSize_.h;

    eyeTexture_[ovrEye_Left].OGL.Header.API = ovrRenderAPI_OpenGL;
    eyeTexture_[ovrEye_Left].OGL.Header.TextureSize = renderTexSize_;
    eyeTexture_[ovrEye_Left].OGL.Header.RenderViewport = eyeViewport_[ovrEye_Left];
    eyeTexture_[ovrEye_Left].OGL.TexId = renderTex_;

    eyeTexture_[ovrEye_Right].OGL.Header.API = ovrRenderAPI_OpenGL;
    eyeTexture_[ovrEye_Right].OGL.Header.TextureSize = renderTexSize_;
    eyeTexture_[ovrEye_Right].OGL.Header.RenderViewport = eyeViewport_[ovrEye_Right];
    eyeTexture_[ovrEye_Right].OGL.TexId = renderTex_;

    ovrSizei targetSize;
    SDL_GetWindowSize(window_, &targetSize.w, &targetSize.h);

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);

    if(!SDL_GetWindowWMInfo(window_, &wmInfo))
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

    if(!ovrHmd_ConfigureRendering(hmd_, &cfg.Config, distortionCaps, hmd_->DefaultEyeFov, eyeRenderDesc_))
    {
        throw runtime_error("Failed to configure HMD rendering");
    }

#ifdef OVR_OS_WIN32
    if(!ovrHmd_AttachToWindow(hmd_, wmInfo.info.win.window, nullptr, nullptr))
    {
        throw runtime_error("Failed to attach HMD to window");
    }
#endif

    glGenFramebuffers(1, &framebuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex_, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex_, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw runtime_error("Framebuffer not complete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int trackingCaps = ovrTrackingCap_Orientation|ovrTrackingCap_Position;

    if(!ovrHmd_ConfigureTracking(hmd_, trackingCaps, 0))
    {
        throw runtime_error("Failed to configure HMD tracking");
    }

    // warning will disappear as soon as the timeout expires
    ovrHmd_DismissHSWDisplay(hmd_);
}

void Renderer::cleanupOVR()
{
    if(hmd_)
    {
        glDeleteFramebuffers(1, &framebuffer_);
        glDeleteTextures(1, &renderTex_);
        glDeleteTextures(1, &depthTex_);

        ovrHmd_Destroy(hmd_);
    }

    ovr_Shutdown();
}

void Renderer::renderOVR(fp_t interp)
{
    (void)interp;

    ovrHmd_BeginFrame(hmd_, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glClear(GL_DEPTH_BUFFER_BIT);

    ovrPosef eyePose[ovrEye_Count];

    for(int i = 0; i < ovrEye_Count; i++)
    {
        ovrEyeType eye = hmd_->EyeRenderOrder[i];

        glViewport(eyeViewport_[eye].Pos.x, eyeViewport_[eye].Pos.y,
                   eyeViewport_[eye].Size.w, eyeViewport_[eye].Size.h);

        glm::mat4 projectionMat = convertOvrMatrix4f(ovrMatrix4f_Projection(eyeRenderDesc_[eye].Fov, 0.1f, 1000.0f, /*rightHanded*/ true));

        eyePose[eye] = ovrHmd_GetEyePose(hmd_, eye);
        Core::get().camera().setHeadOrientation(glm::conjugate(convertOvrQuatf(eyePose[eye].Orientation)));
        Core::get().camera().setHeadPosition(convertOvrVector3f(eyePose[eye].Position));

        glm::mat4 viewMat = glm::translate(glm::mat4{}, convertOvrVector3f(eyeRenderDesc_[eye].ViewAdjust));
        viewMat = viewMat * Core::get().camera().viewMatrix();

        skyRenderer_->render();
        landRenderer_->render(projectionMat, viewMat);
        structureRenderer_->render(projectionMat, viewMat);
        modelRenderer_->render(projectionMat, viewMat);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ovrHmd_EndFrame(hmd_, eyePose, (ovrTexture*)eyeTexture_);
}
#endif
