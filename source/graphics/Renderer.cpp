#include "graphics/Renderer.h"
#include "graphics/LandblockRenderer.h"
#include "graphics/SkyRenderer.h"
//#include "graphics/SkyModel.h"
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
    
    // Enable 16x MSAA
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

    // TODO configurable
    _width = 1024;
    _height = 768;
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

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // the default is 4

    _skyRenderer.reset(new SkyRenderer());
    _landblockRenderer.reset(new LandblockRenderer());

    /*SkyModel model;

    SkyModel::Params params;
    params.dt = 180.0;
    params.tm = 0.5;
    params.lng = 0.0;
    params.lat = 0.0;
    params.tu = 0.0;
    model.prepare(params);

    for(auto theta = -PI; theta < PI; theta += PI / 5.0)
    {
        for(auto phi = -PI; phi < PI; phi += PI / 5.0)
        {
            auto c = model.getColor(theta, phi);
            printf("%+3.2f %+3.2f: %.2f %.2f %.2f\n", theta, phi, c.x, c.y, c.z);
        }
    }*/
}

Renderer::~Renderer()
{
    _landblockRenderer.reset();
    _skyRenderer.reset();

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

    glClear(GL_DEPTH_BUFFER_BIT);

    _skyRenderer->render(projectionMat, viewMat);
    _landblockRenderer->render(projectionMat, viewMat);

    SDL_GL_SwapWindow(_window);
}
