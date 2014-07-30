#ifndef BZR_GRAPHICS_RENDERER_H
#define BZR_GRAPHICS_RENDERER_H

#include "Noncopyable.h"
#ifdef OCULUSVR
#include <OVR_CAPI.h>
#endif

class SkyRenderer;
class LandblockRenderer;

class Renderer : Noncopyable
{
public:
    Renderer();
    ~Renderer();

    void render(double interp);

private:
    int _width;
    int _height;
    double _fieldOfView;

    bool _videoInit;
    SDL_Window* _window;
    SDL_GLContext _context;

#ifdef OCULUSVR
    ovrHmd _hmd;
    ovrSizei _renderTexSize;
    GLuint _renderTex;
#endif

    unique_ptr<SkyRenderer> _skyRenderer;
    unique_ptr<LandblockRenderer> _landblockRenderer;
};

#endif
