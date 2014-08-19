#include "graphics/TextureAtlas.h"
#include "Core.h"
#include "ResourceCache.h"
#include "Texture.h"
#include "TextureLookup5.h"
#include "TextureLookup8.h"

TextureAtlas::TextureAtlas() : _nextIndex(0), _buildIndex(0)
{
    glGenTextures(1, &_atlas);
    glGenTextures(1, &_atlasToc);
}

TextureAtlas::~TextureAtlas()
{
    glDeleteTextures(1, &_atlas);
    glDeleteTextures(1, &_atlasToc);
}

int TextureAtlas::get(uint32_t resourceId)
{
    auto it = _textures.find(resourceId);

    if(it == _textures.end())
    {
        pair<uint32_t, TextureInfo> value;
        value.first = resourceId;
        value.second.resource = Core::get().resourceCache().get(resourceId);
        value.second.index = _nextIndex++;

        it = _textures.insert(value).first;
    }

    return it->second.index;
}

void TextureAtlas::bind()
{
    if(_buildIndex != _nextIndex)
    {
        generate();
        _buildIndex = _nextIndex;
    }
    else
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _atlas);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, _atlasToc);
    }
}

void TextureAtlas::generate()
{
    // TODO Look at using bin packing algorithm to pack textures in tighter
    GLsizei width = 0;
    GLsizei height = 0;

    for(auto& pair : _textures)
    {
        auto& image = pair.second.resource->cast<TextureLookup8>().textureLookup5().texture().image();

        width += image.width();
        height = max(height, image.height());
    }

    printf("Uploading texture atlas with dimensions %d x %d, %lu textures\n", width, height, _textures.size());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _atlas);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    vector<GLfloat> atlasTocData(_textures.size() * 4);

    GLint x = 0;

    for(auto& pair : _textures)
    {
        auto& image = pair.second.resource->cast<TextureLookup8>().textureLookup5().texture().image();

        if(image.format() == ImageFormat::BGRA32)
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, image.width(), image.height(), GL_BGRA, GL_UNSIGNED_BYTE, image.data());
        }
        else if(image.format() == ImageFormat::BGR24)
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, image.width(), image.height(), GL_BGR, GL_UNSIGNED_BYTE, image.data());
        }
        else
        {
            throw runtime_error("Unsupported image format for texture atlas");
        }

        // bottom left
        auto tocOffset = pair.second.index * 4;
        atlasTocData[tocOffset + 0] = GLfloat(x) / GLfloat(width);
        atlasTocData[tocOffset + 1] = 0.0f;
        // top right
        atlasTocData[tocOffset + 2] = GLfloat(image.width()) / GLfloat(width);
        atlasTocData[tocOffset + 3] = GLfloat(image.height()) / GLfloat(height);

        x += image.width();
    }

    // TODO Generate our own mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    printf("Uploading texture atlas TOC\n");

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, _atlasToc);
    // we need to do this even if we're using texelFetch and no sampling is done
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, (GLsizei)(atlasTocData.size() / 4), 0, GL_RGBA, GL_FLOAT, atlasTocData.data());

    printf("All done with texture atlas\n");
}
