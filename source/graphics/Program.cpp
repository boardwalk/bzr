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
#include "graphics/Program.h"

Program::Program() : _handle(0)
{}

Program::~Program()
{
    destroy();
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
