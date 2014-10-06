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
#ifndef BZR_SPELLTABLE_H
#define BZR_SPELLTABLE_H

struct SpellFlag
{
    enum Value
    {
        kResistable = 0x00000001,
        kPKSensitive = 0x00000002,
        kBeneficial = 0x00000004,
        kSelfTargeted = 0x00000008,
        kReversed = 0x00000010,
        kNotIndoor = 0x00000020,
        kNotOutdoor = 0x00000040, // not used
        kNotResearchable = 0x00000080,
        kProjectile = 0x00000100, // not used
        kCreatureSpell = 0x00000200, // not used
        kExcludedFromItemDescriptions = 0x00000400,
        kIgnoresManaConversion = 0x00000800,
        kNonTrackingProjectile = 0x00001000,
        kFellowshipSpell = 0x00002000,
        kFastCast = 0x00004000,
        kIndoorLongRange = 0x00008000, // not used
        kDamageOverTime = 0x00010000
    };
};

#endif
