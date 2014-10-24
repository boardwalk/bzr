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
#include "Core.h"
#include "Log.h"

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

void BlobAssembler::addFragment(const FragmentHeader* fragment)
{
    // shortcut common case
    if(fragment->count == 1 && fragment->index == 0)
    {
        if(fragment->queueId <= kInvalid || fragment->queueId >= kMax)
        {
            throw runtime_error("bad queue id");
        }

        BlobPtr blob;

        void* p = malloc(sizeof(Blob) + kMaxFragmentSize);
        blob.reset(reinterpret_cast<Blob*>(p));
        blob->count = 1;
        blob->queueId = fragment->queueId;
        blob->fragmentsReceived = 1;

        const void* source = reinterpret_cast<const uint8_t*>(fragment) + sizeof(FragmentHeader);
        void* dest = reinterpret_cast<uint8_t*>(blob.get()) + sizeof(Blob);

        memcpy(dest, source, fragment->size - sizeof(FragmentHeader));

        completeBlobs_.push_back(move(blob));
        return;
    }

    BlobPtr& blob = partialBlobs_[fragment->id];

    if(!blob)
    {
        if(fragment->count < 1 || fragment->count > sizeof(uint32_t) * CHAR_BIT)
        {
            throw runtime_error("bad fragment count");
        }

        if(fragment->queueId <= kInvalid || fragment->queueId >= kMax)
        {
            throw runtime_error("bad queue id");
        }

        void* p = malloc(sizeof(Blob) + kMaxFragmentSize * fragment->count);
        blob.reset(reinterpret_cast<Blob*>(p));
        blob->count = fragment->count;
        blob->queueId = fragment->queueId;
        blob->fragmentsReceived = 0;
    }
    else
    {
        if(fragment->count != blob->count)
        {
            throw runtime_error("inconsistent fragment count");
        }

        if(fragment->queueId != blob->queueId)
        {
            throw runtime_error("inconsistent queue id");
        }
    }

    if(fragment->index >= fragment->count)
    {
        throw runtime_error("fragment index out of range");
    }

    if(fragment->index < fragment->count - 1 && fragment->size != kMaxFragmentSize + sizeof(FragmentHeader))
    {
        throw runtime_error("bad fragment size before last fragment");
    }

    if(fragment->count == fragment->index - 1)
    {
        blob->size = kMaxFragmentSize * (fragment->count - 1) + (fragment->size - sizeof(FragmentHeader));
    }

    blob->fragmentsReceived |= (1 << fragment->index);

    const void* source = reinterpret_cast<const uint8_t*>(fragment) + sizeof(FragmentHeader);
    void* dest = reinterpret_cast<uint8_t*>(blob.get()) + sizeof(Blob) + kMaxFragmentSize * fragment->index;

    memcpy(dest, source, fragment->size - sizeof(FragmentHeader));

    if(blob->fragmentsReceived == (1u << fragment->count) - 1u)
    {
        completeBlobs_.push_back(move(blob));
        partialBlobs_.erase(fragment->id);
    }
}

void BlobAssembler::getBlobs(vector<BlobPtr>& blobs)
{
    completeBlobs_.swap(blobs);
    completeBlobs_.clear();
}
