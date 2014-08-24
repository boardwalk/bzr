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
#include "graphics/TextureRenderData.h"
#include "graphics/Renderer.h"
#include "Core.h"
#include "Texture.h"

TextureRenderData::TextureRenderData(const Texture& texture)
{
    auto& image = texture.image();

    glGenTextures(1, &_handle);
    glBindTexture(GL_TEXTURE_2D, _handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Core::get().renderer().textureMinFilter());
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, Core::get().renderer().textureMaxAnisotropy());

    switch(image.format())
    {
        case ImageFormat::BGRA32:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, image.data());
            break;
        case ImageFormat::BGR24:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, image.width(), image.height(), 0, GL_BGR, GL_UNSIGNED_BYTE, image.data());
            break;
        case ImageFormat::DXT1:
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, image.width(), image.height(), 0, (GLsizei)image.size(), image.data());
            break;
        case ImageFormat::DXT3:
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, image.width(), image.height(), 0, (GLsizei)image.size(), image.data());
            break;
        case ImageFormat::DXT5:
            glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, image.width(), image.height(), 0, (GLsizei)image.size(), image.data());
            break;
        default:
            throw runtime_error("Unsupported image format");
    }

    glGenerateMipmap(GL_TEXTURE_2D);
}

TextureRenderData::~TextureRenderData()
{
    glDeleteTextures(1, &_handle);
}

void TextureRenderData::bind()
{
    glBindTexture(GL_TEXTURE_2D, _handle);
}
