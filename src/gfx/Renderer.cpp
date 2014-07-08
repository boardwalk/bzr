#include "gfx/Renderer.h"
#include "util.h"
#include <SDL.h>
#include <string>
#include <vector>

#include "gfx/shaders/VertexShader.glsl.h"
#include "gfx/shaders/FragmentShader.glsl.h"

static const GLfloat PI = 3.14159265359;

static void identityMatrix(GLfloat mat[16])
{
    memset(mat, 0, sizeof(GLfloat) * 16);

    mat[0] = 1.0f;
    mat[5] = 1.0f;
    mat[10] = 1.0f;
    mat[15] = 1.0f;
}

//
// on coordinates
// our coordinate system is:
// +x goes right
// +y goes up
// +z goes into the screen
// this is "left handed"
// gluPerspective traditionally transforms "right handed" (+z out of the screen)
// to the "left handed" used by normalized device coordinates by flipping the z-axis.
// we don't do this.
// http://www.songho.ca/opengl/gl_projectionmatrix.html
//
static void perspectiveMatrix(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar, GLfloat m[16])
{
   memset(m, 0, sizeof(GLfloat) * 16);

   auto f = 1.0f / tan(fovy * PI / 360.0f);
   m[0] = f / aspect;
   m[5] = f;
   m[10] = (zFar + zNear) / (zFar - zNear); // negated!
   m[11] = 1.0f; // negated!
   m[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
}

void checkGL()
{
    auto e = glGetError();

    if(e != GL_NO_ERROR)
    {
        char buf[32];
        sprintf(buf, "OpenGL error %08x", e);

        throw runtime_error(buf);
    }
}

Renderer::Renderer() :
    _videoInit(false), _window(nullptr), _context(nullptr)
{
    if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    {
        throwSDLError();
    }

    _videoInit = true;

    _window = SDL_CreateWindow("Bael'Zharon's Respite",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);

    if(_window == nullptr)
    {
        throwSDLError();
    }

    // prevents us from using legacy features
    // as well as allows access to higher versions of OpenGL on OS X
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    _context = SDL_GL_CreateContext(_window);

    if(_context == nullptr)
    {
        throwSDLError();
    }

    initProgram();

    GLfloat projectionMat[16];
    perspectiveMatrix(90.0f, 800.0/600.0f, 0.1f, 1000.0f, projectionMat);

    GLfloat viewMat[16];
    identityMatrix(viewMat);

    GLfloat modelMat[16];
    identityMatrix(modelMat);

    auto projectionLoc = glGetUniformLocation(_program, "projection");
    CHECK_GL(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projectionMat));

    auto viewLoc = glGetUniformLocation(_program, "view");
    CHECK_GL(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMat));

    auto modelLoc = glGetUniformLocation(_program, "model");
    CHECK_GL(glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelMat));
}

Renderer::~Renderer()
{
    cleanupProgram();

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

void Renderer::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapWindow(_window);
}

static GLuint initShader(GLenum type, const GLchar* source)
{
    GLint length = strlen(source);

    GLuint shader;
    CHECK_GL(shader = glCreateShader(type));
    CHECK_GL(glShaderSource(shader, 1, &source, &length));
    CHECK_GL(glCompileShader(shader));

    GLint success = GL_FALSE;
    CHECK_GL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));

    if(!success)
    {
        GLint logLength;
        CHECK_GL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength));

        vector<GLchar> log(logLength);
        CHECK_GL(glGetShaderInfoLog(shader, logLength, &logLength, log.data()));

        string logStr(log.data(), logLength);
        throw runtime_error(logStr);
    }

    return shader;
}

void Renderer::initProgram()
{
    auto vertexShader = initShader(GL_VERTEX_SHADER, VertexShader);
    auto fragmentShader = initShader(GL_FRAGMENT_SHADER, FragmentShader);

    CHECK_GL(_program = glCreateProgram());
    CHECK_GL(glAttachShader(_program, vertexShader));
    CHECK_GL(glAttachShader(_program, fragmentShader));
    CHECK_GL(glLinkProgram(_program));

    GLint success = GL_FALSE;
    CHECK_GL(glGetProgramiv(_program, GL_LINK_STATUS, &success));

    if(!success)
    {
        GLint logLength;
        CHECK_GL(glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &logLength));

        vector<GLchar> log(logLength);
        CHECK_GL(glGetProgramInfoLog(_program, logLength, &logLength, log.data()));

        string logStr(log.data(), logLength);
        throw runtime_error(logStr);
    }

    CHECK_GL(glUseProgram(_program));
    CHECK_GL(glDeleteShader(vertexShader));
    CHECK_GL(glDeleteShader(fragmentShader));
    CHECK_GL(glDeleteProgram(_program));
}

void Renderer::cleanupProgram()
{
    CHECK_GL(glUseProgram(0));
}

