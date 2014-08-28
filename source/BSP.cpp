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
#include "BSP.h"
#include "BinReader.h"

static void skipBSPLeaf(BinReader& reader, int treeType)
{
    reader.read<uint32_t>(); // leaf index

    if(treeType != 1)
    {
        return;
    }

    reader.read<uint32_t>(); // if 1, sphere parameters are valid and there are indices

    reader.read<float>(); // sx
    reader.read<float>(); // sy
    reader.read<float>(); // sz
    reader.read<float>(); // sr

    auto indexCount = reader.read<uint32_t>();

    for(auto i = 0u; i < indexCount; i++)
    {
        reader.read<uint16_t>();
    }
}

static void skipBSPPortal(BinReader& reader, int treeType)
{
    reader.read<float>(); // px
    reader.read<float>(); // py
    reader.read<float>(); // pz
    reader.read<float>(); // pd

    skipBSP(reader, treeType);
    skipBSP(reader, treeType);

    if(treeType != 0)
    {
        return;
    }

    reader.read<float>(); // sx
    reader.read<float>(); // sy
    reader.read<float>(); // sz
    reader.read<float>(); // sr

    auto triCount = reader.read<uint32_t>();
    auto polyCount = reader.read<uint32_t>();

    for(auto i = 0u; i < triCount; i++)
    {
        reader.read<uint16_t>(); // wTriIndex
    }

    for(auto i = 0u; i < polyCount; i++)
    {
        reader.read<uint16_t>(); // wIndex
        reader.read<uint16_t>(); // wWhat
    }
}

static void skipBSPNode(BinReader& reader, int treeType, uint32_t nodeType)
{
    reader.read<float>(); // px
    reader.read<float>(); // py
    reader.read<float>(); // pz
    reader.read<float>(); // pd

    if(nodeType == 0x42506e6e || nodeType == 0x4250496e) // BPnn, BPIn
    {
        skipBSP(reader, treeType);
    }
    else if(nodeType == 0x4270494e || nodeType == 0x42706e4e) // BpIN, BpnN
    {
        skipBSP(reader, treeType);
    }
    else if(nodeType == 0x4250494e || nodeType == 0x42506e4e) // BPIN, BPnN
    {
        skipBSP(reader, treeType);
        skipBSP(reader, treeType);
    }

    if(treeType == 0 || treeType == 1)
    {
        reader.read<float>(); // sx
        reader.read<float>(); // sy
        reader.read<float>(); // sz
        reader.read<float>(); // sr
    }

    if(treeType != 0)
    {
        return;
    }

    auto indexCount = reader.read<uint32_t>();

    for(auto i = 0u; i < indexCount; i++)
    {
        reader.read<uint16_t>();
    }
}

void skipBSP(BinReader& reader, int treeType)
{
    auto nodeType = reader.read<uint32_t>();

    if(nodeType == 0x4c454146) // LEAF
    {
        skipBSPLeaf(reader, treeType);
    }
    else if(nodeType == 0x504f5254) // PORT
    {
        skipBSPPortal(reader, treeType);
    }
    else
    {
        skipBSPNode(reader, treeType, nodeType);
    }
}
