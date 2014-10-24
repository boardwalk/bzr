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
#ifndef BZR_NET_BLOBHANDLER_H
#define BZR_NET_BLOBHANDLER_H

#include "net/Blob.h"
#include "net/MessageType.h"
#include "Noncopyable.h"
#include <map>

class BinReader;

class BlobHandler : Noncopyable
{
public:
    BlobHandler();

    void handle(BlobPtr blob);

private:
    void pumpOrderedBlobs();
    void handle(MessageType messageType, BinReader& reader);

    // sequence of the next WEENIE_ORDERED blob
    // starts at 1, persistent across sessions
    uint32_t sequence_;

    // contains WEENIE_ORDERED blobs with sequence >= sequence_ keyed by sequence
    map<uint32_t, BlobPtr> orderedBlobs_;
};

#endif