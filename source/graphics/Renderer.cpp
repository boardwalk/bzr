#include "graphics/Renderer.h"
#include "graphics/LandblockRenderer.h"
#include "graphics/util.h"
#include "math/Mat3.h"
#include "Camera.h"
#include "Core.h"
#include "Landblock.h"
#include "util.h"

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
    _width = 1280;
    _height = 960;
    _fieldOfView = 90.0;

    _window = SDL_CreateWindow("Bael'Zharon's Respite",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _width, _height, SDL_WINDOW_OPENGL);

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
        string err("Unable to initialize GLEW: ");
        err.append((const char*)glewGetErrorString(glewErr));
        throw runtime_error(err);
	}
#endif

    SDL_GL_SetSwapInterval(1); // vsync
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xffff);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // the default is 4
}

Renderer::~Renderer()
{
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
    // projection * view * model * vertex
    Mat4 projectionMat;
    projectionMat.makePerspective(_fieldOfView, double(_width)/double(_height), 0.1, 1000.0);

    const Mat4& viewMat = Core::get().camera().viewMatrix();

    Mat4 modelMat;
    modelMat.makeIdentity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    auto& landblock = Core::get().landblock();
    auto& renderData = landblock.renderData();

    if(!renderData)
    {
        renderData.reset(new LandblockRenderer(landblock));
    }

    auto& landblockRenderer = (LandblockRenderer&)*renderData;
    landblockRenderer.render(projectionMat, viewMat * modelMat);

    SDL_GL_SwapWindow(_window);
}
