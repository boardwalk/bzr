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
#include "ObjectManager.h"
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

// PlayerDescription.cpp
void handlePlayerDescription(BinReader& reader);

// CreateObject.cpp
void handleCreateObject(BinReader& reader);

void BlobHandler::handle(MessageType messageType, BinReader& reader)
{
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
        Core::get().objectManager().setPlayerId(ObjectId{0x500F4CFB});
    }
    else if(messageType == MessageType::kPhysics_CreatePlayer)
    {
        ObjectId playerId = ObjectId{reader.readInt()};
        Core::get().objectManager().setPlayerId(playerId);
    }
    else if(messageType == MessageType::kPlayer_Description)
    {
        handlePlayerDescription(reader);
    }
    else if(messageType == MessageType::kPhysics_CreateObject)
    {
        handleCreateObject(reader);
    }
    else if(messageType == MessageType::kQualities_PrivateUpdateBool)
    {
        /*sequence*/ reader.readByte();
        BoolProperty property = BoolProperty(reader.readInt());
        uint32_t value = reader.readInt();
        Core::get().objectManager().player().setProperty(property, value != 0);
    }
    else if(messageType == MessageType::kQualities_PrivateUpdateInt)
    {
        /*sequence*/ reader.readByte();
        IntProperty property = IntProperty(reader.readInt());
        uint32_t value = reader.readInt();
        Core::get().objectManager().player().setProperty(property, value);
    }
    else if(messageType == MessageType::kQualities_PrivateUpdateFloat)
    {
        /*sequence*/ reader.readByte();
        FloatProperty property = FloatProperty(reader.readInt());
        double value = reader.readDouble();
        Core::get().objectManager().player().setProperty(property, value);
    }
    else if(messageType == MessageType::kQualities_UpdateDataID)
    {
        /*sequence*/ reader.readByte();
        ObjectId objectId = ObjectId{reader.readInt()};
        DIDProperty property = DIDProperty(reader.readInt());
        uint32_t value = reader.readInt();
        Core::get().objectManager()[objectId].setProperty(property, value);
    }
    else if(messageType == MessageType::kPhysics_ObjDescEvent)
    {
        // ignore for now
    }
    else if(messageType == MessageType::kMovement_UpdatePosition)
    {
        // ignore for now
    }
    else if(messageType == MessageType::kMovement_MovementEvent)
    {
        // ignore for now
    }
    else if(messageType == MessageType::kPhysics_PlayScriptType)
    {
        // ignore for now
    }
    else
    {
        LOG(Net, Info) << "received blob " << hexn(static_cast<uint32_t>(messageType)) << " " << getMessageName(messageType) << "\n";
    }
}
