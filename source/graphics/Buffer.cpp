#include "graphics/Buffer.h"

Buffer::Buffer() : _handle(0)
{}

Buffer::Buffer(Buffer&& other)
{
    _handle = other._handle;
    other._handle = 0;
}

Buffer::~Buffer()
{
    destroy();
}

Buffer& Buffer::operator=(Buffer&& other)
{
    destroy();
    _handle = other._handle;
    other._handle = 0;
    return *this;
}

void Buffer::create()
{
    destroy();

    glGenBuffers(1, &_handle);
}

void Buffer::destroy()
{
    glDeleteBuffers(1, &_handle);
}

void Buffer::bind(GLenum target)
{
    glBindBuffer(target, _handle);
}
