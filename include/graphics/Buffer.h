#ifndef BZR_GRAPHICS_BUFFER_H
#define BZR_GRAPHICS_BUFFER_H

class Buffer
{
public:
    Buffer();
    Buffer(Buffer&& other);
    ~Buffer();
    Buffer& operator=(Buffer&& other);

    void create();
    void destroy();

    void bind(GLenum target);

private:
    GLuint _handle;
};

#endif