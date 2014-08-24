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
    glGenTextures(1, &_handle);
    glBindTexture(GL_TEXTURE_2D, _handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Core::get().renderer().textureMinFilter());
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, Core::get().renderer().textureMaxAnisotropy());

    if(texture.image().format() == ImageFormat::BGRA32)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture.image().width(), texture.image().height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, texture.image().data());
    }
    else if(texture.image().format() == ImageFormat::BGR24)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture.image().width(), texture.image().height(), 0, GL_BGR, GL_UNSIGNED_BYTE, texture.image().data());
    }
    else
    {
        printf("skipped! %08x\n", texture.image().format());
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
