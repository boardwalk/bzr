#ifndef BZR_BASIC_H
#define BZR_BASIC_H

#include <SDL.h>

#ifdef _MSC_VER
#include <GL/glew.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#endif

#include <cassert>
#include <memory>

using namespace std;

template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args)
{
    return unique_ptr<T>(new T(forward<Args>(args)...));
}

#endif
