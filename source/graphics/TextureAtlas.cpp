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
#include "graphics/TextureAtlas.h"
#include "graphics/Renderer.h"
#include "Core.h"
#include "ResourceCache.h"
#include "Texture.h"
#include "TextureLookup5.h"
#include "TextureLookup8.h"
#include <algorithm>

struct SortBySize
{
    bool operator()(const TextureAtlas::TextureInfo* a, const TextureAtlas::TextureInfo* b) const
    {
        auto& imageA = a->resource->cast<TextureLookup8>().textureLookup5().texture().image();
        auto& imageB = b->resource->cast<TextureLookup8>().textureLookup5().texture().image();
        return imageA.width() * imageA.height() > imageB.width() * imageB.height(); // sort descending
    }
};

uint32_t nextPowerOfTwo(uint32_t n)
{
    n--;
    n |= n >> 1;   // Divide by 2^k for consecutive doublings of k up to 32,
    n |= n >> 2;   // and then or the results.
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

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
    // Collect used and remove unused textures
    vector<TextureInfo*> sortedTextureInfos;
    sortedTextureInfos.reserve(_textures.size());

    for(auto it = _textures.begin(); it != _textures.end(); /**/)
    {
        if(it->second.resource.use_count() > 1)
        {
            sortedTextureInfos.push_back(&it->second);
            ++it;
        }
        else
        {
            it = _textures.erase(it);
        }
    }

    // Sort textures by height
    // We waste significantly less space when we do this
    sort(sortedTextureInfos.begin(), sortedTextureInfos.end(), SortBySize());

    // Find the required size of the base mipmap level
    GLint x = 0;
    GLint y = 0;
    GLsizei rowHeight = 0;
    GLsizei width = 0;

    GLint maxSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);

    for(auto textureInfo : sortedTextureInfos)
    {
        auto& image = textureInfo->resource->cast<TextureLookup8>().textureLookup5().texture().image();

        // start new row when we exceed the maximum width
        if(x + image.width() > maxSize)
        {
            x = 0;
            y += rowHeight;
            rowHeight = 0;
        }

        // bail when we exceed the maximum height
        if(y + image.height() > maxSize)
        {
            throw runtime_error("Maximum texture size exceeded");
        }

        // image is placed at x, y

        x += image.width();
        rowHeight = max(rowHeight, image.height());
        width = max(width, x);
    }

    GLsizei height = y + rowHeight;

    // Initialize mipmap levels
    // Our nominal largest texture is 512x512, so we need 10 levels including the base
    vector<Image> mipmaps(10);

    for(auto level = 0u; level < mipmaps.size(); level++)
    {
        mipmaps[level].init(ImageFormat::BGRA32, width >> level, height >> level, nullptr);
    }

    // Generate mipmaps
    x = 0;
    y = 0;
    rowHeight = 0;

    for(auto textureInfo : sortedTextureInfos)
    {
        auto& image = textureInfo->resource->cast<TextureLookup8>().textureLookup5().texture().image();

        // start new row when we exceed the maximum width
        if(x + image.width() > maxSize)
        {
            x = 0;
            y += rowHeight;
            rowHeight = 0;
        }

        if(image.format() == ImageFormat::BGRA32)
        {
            mipmaps[0].blit(image, x, y);

            if(image.width() > 1 || image.height() > 1)
            {
                auto scaledImage = image.scaleHalf();

                for(auto level = 1u; level < mipmaps.size(); level++)
                {
                    mipmaps[level].blit(scaledImage, x >> level, y >> level);

                    if(scaledImage.width() == 1 || scaledImage.height() == 1)
                    {
                        break;
                    }

                    scaledImage = scaledImage.scaleHalf();
                }
            }
        }
        else if(image.format() == ImageFormat::BGR24)
        {
            printf("skipping... %08x\n", image.format());
        }
        else
        {
            throw runtime_error("Unsupported image format for texture atlas");
        }

        x += image.width();
        rowHeight = max(rowHeight, image.height());
    }

    // Upload mipmaps
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _atlas);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, GLint(mipmaps.size() - 1));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Core::get().renderer().textureMinFilter());
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, Core::get().renderer().textureMaxAnisotropy());

    for(auto level = 0u; level < mipmaps.size(); level++)
    {
        glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA8, mipmaps[level].width(), mipmaps[level].height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, mipmaps[level].data());
    }

    // Generate toc data
    vector<GLfloat> atlasTocData(_nextIndex * 4);

    x = 0;
    y = 0;
    rowHeight = 0;
    
    for(auto textureInfo : sortedTextureInfos)
    {
        auto& image = textureInfo->resource->cast<TextureLookup8>().textureLookup5().texture().image();

        // start new row when we exceed the maximum width
        if(x + image.width() > maxSize)
        {
            x = 0;
            y += rowHeight;
            rowHeight = 0;
        }

        auto tocOffset = textureInfo->index * 4;
        atlasTocData[tocOffset + 0] = (GLfloat(x) + 0.5f) / GLfloat(width);
        atlasTocData[tocOffset + 1] = (GLfloat(y) + 0.5f) / GLfloat(height);
        atlasTocData[tocOffset + 2] = GLfloat(image.width() - 1) / GLfloat(width);
        atlasTocData[tocOffset + 3] = GLfloat(image.height() - 1) / GLfloat(height);

        x += image.width();
        rowHeight = max(rowHeight, image.height());
    }

    // Upload toc data
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, _atlasToc);
    // we need to do this even if we're using texelFetch and no sampling is done
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, (GLsizei)(atlasTocData.size() / 4), 0, GL_RGBA, GL_FLOAT, atlasTocData.data());
}
