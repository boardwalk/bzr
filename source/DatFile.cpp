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

static const size_t HEADER_OFFSET = 0x140;
static const uint32_t HEADER_MAGIC_NUMBER = 0x5442; // 'BT\0\0'
static const int MAX_NODE_COUNT = 62;

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
    uint32_t internalNodes[MAX_NODE_COUNT];
    uint32_t nodeCount;
    DatLeaf leafNodes[MAX_NODE_COUNT];
});

DatFile::DatFile(const string& path) :
    _fs(path.c_str(), ios_base::in|ios_base::binary)
{
    DatHeader header;

    _fs.seekg(HEADER_OFFSET);
    _fs.read((char*)&header, sizeof(header));

    if(!_fs.good())
    {
        throw runtime_error("Could not read dat file header");
    }

    if(header.magicNumber != HEADER_MAGIC_NUMBER)
    {
        throw runtime_error("Dat file header has bad magic number");
    }

    _blockSize = header.blockSize - sizeof(uint32_t); // exclude next block position
    _rootPosition = header.rootPosition;
}

DatFile::~DatFile()
{}

vector<uint8_t> DatFile::read(uint32_t id) const
{
    auto nodePosition = _rootPosition;

    for(;;)
    {
        auto nodeData = readBlocks(nodePosition);
        auto node = (DatNode*)nodeData.data();

        if(node->nodeCount > MAX_NODE_COUNT)
        {
            throw runtime_error("Node has bad node count");
        }

        auto i = 0u;

        for(; i < node->nodeCount; i++)
        {
            if(id <= node->leafNodes[i].id)
            {
                break;
            }
        }

        if(i < node->nodeCount && id == node->leafNodes[i].id)
        {
            auto result = readBlocks(node->leafNodes[i].position);
            result.resize(node->leafNodes[i].size);
            return result;
        }

        if(node->internalNodes[0] == 0)
        {
            return vector<uint8_t>();
        }

        nodePosition = node->internalNodes[i];
    }
}

vector<uint8_t> DatFile::readBlocks(uint32_t position) const
{
    vector<uint8_t> result;

    while(position != 0)
    {
        result.resize(result.size() + _blockSize);

        _fs.seekg(position);
        _fs.read((char*)&position, sizeof(position));
        _fs.read((char*)result.data() + result.size() - _blockSize, _blockSize);

        if(!_fs.good())
        {
            throw runtime_error("Failed to read block");
        }
    }

    return result;
}

