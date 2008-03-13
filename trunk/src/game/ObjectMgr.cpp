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

	// check data correctness
	for(uint32 i = 1; i < sCreatureStorage.MaxEntry; ++i)
	{
		CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(i);

		if(!cInfo)
			continue;

		//std::string s = "";
		//s = cInfo->ScriptName;
		sLog.outString("ObjectMgr::LoadCreatureTemplate Checking for ENTRY(%u) NAME('%s') SCRIPTNAME('%s')", cInfo->Entry, cInfo->Name, cInfo->ScriptName);

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

	sLog.outString( " - Adding Creature %u GUID(%u) Map(%u[%u,%u]) NpcId(%u) CellId(%u)", data->id, guid, data->mapid, data->posX, data->posY, data->map_npcid, cell_id);

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
