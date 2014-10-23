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
#include "net/BlobAssembler.h"

enum NetQueue
{
    kInvalid,
    kEvent,
    kControl,
    kWeenie,
    kLogon,
    kDatabase,
    kSecureControl,
    kSecureWeenie,
    kSecureLogon,
    kUI,
    kSmartBox,
    kObserver,
    kMax
};

void BlobAssembler::add(const FragmentHeader* header)
{
    // shortcut common case
    if(header->count == 1 && header->index == 0)
    {
        if(header->queueId <= kInvalid || header->queueId >= kMax)
        {
            throw runtime_error("bad queue id");
        }

        BlobPtr blob;

        void* p = malloc(sizeof(Blob) + kMaxFragmentSize);
        blob.reset(reinterpret_cast<Blob*>(p));
        blob->count = 1;
        blob->queueId = header->queueId;
        blob->fragmentsReceived = 1;

        completeBlobs_.push_back(move(blob));
    }

    BlobPtr& blob = partialBlobs_[header->id];

    if(!blob)
    {
        if(header->count < 1 || header->count > sizeof(uint32_t) * CHAR_BIT)
        {
            throw runtime_error("bad fragment count");
        }

        if(header->queueId <= kInvalid || header->queueId >= kMax)
        {
            throw runtime_error("bad queue id");
        }

        void* p = malloc(sizeof(Blob) + kMaxFragmentSize * header->count);
        blob.reset(reinterpret_cast<Blob*>(p));
        blob->count = header->count;
        blob->queueId = header->queueId;
        blob->fragmentsReceived = 0;
    }
    else
    {
        if(header->count != blob->count)
        {
            throw runtime_error("inconsistent fragment count");
        }

        if(header->queueId != blob->queueId)
        {
            throw runtime_error("inconsistent queue id");
        }
    }

    if(header->index >= header->count)
    {
        throw runtime_error("fragment index out of range");
    }

    if(header->index < header->count - 1 && header->size != kMaxFragmentSize + sizeof(FragmentHeader))
    {
        throw runtime_error("bad fragment size before last fragment");
    }

    if(header->count == header->index - 1)
    {
        blob->size = kMaxFragmentSize * (header->count - 1) + (header->size - sizeof(FragmentHeader));
    }

    blob->fragmentsReceived |= (1 << header->index);

    const void* source = reinterpret_cast<const uint8_t*>(header) + sizeof(FragmentHeader);
    void* dest = reinterpret_cast<uint8_t*>(blob.get()) + sizeof(Blob) + kMaxFragmentSize * header->index;

    memcpy(dest, source, header->size - sizeof(FragmentHeader));

    if(blob->fragmentsReceived == (1 << header->count) - 1)
    {
        completeBlobs_.push_back(move(blob));
        partialBlobs_.erase(header->id);
    }
}

void BlobAssembler::clear()
{
    completeBlobs_.clear();
}

BlobAssembler::iterator BlobAssembler::begin()
{
    return completeBlobs_.begin();
}

BlobAssembler::iterator BlobAssembler::end()
{
    return completeBlobs_.end();
}
