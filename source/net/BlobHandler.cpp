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
#include "Core.h"
#include "Log.h"
#include "BinReader.h"

enum class BoolProperty
{
    kUndef = 0x0000,
    kStuck = 0x0001,
    kOpen = 0x0002,
    kLocked = 0x0003,
    kRotProof = 0x0004,
    kAllegianceUpdateRequest = 0x0005,
    kAIUsesMana = 0x0006,
    kAIUseHumanMagicAnimations = 0x0007,
    kAllowGive = 0x0008,
    kCurrentlyAttacking = 0x0009,
    kAttackerAI = 0x000a,
    kIgnoreCollisions = 0x000b,
    kReportCollisions = 0x000c,
    kEthereal = 0x000d,
    kGravityStatus = 0x000e,
    kLightsStatus = 0x000f,
    kScriptedCollision = 0x0010,
    kInelastic = 0x0011,
    kVisibility = 0x0012,
    kAttackable = 0x0013,
    kSafeSpellComponents = 0x0014,
    kAdvocateState = 0x0015,
    kInscribable = 0x0016,
    kDestroyOnSell = 0x0017,
    kUIHidden = 0x0018,
    kIgnoreHouseBarriers = 0x0019,
    kHiddenAdmin = 0x001a,
    kPKWounder = 0x001b,
    kPKKiller = 0x001c,
    kNoCorpse = 0x001d,
    kUnderLifestoneProtection = 0x001e,
    kItemManaUpdatePending = 0x001f,
    kGeneratorStatus = 0x0020,
    kResetMessagePending = 0x0021,
    kDefaultOpen = 0x0022,
    kDefaultLocked = 0x0023,
    kDefaultOn = 0x0024,
    kOpenForBusiness = 0x0025,
    kIsFrozen = 0x0026,
    kDealMagicalItems = 0x0027,
    kLogoffImDead = 0x0028,
    kReportCollisionsAsEnvironment = 0x0029,
    kAllowEdgeSlide = 0x002a,
    kAdvocateQuest = 0x002b,
    kIsAdmin = 0x002c,
    kIsArch = 0x002d,
    kIsSentinel = 0x002e,
    kIsAdvocate = 0x002f,
    kCurrentlyPoweringUp = 0x0030,
    kGeneratorEnteredWorld = 0x0031,
    kNeverFailCasting = 0x0032,
    kVendorService = 0x0033,
    kAIImmobile = 0x0034,
    kDamagedByCollisions = 0x0035,
    kIsDynamic = 0x0036,
    kIsHot = 0x0037,
    kIsAffecting = 0x0038,
    kAffectsAIs = 0x0039,
    kSpellQueueActive = 0x003a,
    kGeneratorDisabled = 0x003b,
    kIsAcceptingTells = 0x003c,
    kLoggingChannel = 0x003d,
    kOpensAnyLock = 0x003e,
    kUnlimitedUse = 0x003f,
    kGeneratedTreasureItem = 0x0040,
    kIgnoreMagicResist = 0x0041,
    kIgnoreMagicArmor = 0x0042,
    kAIAllowTrade = 0x0043,
    kSpellComponentsRequired = 0x0044,
    kIsSellable = 0x0045,
    kIgnoreShieldsBySkill = 0x0046,
    kNodraw = 0x0047,
    kActivationUntargeted = 0x0048,
    kHouseHasGottenPriorityBootPos = 0x0049,
    kGeneratorAutomaticDestruction = 0x004a,
    kHouseHooksVisible = 0x004b,
    kHouseRequiresMonarch = 0x004c,
    kHouseHooksEnabled = 0x004d,
    kHouseNotifiedHudOfHookCount = 0x004e,
    kAIAcceptEverything = 0x004f,
    kIgnorePortalRestrictions = 0x0050,
    kRequiresBackpackSlot = 0x0051,
    kDontTurnOrMoveWhenGiving = 0x0052,
    kNPCLooksLikeObject = 0x0053,
    kIgnoreCloIcons = 0x0054,
    kAppraisalHasAllowedWielder = 0x0055,
    kChestRegenOnClose = 0x0056,
    kLogoffInMinigame = 0x0057,
    kPortalShowDestination = 0x0058,
    kPortalIgnoresPKAttackTimer = 0x0059,
    kNPCInteractsSilently = 0x005a,
    kRetained = 0x005b,
    kIgnoreAuthor = 0x005c,
    kLimbo = 0x005d,
    kAppraisalHasAllowedActivator = 0x005e,
    kExistedBeforeAllegianceXPChanges = 0x005f,
    kIsDeaf = 0x0060,
    kIsPsr = 0x0061,
    kInvincible = 0x0062,
    kIvoryable = 0x0063,
    kDyable = 0x0064,
    kCanGenerateRare = 0x0065,
    kCorpseGeneratedRare = 0x0066,
    kNonProjectileMagicImmune = 0x0067,
    kActdReceivedItems = 0x0069,
    kFirstEnterWorldDone = 0x006a,
    kRecallsDisabled = 0x006b,
    kRareUsesTimer = 0x006c,
    kActdPreorderReceivedItems = 0x006d,
    kAFK = 0x006e,
    kIsGagged = 0x006f,
    kProcSpellSelfTargeted = 0x0070,
    kIsAllegianceGagged = 0x0071,
    kEquipmentSetTriggerPiece = 0x0072,
    kUninscribe = 0x0073,
    kWieldOnUse = 0x0074,
    kChestClearedWhenClosed = 0x0075,
    kNeverAttack = 0x0076,
    kSuppressGenerateEffect = 0x0077,
    kTreasureCorpse = 0x0078,
    kEquipmentSetAddLevel = 0x0079,
    kBarberActive = 0x007a,
    kTopLayerPriority = 0x007b,
    kNoHeldItemShown = 0x007c,
    kLoginAtLifestone = 0x007d,
    kOlthoiPK = 0x007e,
    kAccount15Days = 0x007f,
    kHadNoVitae = 0x0080,
    kNoOlthoiTalk = 0x0081,
    kAutowieldLeft = 0x0082
};

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
}
