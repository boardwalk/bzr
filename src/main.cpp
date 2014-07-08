#include "DatFile.hpp"
#include <SDL.h>
#include <SDL_opengl.h>
#include <iostream>
#include <cstdlib>

#include "VertexShader.h"
#include "FragmentShader.h"

static const GLfloat PI = 3.14159265359;

class GLError : public runtime_error
{
public:
    GLError(const string& msg) : runtime_error(msg) { }
};

#ifndef NDEBUG
#define CHECK_GL(stmt) stmt
#else
#define CHECK_GL(stmt) { stmt; CheckGL(); }

static void CheckGL()
{
    GLenum e = glGetError();

    if(e != GL_NO_ERROR)
    {
        char buf[16];
        sprintf(buf, "%08x", e);

        throw GLError(buf);
    }
}
#endif

class Renderer
{
public:
    void init();
    void cleanup();
    void render();

private:
    void initProgram();
    void cleanupProgram();

    GLuint _program;
};

void identityMatrix(GLfloat mat[16])
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
void perspectiveMatrix(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar, GLfloat m[16])
{
   memset(m, 0, sizeof(GLfloat) * 16);

   auto f = 1.0 / tan(fovy * PI / 360.0f);
   m[0] = f / aspect;
   m[5] = f;
   m[10] = (zFar + zNear) / (zFar - zNear); // negated!
   m[11] = 1.0f; // negated!
   m[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
}

void Renderer::init()
{
    initProgram();

    GLfloat projectionMat[16];
    perspectiveMatrix(90.0f, 800.0/600.0f, 0.1f, 1000.0f, projectionMat);

    GLfloat viewMat[16];
    identityMatrix(viewMat);

    GLfloat modelMat[16];
    identityMatrix(modelMat);

    GLint projectionLoc = glGetUniformLocation(_program, "projection");
    CHECK_GL(glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projectionMat));

    GLint viewLoc = glGetUniformLocation(_program, "view");
    CHECK_GL(glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMat));

    GLint modelLoc = glGetUniformLocation(_program, "model");
    CHECK_GL(glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelMat));
}

void Renderer::cleanup()
{
    cleanupProgram();
}

void Renderer::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static GLuint initShader(GLenum type, const GLchar* source, const GLint length)
{
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
        throw GLError(logStr);
    }

    return shader;
}

void Renderer::initProgram()
{
    GLuint vertexShader = initShader(GL_VERTEX_SHADER, VertexShader, sizeof(VertexShader)/sizeof(VertexShader[0]));
    GLuint fragmentShader = initShader(GL_FRAGMENT_SHADER, FragmentShader, sizeof(FragmentShader)/sizeof(FragmentShader[0]));

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
        throw GLError(logStr);
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

int main()
{
    try
    {
        SDL_Init(SDL_INIT_VIDEO);

        SDL_Window* window = SDL_CreateWindow("Bael'Zharon's Respite",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GLContext context = SDL_GL_CreateContext(window);

        Renderer renderer;
        renderer.init();

        bool running = true;

        while(running)
        {
            SDL_Event event;

            while(SDL_PollEvent(&event) != 0)
            {
                switch(event.type)
                {
                    case SDL_QUIT:
                        running = false;
                        break;
                }
            }

            renderer.render();
            SDL_GL_SwapWindow(window);
        }

        renderer.cleanup();

        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    catch(const runtime_error& e)
    {
        cerr << "An error ocurred: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
