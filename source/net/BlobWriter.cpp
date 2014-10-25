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
#include "net/BlobWriter.h"
#include "net/SessionManager.h"
#include "Core.h"

BlobWriter::BlobWriter(MessageType messageType, NetQueueId queueId)
{
    void* p = malloc(sizeof(Blob) + kMaxFragmentSize);
    blob_.reset(reinterpret_cast<Blob*>(p));
    blob_->count = 1;
    blob_->queueId = static_cast<uint16_t>(queueId);

    data_ = blob_.get() + 1;
    size_ = kMaxFragmentSize;

    writeInt(static_cast<uint32_t>(messageType));
}

BlobWriter::~BlobWriter()
{
    blob_->size = position_;
    Core::get().sessionManager().sendBlob(move(blob_));
}
