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
//#include "Bag.h"
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
//extern SQLStorage sEquipmentStorage;
//extern SQLStorage sGOStorage;
//extern SQLStorage sPageTextStorage;
extern SQLStorage sItemStorage;
extern SQLStorage sSpellStorage;

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

typedef std::map<uint32, uint32> ItemPrototypeMap;


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


		static ItemPrototype const* GetItemPrototype(uint32 entry) { return sItemStorage.LookupEntry<ItemPrototype>(entry); }
		uint32 GetItemEntryByModelId(uint32 modelid);


		uint32 GetPlayerGuidByAccountId(const uint32 acc_id) const;
		uint64 GetPlayerGUIDByName(std::string name) const;
		bool GetPlayerNameByGUID(const uint64 &guid, std::string &name) const;

		uint32 GenerateNpcGuidLow(uint16 mapid, uint8 map_npcid) const
		{
			return (mapid * MAP_NPCID_MULTIPLIER) + map_npcid;
		}

		uint64 GetNpcGuidByMapNpcId(uint16 mapid, uint8 map_npcid) const
		{
			uint32 guid = GenerateNpcGuidLow(mapid, map_npcid);
			return MAKE_GUID(guid, HIGHGUID_UNIT);
		}

		void LoadGameObjectScripts();
		void LoadQuestEndScripts() {}
		void LoadQuestStartScripts() {}
		void LoadEventScripts() {}
		void LoadSpellScripts() {}





		void LoadCreatureTemplates();
		void LoadCreatures();
		void LoadCreature(uint32 guid);
		void _LoadCreatures(QueryResult *result, bool addtogrid=true);
		void LoadItemPrototypes();
		void LoadSpellPrototypes();

		bool IsCreatureExists(uint32 guid);




		// grid objects
		void AddCreatureToGrid(uint32 guid, CreatureData const* data);
		void RemoveCreatureFromGrid(uint32 guid, CreatureData const* data);






		CreatureInfo const *GetCreatureTemplate( uint32 entry );
		CreatureInfo const *GetCreatureTemplateByModelId( uint32 modelid );
		SpellInfo const *GetSpellTemplate( uint16 entry );

		uint32 GetCreatureGuidByModelId(uint16 modelid) const;


		OpcodeTableMap opcodeTable;

		CellObjectGuids const& GetCellObjectGuids(uint32 mapid, uint32 cell_id)
		{
			return mMapObjectGuids[mapid][cell_id];
		}

		CreatureData const* GetCreatureData(uint32 guid) const
		{
			//sLog.outString("    >> ObjectMgr::GetCreatureData %u", guid);
			CreatureDataMap::const_iterator itr = mCreatureDataMap.find(guid);
			if(itr==mCreatureDataMap.end()) return NULL;
			return &itr->second;
		}

		void LoadPetNumber();

		void SetHighestGuids();

		uint32 GenerateLowGuid(HighGuid guidhigh);
		uint32 GeneratePetNumber();

	protected:
		uint32 m_hiCharGuid;
		uint32 m_hiCreatureGuid;
		uint32 m_hiPetGuid;
		uint32 m_hiItemGuid;
		uint32 m_hiGoGuid;

		uint32 m_hiPetNumber;

	private:








		MapObjectGuids mMapObjectGuids;
		CreatureDataMap mCreatureDataMap;

		ItemPrototypeMap m_itemPrototypeList;
};

#define objmgr LeGACY::Singleton<ObjectMgr>::Instance()
#endif
