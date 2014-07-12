#include "graphics/Renderer.h"
#include "graphics/LandblockRenderer.h"
#include "graphics/util.h"
#include "Camera.h"
#include "Core.h"
#include "Landblock.h"
#include "util.h"

#include "graphics/shaders/VertexShader.glsl.h"
#include "graphics/shaders/FragmentShader.glsl.h"

Renderer::Renderer() : _videoInit(false), _window(nullptr), _context(nullptr)
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

    // TODO configurable
    float fieldOfView = 90.0f;
    int width = 800;
    int height = 600;

    _window = SDL_CreateWindow("Bael'Zharon's Respite",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);

    if(_window == nullptr)
    {
        throwSDLError();
    }

    _context = SDL_GL_CreateContext(_window);

    if(_context == nullptr)
    {
        throwSDLError();
    }

#ifdef _MSC_VER
	auto glewErr = glewInit();

	if(glewErr != GLEW_OK)
	{
		throw runtime_error(string("Unable to initialize GLEW: ") + (const char*)glewGetErrorString(glewErr));
	}
#endif

    SDL_GL_SetSwapInterval(1); // vsync
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0, 0.5f, 1.0f);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xffff);

    _program.create(VertexShader, FragmentShader);
    _program.use();

    Mat4 projectionMat;
    projectionMat.makePerspective(fieldOfView, double(width)/double(height), 0.1, 1000.0);

    Mat4 modelMat;
    modelMat.makeIdentity();

    auto projectionLoc = _program.getUniform("projection");
    loadMat4ToUniform(projectionMat, projectionLoc);

    auto modelLoc = _program.getUniform("model");
    loadMat4ToUniform(modelMat, modelLoc);

    auto fragTexLocation = _program.getUniform("fragTex");
    glUniform1i(fragTexLocation, 0); // corresponds to GL_TEXTURE_0

    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    initFramebuffer(width, height);
}

Renderer::~Renderer()
{
    cleanupFramebuffer();
    
    // TODO delete VAO
    // unuse program
    //glBindBuffer(GL_ARRAY_BUFFER, 0);

    _program.destroy();
    //glDeleteBuffers(1, & _buffer);

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

void Renderer::render(double interp)
{
    auto viewLoc = _program.getUniform("view");
    loadMat4ToUniform(Core::get().camera().viewMatrix(), viewLoc);

    // xx
    //glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    auto& landblock = Core::get().landblock();
    auto& renderData = landblock.renderData();

    if(!renderData)
    {
        renderData.reset(new LandblockRenderer(landblock));
    }

    auto& landblockRenderer = (LandblockRenderer&)*renderData;
    landblockRenderer.render();

    // xx
    //glBindFramebuffer(GL_READ_FRAMEBUFFER, _framebuffer);
    //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    //glBlitFramebuffer(0, 0, 800, 600, 0, 0, 800, 600, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

    SDL_GL_SwapWindow(_window);
}

void Renderer::initFramebuffer(int width, int height)
{
    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

    glGenTextures(1, &_colorTexture);
    glBindTexture(GL_TEXTURE_2D, _colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _colorTexture, 0);

    glGenTextures(1, &_normalTexture);
    glBindTexture(GL_TEXTURE_2D, _normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _normalTexture, 0);

    glGenTextures(1, &_depthTexture);
    glBindTexture(GL_TEXTURE_2D, _depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw runtime_error("Could not setup framebuffer");
    }

    GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, buffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::cleanupFramebuffer()
{
    glDeleteFramebuffers(1, &_framebuffer);
    glDeleteTextures(1, &_colorTexture);
    glDeleteTextures(1, &_normalTexture);
    glDeleteTextures(1, &_depthTexture);
}