#ifndef BZR_GRAPHICS_TEXTURE_H
#define BZR_GRAPHICS_TEXTURE_H

class Texture
{
public:
    enum Format
    {
        BGR24, BGRA32, RGB24, A8
    };

    Texture();
    Texture(Texture&& other);
    ~Texture();
    Texture& operator=(Texture&& other);

    void create(Format format, const void* data, int width, int height);
    void destroy();

    void bind(int i);

private:
    GLuint _handle;
};

#endif
