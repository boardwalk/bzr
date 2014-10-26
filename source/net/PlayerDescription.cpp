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
#include "BinReader.h"
#include "Core.h"
#include "Log.h"
#include "Object.h"
#include "util.h"

enum PlayerDescFlags
{
    Packed_IntHashTable = 0x0001,
    Packed_BoolHashTable = 0x0002,
    Packed_FloatStats = 0x0004,
    Packed_DataIDHashTable = 0x0008,
    Packed_StringHashTable = 0x0010,
    Packed_PositionHashTable = 0x0020,
    Packed_InstanceIDHashTable = 0x0040,
    Packed_Int64HashTable = 0x0080
};

void handlePlayerDescription(BinReader& reader)
{
    unique_ptr<Object> objectPtr(new Object());
    Object& object = *objectPtr;

    uint32_t flags = reader.readInt();
    /*unknown1*/ reader.readInt();

    if(flags & Packed_IntHashTable)
    {
        uint16_t numInt = reader.readShort();
        /*unknown*/ reader.readShort();

        for(uint16_t i = 0; i < numInt; i++)
        {
            IntProperty property = IntProperty(reader.readInt());
            uint32_t value = reader.readInt();
            object.setProperty(property, value);
        }
    }

    if(flags & Packed_Int64HashTable)
    {
        uint16_t numInt64 = reader.readShort();
        /*unknown*/ reader.readShort();

        for(uint16_t i = 0; i < numInt64; i++)
        {
            Int64Property property = Int64Property(reader.readInt());
            uint64_t value = reader.readLong();
            object.setProperty(property, value);
        }   
    }

    if(flags & Packed_BoolHashTable)
    {
        uint16_t numBool = reader.readShort();
        /*unknown*/ reader.readShort();

        for(uint16_t i = 0; i < numBool; i++)
        {
            BoolProperty property = BoolProperty(reader.readInt());
            uint32_t value = reader.readInt();
            object.setProperty(property, value != 0);
        }   
    }

    if(flags & Packed_FloatStats)
    {
        uint16_t numFloat = reader.readShort();
        /*unknown*/ reader.readShort();

        for(uint16_t i = 0; i < numFloat; i++)
        {
            FloatProperty property = FloatProperty(reader.readInt());
            double value = reader.readDouble();
            object.setProperty(property, value);
        }
    }

    if(flags & Packed_StringHashTable)
    {
        uint16_t numString = reader.readShort();
        /*unknown*/ reader.readShort();

        for(uint16_t i = 0; i < numString; i++)
        {
            StringProperty property = StringProperty(reader.readInt());
            string value = reader.readString();
            object.setProperty(property, value);
        }
    }

    if(flags & Packed_DataIDHashTable)
    {
        uint16_t numDID = reader.readShort();
        /*unknown*/ reader.readShort();

        for(uint16_t i = 0; i < numDID; i++)
        {
            DIDProperty property = DIDProperty(reader.readInt());
            uint32_t value = reader.readInt();
            object.setProperty(property, value);
        }
    }

    if(flags & Packed_InstanceIDHashTable)
    {
        uint16_t numIID = reader.readShort();
        /*unknown*/ reader.readShort();

        for(uint16_t i = 0; i < numIID; i++)
        {
            IIDProperty property = IIDProperty(reader.readInt());
            uint32_t value = reader.readInt();
            object.setProperty(property, value);
        }
    }

    if(flags & Packed_PositionHashTable)
    {
        uint16_t numPosition = reader.readShort();
        /*unknown*/ reader.readShort();

        for(uint16_t i = 0; i < numPosition; i++)
        {
            PositionProperty property = PositionProperty(reader.readInt());
            Position value;
            value.landcell = LandcellId(reader.readInt());
            read(reader, value.position);
            read(reader, value.rotation);
            object.setProperty(property, value);
        }
    }

    // TODO more to do
}
