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
#include "DatFile.h"
#include <algorithm>
 
static const size_t kHeaderOffset = 0x140;
static const uint32_t kHeaderMagicNumber = 0x5442; // 'BT\0\0'
static const int kMaxNodeCount = 62;

PACK(struct DatHeader
{
    uint32_t magicNumber;
    uint32_t blockSize;
    uint32_t fileSize;
    uint32_t fileVersion;
    uint32_t fileVersion2;
    uint32_t freeHead;
    uint32_t freeTail;
    uint32_t freeBlockCount;
    uint32_t rootPosition;
});

PACK(struct DatLeaf
{
    uint32_t flags;
    uint32_t id;
    uint32_t position;
    uint32_t size;
    uint32_t timestamp;
    uint32_t version;
});

PACK(struct DatNode
{
    uint32_t internalNodes[kMaxNodeCount];
    uint32_t nodeCount;
    DatLeaf leafNodes[kMaxNodeCount];
});

DatFile::DatFile(const string& path) :
    fs_(path.c_str(), ios_base::in|ios_base::binary)
{
    DatHeader header;

    fs_.seekg(kHeaderOffset);
    fs_.read(reinterpret_cast<char*>(&header), sizeof(header));

    if(!fs_.good())
    {
        throw runtime_error("Could not read dat file header");
    }

    if(header.magicNumber != kHeaderMagicNumber)
    {
        throw runtime_error("Dat file header has bad magic number");
    }

    blockSize_ = header.blockSize - sizeof(uint32_t); // exclude next block position
    rootPosition_ = header.rootPosition;
}

vector<uint8_t> DatFile::read(uint32_t id) const
{
    uint32_t position = rootPosition_;

    for(;;)
    {
        vector<uint8_t> nodeData = readBlocks(position, sizeof(DatNode));
        DatNode* node = reinterpret_cast<DatNode*>(nodeData.data());

        if(node->nodeCount >= kMaxNodeCount)
        {
            throw runtime_error("Node has bad node count");
        }

        uint32_t i = 0;

        for(; i < node->nodeCount; i++)
        {
            if(id <= node->leafNodes[i].id)
            {
                break;
            }
        }

        if(i < node->nodeCount && id == node->leafNodes[i].id)
        {
            return readBlocks(node->leafNodes[i].position, node->leafNodes[i].size);
        }

        if(node->internalNodes[0] == 0)
        {
            return vector<uint8_t>();
        }

        position = node->internalNodes[i];
    }
}

vector<uint32_t> DatFile::list() const
{
    vector<uint32_t> result;
    listDir(rootPosition_, result);
    return result;
}

vector<uint8_t> DatFile::readBlocks(uint32_t position, size_t size) const
{
    vector<uint8_t> result(size);

    size_t offset = 0;

    while(offset < size)
    {
        if(position == 0)
        {
            throw runtime_error("Not enough blocks for resource");
        }

        size_t readSize = min<size_t>(size - offset, blockSize_);

        fs_.seekg(position);
        fs_.read(reinterpret_cast<char*>(&position), sizeof(position));
        fs_.read(reinterpret_cast<char*>(result.data() + offset), readSize);

        if(!fs_.good())
        {
            throw runtime_error("Failed to read block");
        }

        offset += readSize;
    }

    return result;
}

void DatFile::listDir(uint32_t position, vector<uint32_t>& result) const
{
    vector<uint8_t> nodeData = readBlocks(position, sizeof(DatNode));
    DatNode* node = reinterpret_cast<DatNode*>(nodeData.data());

    if(node->nodeCount >= kMaxNodeCount)
    {
        throw runtime_error("Node has bad node count");
    }

    for(uint32_t i = 0; i < node->nodeCount; i++)
    {
        if(node->internalNodes[0] != 0)
        {
            listDir(node->internalNodes[i], result);
        }

        result.push_back(node->leafNodes[i].id);
    }

    if(node->internalNodes[0] != 0 && node->nodeCount < kMaxNodeCount)
    {
        listDir(node->internalNodes[node->nodeCount], result);
    }
}

