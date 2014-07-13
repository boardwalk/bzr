#ifndef BZR_GRAPHICS_TEXTURE_H
#define BZR_GRAPHICS_TEXTURE_H

class Image;

class Texture
{
public:
    Texture();
    Texture(Texture&& other);
    ~Texture();
    Texture& operator=(Texture&& other);

    void create(const Image& image);
    void destroy();

    void bind(int i);

private:
    GLuint _handle;
};

#endif
