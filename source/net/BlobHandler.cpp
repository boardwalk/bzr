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
#include "net/BlobHandler.h"
#include "net/BlobWriter.h"
#include "BinReader.h"
#include "Core.h"
#include "Log.h"
#include "Property.h"

BlobHandler::BlobHandler() : sequence_(1)
{}

void BlobHandler::handle(BlobPtr blob)
{
    BinReader reader(blob.get() + 1, blob->size);

    MessageType messageType = static_cast<MessageType>(reader.readInt());

    if(messageType == MessageType::kWeenie_Ordered)
    {
        /*objectId*/ reader.readInt();
        uint32_t messageSequence = reader.readInt();

        orderedBlobs_[messageSequence] = move(blob);
        pumpOrderedBlobs();
    }
    else
    {
        handle(messageType, reader);
    }
}

void BlobHandler::pumpOrderedBlobs()
{
    for(auto it = orderedBlobs_.begin(); it != orderedBlobs_.end(); it = orderedBlobs_.erase(it))
    {
        if(it->first > sequence_)
        {
            break;
        }

        BinReader reader(it->second.get() + 1, it->second->size);

        /*messageType*/ reader.readInt();
        /*objectId*/ reader.readInt();
        /*messageSequence*/ reader.readInt();
        MessageType actualMessageType = static_cast<MessageType>(reader.readInt());

        handle(actualMessageType, reader);
        sequence_++;
    }
}

// CreateObject.cpp
void handleCreateObject(BinReader& reader);

void BlobHandler::handle(MessageType messageType, BinReader& reader)
{
    LOG(Net, Info) << "received blob " << hexn(static_cast<uint32_t>(messageType)) << " " << getMessageName(messageType) << "\n";

    if(messageType == MessageType::kLogin_CharacterSet)
    {
        /*unknown1*/ reader.readInt();
        uint32_t characterCount = reader.readInt();

        for(uint32_t i = 0; i < characterCount; i++)
        {
            uint32_t objectId = reader.readInt();
            string name = reader.readString();
            uint32_t deleteTimeout = reader.readInt();

            LOG(Net, Debug) << "  objectId=" << hexn(objectId) << " name=" << name << " deleteTimeout=" << deleteTimeout << "\n";
        }
    }
    else if(messageType == MessageType::kLogin_WorldInfo)
    {
        uint32_t playerCount = reader.readInt();
        uint32_t unknown = reader.readInt();
        string server = reader.readString();

        LOG(Net, Debug) << "  playerCount=" << playerCount << " unknown=" << hexn(unknown) << " server=" << server << "\n";
    }
    else if(messageType == MessageType::kDDD_Interrogation)
    {
        uint32_t serversRegion = reader.readInt();
        assert(serversRegion == 1);

        uint32_t nameRuleLanguage = reader.readInt();
        assert(nameRuleLanguage == 1);

        uint32_t productId = reader.readInt();
        assert(productId == 1);

        uint32_t numSupportedLanguages = reader.readInt();
        assert(numSupportedLanguages == 2);

        for(uint32_t i = 0; i < numSupportedLanguages; i++)
        {
            /*supportedLanguage*/ reader.readInt();
        }

        assert(reader.remaining() == 0);

        BlobWriter writer(MessageType::kDDD_InterrogationResponse, NetQueueId::kDatabase);
        /*clientLanguage*/ writer.writeInt(1);
        /*numItersWithKeys*/ writer.writeInt(0);
        /*numItersWithoutKeys*/ writer.writeInt(0);
        /*flags*/ writer.writeInt(0);
    }
    else if(messageType == MessageType::kDDD_EndDDD)
    {
        BlobWriter writer(MessageType::kDDD_EndDDD, NetQueueId::kDatabase);
        // empty body

        BlobWriter writer2(MessageType::kClient_Request_Enter_Game, NetQueueId::kLogon);
        // empty body
    }
    else if(messageType == MessageType::kCharacter_EnterGame_ServerReady)
    {
        //NET   DEBUG    objectId=500F4D79h name=Aeriin deleteTimeout=0
        //NET   DEBUG    objectId=500F4CFBh name=Madlyn deleteTimeout=0
        //NET   DEBUG    objectId=500F4D7Ah name=Madds deleteTimeout=0

        BlobWriter writer(MessageType::kCharacter_Enter_Game, NetQueueId::kLogon);
        writer.writeInt(0x500F4CFB);
        writer.writeString("md9nq9m3njxjlpdcwy4anqdnq");
    }
    else if(messageType == MessageType::kQualities_PrivateUpdateBool)
    {
        /*sequence*/ reader.readByte();
        BoolProperty property = BoolProperty(reader.readInt());
        uint32_t value = reader.readInt();
        LOG(Net, Debug) << " set bool " << getBoolPropertyName(property) << " to " << (value ? "true" : "false") << "\n";
    }
    else if(messageType == MessageType::kQualities_PrivateUpdateInt)
    {
        /*sequence*/ reader.readByte();
        IntProperty property = IntProperty(reader.readInt());
        uint32_t value = reader.readInt();
        LOG(Net, Debug) << " set int " << getIntPropertyName(property) << " to " << value << "\n";
    }
    else if(messageType == MessageType::kQualities_PrivateUpdateFloat)
    {
        /*sequence*/ reader.readByte();
        FloatProperty property = FloatProperty(reader.readInt());
        double value = reader.readDouble();
        LOG(Net, Debug) << " set float " << getFloatPropertyName(property) << " to " << value << "\n";
    }
    else if(messageType == MessageType::kPlayer_Description)
    {
        uint32_t flags = reader.readInt();
        /*unknown1*/ reader.readInt();

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

        if(flags & Packed_IntHashTable)
        {
            uint16_t numInt = reader.readShort();
            /*unknown*/ reader.readShort();

            for(uint16_t i = 0; i < numInt; i++)
            {
                IntProperty property = IntProperty(reader.readInt());
                uint32_t value = reader.readInt();
                LOG(Net, Debug) << " set int " << getIntPropertyName(property) << " to " << hexn(value) << "\n";
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
                LOG(Net, Debug) << " set int64 " << getInt64PropertyName(property) << " to " << hexn(value) << "\n";
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
                LOG(Net, Debug) << " set bool " << getBoolPropertyName(property) << " to " << (value ? "true" : "false") << "\n";
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
                LOG(Net, Debug) << " set float " << getFloatPropertyName(property) << " to " << value << "\n";
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
                LOG(Net, Debug) << " set string " << getStringPropertyName(property) << " to " << value << "\n";
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
                LOG(Net, Debug) << " set DID " << getDIDPropertyName(property) << " to " << hexn(value) << "\n";
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
                LOG(Net, Debug) << " set IID " << getIIDPropertyName(property) << " to " << hexn(value) << "\n";
            }
        }

        if(flags & Packed_PositionHashTable)
        {
            uint16_t numPosition = reader.readShort();
            /*unknown*/ reader.readShort();

            for(uint16_t i = 0; i < numPosition; i++)
            {
                PositionProperty property = PositionProperty(reader.readInt());
                reader.readRaw(32);
                LOG(Net, Debug) << " set position " << getPositionPropertyName(property) << "\n";
            }
        }

        // TODO more to do
    }
    else if(messageType == MessageType::kPhysics_CreateObject)
    {
        handleCreateObject(reader);
    }
}
