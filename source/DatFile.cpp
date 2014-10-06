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

static const uint32_t kHeaderMagicNumber = 0x5442; // 'BT\0\0'
static const int kMaxEntries = 61;

PACK(struct DiskFileInfo
{
    uint32_t magicNumber;
    uint32_t blockSize;
    uint32_t fileSize;
    uint32_t dataSet_lm;
    uint32_t dataSubset_lm;
    uint32_t firstFree;
    uint32_t finalFree;
    uint32_t freeBlockCount;
    uint32_t rootPosition;
    uint32_t youngLRU_lm;
    uint32_t oldLRU_lm;
    uint8_t useLRU_fm;
    uint8_t pad1;
    uint8_t pad2;
    uint8_t pad3;
    uint32_t masterMapId;
    uint32_t englishPackVersion;
    uint32_t gamePackVersion;
});

PACK(struct DiskHeaderBlock
{
    uint8_t acVersionStr[256];
    uint8_t acTransactionRecord[64];
    DiskFileInfo fileInfo;
});

PACK(struct BTEntry
{
    uint32_t flags; // comp, resv, ver?
    uint32_t id;
    uint32_t position;
    uint32_t size;
    uint32_t timestamp;
    uint32_t version;
});

PACK(struct BTNode
{
    uint32_t nextNode[kMaxEntries + 1];
    uint32_t numEntries;
    BTEntry entries[kMaxEntries];
});

DatFile::DatFile(const string& path) :
    fs_(path.c_str(), ios_base::in|ios_base::binary)
{
    DiskHeaderBlock headerBlock;
    fs_.read(reinterpret_cast<char*>(&headerBlock), sizeof(headerBlock));

    if(!fs_.good())
    {
        throw runtime_error("Could not read header block");
    }

    if(headerBlock.fileInfo.magicNumber != kHeaderMagicNumber)
    {
        throw runtime_error("Header block has bad magic number");
    }

    blockSize_ = headerBlock.fileInfo.blockSize - sizeof(uint32_t); // exclude next block position
    rootPosition_ = headerBlock.fileInfo.rootPosition;
}

vector<uint8_t> DatFile::read(uint32_t id) const
{
    uint32_t position = rootPosition_;

    for(;;)
    {
        vector<uint8_t> nodeData = readBlocks(position, sizeof(BTNode));
        BTNode* node = reinterpret_cast<BTNode*>(nodeData.data());

        if(node->numEntries > kMaxEntries)
        {
            throw runtime_error("Node has bad entry count");
        }

        uint32_t i = 0;

        for(; i < node->numEntries; i++)
        {
            if(id <= node->entries[i].id)
            {
                break;
            }
        }

        if(i < node->numEntries && id == node->entries[i].id)
        {
            return readBlocks(node->entries[i].position, node->entries[i].size);
        }

        if(node->nextNode[0] == 0)
        {
            return vector<uint8_t>();
        }

        position = node->nextNode[i];
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
    vector<uint8_t> nodeData = readBlocks(position, sizeof(BTNode));
    BTNode* node = reinterpret_cast<BTNode*>(nodeData.data());

    if(node->numEntries > kMaxEntries)
    {
        throw runtime_error("Node has bad entry count");
    }

    for(uint32_t i = 0; i < node->numEntries; i++)
    {
        if(node->nextNode[0] != 0)
        {
            listDir(node->nextNode[i], result);
        }

        result.push_back(node->entries[i].id);
    }

    if(node->nextNode[0] != 0)
    {
        listDir(node->nextNode[node->numEntries], result);
    }
}
