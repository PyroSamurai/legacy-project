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

#ifndef __LEGACY_OBJECTMGR_H
#define __LEGACY_OBJECTMGR_H

#include "Log.h"
#include "Object.h"
#include "Bag.h"
#include "Creature.h"
#include "Player.h"
#include "DynamicObject.h"
#include "GameObject.h"
#include "Corpse.h"
#include "ItemPrototype.h"
#include "NPCHandler.h"
#include "Database/DatabaseEnv.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "ObjectDefines.h"
#include "Policies/Singleton.h"
#include "Database/SQLStorage.h"

#include <string>
#include <map>

extern SQLStorage sCreatureStorage;
extern SQLStorage sEquipmentStorage;
extern SQLStorage sGOStorage;
extern SQLStorage sPageTextStorage;
extern SQLStorage sItemStorage;

class Item;

struct ScriptInfo
{
	uint32 id;
	uint32 delay;
	uint32 command;
	uint32 datalong;
	uint32 datalong2;
	std::string datatext;
	float x;
	float y;
	float z;
	float o;
};

typedef std::multimap<uint32, ScriptInfo> ScriptMap;
typedef std::map<uint32, ScriptMap> ScriptMapMap;
extern ScriptMapMap sQuestEndScripts;
extern ScriptMapMap sQuestStartScripts;
extern ScriptMapMap sGameObjectScripts;
extern ScriptMapMap sEventScripts;

struct AreaTrigger
{
	uint8 requiredLevel;
	uint32 requiredItem;
	uint32 target_mapId;
	float target_X;
	float target_Y;
	float target_Z;
	float target_Orientation;
};

typedef std::set<uint32> CellGuidSet;
struct CellObjectGuids
{
	CellGuidSet creatures;
	CellGuidSet gameobjects;
};
typedef HM_NAMESPACE::hash_map<uint32/*cell_id*/,CellObjectGuids> CellObjectGuidsMap;
typedef HM_NAMESPACE::hash_map<uint32/*mapid*/,CellObjectGuidsMap> MapObjectGuids;




typedef HM_NAMESPACE::hash_map<uint32, CreatureData> CreatureDataMap;
typedef HM_NAMESPACE::hash_map<uint32, GameObjectData> GameObjectDataMap;

typedef std::multimap<uint32, uint32> QuestRelations;



/// Player state
enum SessionStatus
{
	STATUS_NOT_LOGGEDIN      = 0,  ///< Player not logged in
	STATUS_LOGGEDIN                ///< Player in game
};

struct OpcodeHandler
{
	OpcodeHandler() : status(STATUS_LOGGEDIN), handler(NULL) {};
	OpcodeHandler( SessionStatus _status, void (WorldSession::*_handler)(WorldPacket& recvPacket) ) : status(_status), handler(_handler) {};

	SessionStatus status;
	void (WorldSession::*handler)(WorldPacket& recvPacket);
};

typedef HM_NAMESPACE::hash_map< uint16, OpcodeHandler > OpcodeTableMap;

class ObjectMgr
{
	public:
		ObjectMgr();
		~ObjectMgr();

		void AddToWorld();
		void RemoveFromWorld();

		typedef HM_NAMESPACE::hash_map<uint32, Item*> ItemMap;


		typedef HM_NAMESPACE::hash_map<uint32, AreaTrigger> AreaTriggerMap;

	

		Player* GetPlayer(const char* name) const { return ObjectAccessor::Instance().FindPlayerByName(name); }
		Player* GetPlayer(uint32 guid) const { return ObjectAccessor::FindPlayer(guid); }


		static ItemPrototype const* GetItemPrototype(uint32 id) { return sItemStorage.LookupEntry<ItemPrototype>(id); }



		void LoadGameObjectScripts();
		void LoadQuestEndScripts() {}
		void LoadQuestStartScripts() {}
		void LoadEventScripts() {}
		void LoadSpellScripts() {}





		void LoadCreatureTemplates();
		void LoadCreatures();





		// grid objects
		void AddCreatureToGrid(uint32 guid, CreatureData const* data);
		void RemoveCreatureFromGrid(uint32 guid, CreatureData const* data);






		CreatureInfo const *GetCreatureTemplate( uint32 id );



		OpcodeTableMap opcodeTable;

		CellObjectGuids const& GetCellObjectGuids(uint32 mapid, uint32 cell_id)
		{
			return mMapObjectGuids[mapid][cell_id];
		}

		CreatureData const* GetCreatureData(uint32 guid) const
		{
			sLog.outString("    >> ObjectMgr::GetCreatureData %u", guid);
			CreatureDataMap::const_iterator itr = mCreatureDataMap.find(guid);
			if(itr==mCreatureDataMap.end()) return NULL;
			return &itr->second;
		}

		void LoadPetNumber();

		void SetHighestGuids();

		uint32 GenerateLowGuid(HighGuid guidhigh);

	protected:
		uint32 m_hiCharGuid;
		uint32 m_hiCreatureGuid;
		uint32 m_hiItemGuid;
		uint32 m_hiGoGuid;

		uint32 m_hiPetNumber;

	private:








		MapObjectGuids mMapObjectGuids;
		CreatureDataMap mCreatureDataMap;
};

#define objmgr LeGACY::Singleton<ObjectMgr>::Instance()
#endif