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
#include "PhysicsScriptTable.h"
#include "BinReader.h"

static void read(BinReader& reader, PhysicsScriptTableData& scripts)
{
    uint32_t numScripts = reader.readInt();
    scripts.scripts.resize(numScripts);

    for(ScriptAndModData& script : scripts.scripts)
    {
        script.mod = reader.readFloat();
        script.scriptId = reader.readInt();
        assert((script.scriptId & 0xFF000000) == ResourceType::kPhysicsScript);
    }
}

PhysicsScriptTable::PhysicsScriptTable(uint32_t id, const void* data, size_t size) : ResourceImpl(id)
{
    BinReader reader(data, size);

    uint32_t resourceId = reader.readInt();
    assert(resourceId == id);

    uint32_t scriptTableSize = reader.readInt();

    for(uint32_t i = 0; i < scriptTableSize; i++)
    {
        uint32_t scriptType = reader.readInt();

        read(reader, scriptTable[scriptType]);
    }

    reader.assertEnd();
}
