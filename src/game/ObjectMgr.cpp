/*
 * Copyright (C) 2008-2008 LeGACY <http://www.legacy-project.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Database/SQLStorage.h"

#include "Log.h"
#include "MapManager.h"
#include "ObjectMgr.h"

#include "World.h"
#include "WorldSession.h"

#include "Policies/SingletonImp.h"

#include "Spell.h"
#include "Chat.h"

INSTANTIATE_SINGLETON_1(ObjectMgr);

ScriptMapMap sQuestEndScripts;
ScriptMapMap sQuestStartScripts;
ScriptMapMap sSpellScripts;
ScriptMapMap sGameObjectScripts;
ScriptMapMap sEventScripts;

ObjectMgr::ObjectMgr()
{
}

ObjectMgr::~ObjectMgr()
{
}

void ObjectMgr::LoadCreatureTemplates()
{
	sCreatureStorage.Load();

	sLog.outString( ">> Loaded %u creature definitions", sCreatureStorage.RecordCount );
	sLog.outString();

	sLog.outString("SizeOf CreatureInfo: %u", sizeof(CreatureInfo));
	// check data correctness
	sLog.outString("ObjectMgr::LoadCreatureTemplate");
	sLog.outString(" -       %-5s %10s %5s %2s %-10s", "ENTRY","NAME", "LEVEL", "EL", "SCRIPT");
	for(uint32 i = 1; i < sCreatureStorage.MaxEntry; ++i)
	{
		CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(i);

		if(!cInfo)
			continue;

		sLog.outString(" - Check %5u %10s %5u %2u %10s", cInfo->Entry, cInfo->Name, cInfo->level, cInfo->element, cInfo->ScriptName);
		//std::string s = "";
		//s = cInfo->ScriptName;

	}
}

void ObjectMgr::LoadCreatures()
{
	QueryResult *result = WorldDatabase.Query("SELECT * FROM creature");

	if(!result)
	{
		sLog.outString();
		sLog.outString(">> Loaded 0 creature. DB table `creature` is empty.");
		return;
	}

	do
	{
		Field *f = result->Fetch();

		uint32 guid = f[0].GetUInt32();

		CreatureData& data = mCreatureDataMap[guid];

		data.id            = f[ 1].GetUInt32();
		data.mapid         = f[ 2].GetUInt16();
		data.map_npcid     = f[ 3].GetUInt8();
		data.posX          = f[ 4].GetUInt16();
		data.posY          = f[ 5].GetUInt16();

		data.spawntimesecs = f[ 6].GetUInt32();

		data.spawn_posX    = f[ 7].GetUInt16();
		data.spawn_posY    = f[ 8].GetUInt16();

		data.hp            = f[ 9].GetUInt16();
		data.sp            = f[10].GetUInt16();

		data.deathState    = f[11].GetUInt8();
		data.movementType  = f[12].GetUInt8();

		data.team00        = f[13].GetUInt32();
		data.team01        = f[14].GetUInt32();
		data.team03        = f[15].GetUInt32();
		data.team04        = f[16].GetUInt32();

		data.team10        = f[17].GetUInt32();
		data.team11        = f[18].GetUInt32();
		data.team12        = f[19].GetUInt32();
		data.team13        = f[20].GetUInt32();
		data.team14        = f[21].GetUInt32();

		AddCreatureToGrid(guid, &data);


	} while (result->NextRow());

	delete result;

	sLog.outString();
	sLog.outString( ">> Loaded %u creatures", mCreatureDataMap.size() );
}

void ObjectMgr::AddCreatureToGrid(uint32 guid, CreatureData const* data)
{
	CellPair cell_pair = LeGACY::ComputeCellPair(data->posX, data->posY);
	uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

	sLog.outString( " - Adding Creature %4u GUID(%7u) Map(%5u[%4u,%4u]) NpcId(%1u) CellId(%1u)", data->id, guid, data->mapid, data->posX, data->posY, data->map_npcid, cell_id);

	CellObjectGuids& cell_guids = mMapObjectGuids[data->mapid][cell_id];
	cell_guids.creatures.insert(guid);
}

void ObjectMgr::RemoveCreatureFromGrid(uint32 guid, CreatureData const* data)
{
	CellPair cell_pair = LeGACY::ComputeCellPair(data->posX, data->posY);
	uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

	CellObjectGuids& cell_guids = mMapObjectGuids[data->mapid][cell_id];
	cell_guids.creatures.erase(guid);
}

CreatureInfo const* ObjectMgr::GetCreatureTemplate(uint32 id)
{
	return sCreatureStorage.LookupEntry<CreatureInfo>(id);
}

/*
void ObjectMgr::LoadScripts(ScriptMapMap& scripts, char const* tablename)
{
}
*/

void ObjectMgr::LoadGameObjectScripts()
{
	//LoadScripts
}

void ObjectMgr::LoadPetNumber()
{
	QueryResult* result = CharacterDatabase.Query("SELECT MAX(id) FROM character_pet");
	if(result)
	{
		Field *f = result->Fetch();
		m_hiPetNumber = f[0].GetUInt32()+1;
		delete result;
	}

	sLog.outString("");
	sLog.outString(">> Loaded the max pet number: %d", m_hiPetNumber-1);
}

void ObjectMgr::SetHighestGuids()
{
	QueryResult *result = CharacterDatabase.Query("SELECT MAX(guid) FROM characters");
	if(result)
	{
		m_hiCharGuid = (*result)[0].GetUInt32()+1;
		delete result;
	}

	result = WorldDatabase.Query("SELECT MAX(guid) FROM creature");
	if(result)
	{
		m_hiCreatureGuid = (*result)[0].GetUInt32()+1;
		delete result;
	}

	result = CharacterDatabase.Query("SELECT MAX(guid) FROM item_instance");
	if(result)
	{
		m_hiItemGuid = (*result)[0].GetUInt32()+1;
		delete result;
	}

	result = WorldDatabase.Query("SELECT MAX(guid) FROM gameobject");
	if(result)
	{
		m_hiGoGuid = (*result)[0].GetUInt32()+1;
		delete result;
	}
}

uint32 ObjectMgr::GenerateLowGuid(HighGuid guidhigh)
{
	switch(guidhigh)
	{
		case HIGHGUID_ITEM:
			++m_hiItemGuid;
			if(m_hiItemGuid==0xFFFFFFFF)
			{
				sLog.outError("Item guid overflow!! Can't continue, shutting down server.");
				sWorld.m_stopEvent = true;
			}
			return m_hiItemGuid;
			
		case HIGHGUID_GAMEOBJECT:
			++m_hiGoGuid;
			if(m_hiGoGuid==0xFFFFFFFF)
			{
				sLog.outError("Gameobject guid overflow!! Can't continue shutting down server.");
				sWorld.m_stopEvent = true;
			}
			return m_hiGoGuid;

		case HIGHGUID_PLAYER:
			++m_hiCharGuid;
			if(m_hiCharGuid==0xFFFFFFFF)
			{
				sLog.outError("Players guid overflow!! Can't continue, shutting down server.");
				sWorld.m_stopEvent = true;
			}
			return m_hiCharGuid;

		case HIGHGUID_UNIT:
			++m_hiCreatureGuid;
			if(m_hiCreatureGuid==0xFFFFFFFF)
			{
				sLog.outError("Pet guid overflow!! Can't continue, shutting down server.");
				sWorld.m_stopEvent = true;
			}
			return m_hiCreatureGuid;

		default:
			ASSERT(0);
	}

	ASSERT(0);
	return 0;
}
