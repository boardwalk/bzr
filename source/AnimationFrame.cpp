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
#include "AnimationFrame.h"
#include "BinReader.h"

AnimationFrame::AnimationFrame(BinReader& reader, uint32_t numModels)
{
    orientations.resize(numModels);

    for(Orientation& orientation : orientations)
    {
        orientation.position.x = reader.readFloat();
        orientation.position.y = reader.readFloat();
        orientation.position.z = reader.readFloat();

        orientation.rotation.w = reader.readFloat();
        orientation.rotation.x = reader.readFloat();
        orientation.rotation.y = reader.readFloat();
        orientation.rotation.z = reader.readFloat();
    }

    uint32_t numExtra = reader.readInt();

    for(uint32_t ei = 0; ei < numExtra; ei++)
    {
        uint32_t extraType = reader.readInt();
        uint32_t extraSize = 0;

        switch(extraType)
        {
            case 0x01: extraSize = 2;  break; // 0x00,soundref
            case 0x02: extraSize = 2;  break; // 0x00,0x0C
            case 0x03: extraSize = 8;  break; // 0x00,0x14,6floats
            case 0x05: extraSize = 2;  break; // 0x00,0xBB401
            case 0x06: extraSize = 2;  break; // 0x01,0x01
            case 0x07: extraSize = 5;  break; // 0x00,0x0A,1.0,1.0,0x00
            case 0x0D: extraSize = 11; break; // lotsa stuff (3 floats in there somewhere)
            case 0x0F: extraSize = 2;  break; // 0x00,0x01
            case 0x11: extraSize = 1;  break; // 0x00
            case 0x13: extraSize = 3;  break; // 0x00,someREF,0x00
            case 0x14: extraSize = 4;  break; // 0x00,0x00,0x00,0x00
            case 0x15: extraSize = 5;  break; // 0x00,soundref,3floats
            case 0x16: extraSize = 4;  break; // 0x00,0x00,2floats
            default:
                throw runtime_error("Unknown extraType in animation frame");
        }

        reader.readRaw(extraSize * sizeof(uint32_t));
    }
}

AnimationFrame::AnimationFrame(AnimationFrame&& other)
{
    orientations = move(other.orientations);
}

AnimationFrame& AnimationFrame::operator=(AnimationFrame&& other)
{
    orientations = move(other.orientations);
    return *this;
}
