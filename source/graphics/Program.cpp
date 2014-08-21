#include "graphics/Program.h"
#include <vector>

Program::Program() : _handle(0)
{}

Program::Program(Program&& other)
{
    _handle = other._handle;
    other._handle = 0;
}

Program::~Program()
{
    destroy();
}

Program& Program::operator=(Program&& other)
{
    destroy();
    _handle = other._handle;
    other._handle = 0;
    return *this;
}

void Program::create()
{
    destroy();
    _handle = glCreateProgram();
}

void Program::attach(GLenum type, const GLchar* source)
{
    GLint length = GLint(strlen(source));

    auto shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, &length);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        vector<GLchar> log(logLength);
        glGetShaderInfoLog(shader, logLength, &logLength, log.data());

        string err("Failed to compile shader: ");
        err.append(log.data(), logLength);
        throw runtime_error(err);
    }

    glAttachShader(_handle, shader);
    glDeleteShader(shader);
}

void Program::link()
{
    glLinkProgram(_handle);

    GLint success = GL_FALSE;
    glGetProgramiv(_handle, GL_LINK_STATUS, &success);

    if(!success)
    {
        GLint logLength;
        glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &logLength);

        vector<GLchar> log(logLength);
        glGetProgramInfoLog(_handle, logLength, &logLength, log.data());

        string err("Failed to link program: ");
        err.append(log.data(), logLength);
        throw runtime_error(err);
    }
}

void Program::use()
{
    assert(_handle != 0);
    glUseProgram(_handle);
}

GLint Program::getUniform(const GLchar* name)
{
    assert(_handle != 0);

    auto loc = glGetUniformLocation(_handle, name);

    if(loc < 0)
    {
        //string err("Uniform does not exist: ");
        //err.append(name);
        //throw runtime_error(err);
        //puts(err.c_str());
    }

    return loc;
}

void Program::destroy()
{
    if(_handle != 0)
    {
        glDeleteProgram(_handle);
        _handle = 0;
    }
}
