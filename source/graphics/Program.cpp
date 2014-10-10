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

Program::Program() : handle_(0)
{}

Program::~Program()
{
    destroy();
}

void Program::create()
{
    destroy();
    handle_ = glCreateProgram();
}

void Program::attach(GLenum type, const GLchar* source)
{
    GLint length = static_cast<GLint>(strlen(source));

    GLuint shader = glCreateShader(type);
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

    glAttachShader(handle_, shader);
    glDeleteShader(shader);
}

void Program::link()
{
    glLinkProgram(handle_);

    GLint success = GL_FALSE;
    glGetProgramiv(handle_, GL_LINK_STATUS, &success);

    if(!success)
    {
        GLint logLength;
        glGetProgramiv(handle_, GL_INFO_LOG_LENGTH, &logLength);

        vector<GLchar> log(logLength);
        glGetProgramInfoLog(handle_, logLength, &logLength, log.data());

        string err("Failed to link program: ");
        err.append(log.data(), logLength);
        throw runtime_error(err);
    }
}

void Program::use()
{
    assert(handle_ != 0);
    glUseProgram(handle_);
}

GLint Program::getUniform(const GLchar* name)
{
    assert(handle_ != 0);

    auto it = uniforms_.find(name);

    if(it == uniforms_.end())
    {
        it = uniforms_.insert({name, glGetUniformLocation(handle_, name)}).first;
    }

    if(it->second < 0)
    {
        //string err("Uniform does not exist: ");
        //err.append(name);
        //throw runtime_error(err);
        //puts(err.c_str());
    }

    return it->second;
}

void Program::destroy()
{
    if(handle_ != 0)
    {
        glDeleteProgram(handle_);
        handle_ = 0;
    }
}
