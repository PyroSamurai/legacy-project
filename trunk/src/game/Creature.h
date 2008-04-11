/*
 * Copyright (C) 2008-2008 LeGACY <http://code.google.com/p/legacy-project/>
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

#ifndef __LEGACY_CREATURE_H
#define __LEGACY_CREATURE_H

#include "Common.h"
#include "GossipDef.h"
#include "Unit.h"
#include "ItemPrototype.h"
#include "Database/DatabaseEnv.h"
#include "Cell.h"

class WorldSession;

class CreatureAI;
class Player;
class WorldSession;
/*
enum Gossip_Option
{
	GOSSIP_OPTION_NONE         = 0,  // UNIT_NPC_FLAG_NONE = 0
	GOSSIP_OPTION_GOSSIP       = 1,
	GOSSIP_OPTION_QUESTGIVER   = 2,
	GOSSIP_OPTION_VENDOR       = 3,
	GOSSIP_OPTION_INNKEEPER    = 4,
	GOSSIP_OPTION_BANKER       = 5
};

struct GossipOption
{
	uint32 Id;
	uint32 GossipId;
	uint32 NpcFlag;
	uint32 Icon;
	uint32 Action;
	std::string Option;
};
*/
struct CreatureItem
{
	CreatureItem(uint32 _item, uint32 _maxcount, uint32 _incrtime)
		: id(_item), count(_maxcount), maxcount(_maxcount), incrtime(_incrtime), lastincr((uint32)time(NULL)) {}

	uint32 id;
	uint32 count;
	uint32 maxcount;
	uint32 incrtime;
	uint32 lastincr;
};

// from `creature_template` table
struct CreatureInfo
{
	uint32 Entry;
	uint32 modelid;
	char*  Name;
	uint32 hp;
	uint32 sp;
	uint32 stat_int;
	uint32 stat_atk;
	uint32 stat_def;
	uint32 stat_hpx;
	uint32 stat_spx;
	uint32 stat_agi;
	uint32 level;
	uint32 element;
	uint32 skill1;
	uint32 skill2;
	uint32 skill3;
	uint32 skill4;
	uint32 skill5;
	uint32 drop1;
	uint32 drop2;
	uint32 drop3;
	uint32 drop4;
	uint32 drop5;
	uint32 drop6;
	uint32 drop7;
	uint32 drop8;
	uint32 drop9;
	uint32 drop10;
	uint32 npcflag;
	char const* AIName;
	uint32 MovementType;
	char*  ScriptName;
};

// from `creature` table
struct CreatureData
{
	uint32 id;           // entry in creature_template
	uint16 mapid;
	uint8  map_npcid;
	uint16 posX;
	uint16 posY;

	uint32 spawntimesecs;

	uint16 spawn_posX;
	uint16 spawn_posY;

	uint16 hp;
	uint16 sp;

	uint8  deathState;
	uint8  movementType;

	uint32 team00;
	uint32 team01;
	uint32 team03;
	uint32 team04;

	uint32 team10;
	uint32 team11;
	uint32 team12;
	uint32 team13;
	uint32 team14;
};

typedef std::list<GossipOption> GossipOptionList;

#define MAP_NPCID_MULTIPLIER 100

class LEGACY_DLL_SPEC Creature : public Unit
{
	CreatureAI *i_AI;

	public:

		explicit Creature( WorldObject *instantiator );
		explicit Creature( WorldObject *instantiator, bool puppet );
		virtual ~Creature();

		void AddToWorld();
		void RemoveFromWorld();

		bool Create(uint32 guidlow, uint16 mapid, uint16 x, uint16 y, uint32 Entry, uint32 team, const CreatureData *data = NULL);
		bool CreateFromProto(uint32 guidlow, uint32 Entry, uint32 team, const CreatureData *data = NULL);
		void SelectLevel(const CreatureInfo *cinfo);


		void Update( uint32 time );

		uint8 GetMapNpcId();// { return (GetGUIDLow()-(GetMapId()*MAP_NPCID_MULTIPLIER)); }
		uint16 GetModelId() { return GetUInt16Value(UNIT_FIELD_DISPLAYID); }
		void GetRespawnCoord(uint16 &x, uint16 &y) const { x = respawn_cord[0]; y = respawn_cord[1]; }

		bool isPet() const { return m_isPet; }
		void SetRespawnCoord(uint16 x, uint16 y) { respawn_cord[0] = x; respawn_cord[1] = y; }

		void AI_SendMoveToPacket(uint16 x, uint16 y, uint32 time);
		CreatureAI* AI() { return i_AI; }

		bool HasSpell(uint16 spellID) const;

		bool AIM_Initialize();

		bool LoadFromDB(uint32 guid, uint32 InstanceId=0);











		MovementGeneratorType GetDefaultMovementType() const { return m_defaultMovementType; }
		void SetDefaultMovementType(MovementGeneratorType mgt) { m_defaultMovementType = mgt; }

		

		// for use only in LoadHelper, Map::Add Map::CreatureCellRelocation
		Cell const& GetCurrentCell() const { return m_currentCell; }
		void SetCurrentCell(Cell const& cell) { m_currentCell = cell; }

		/*************************************************************/
		/***                   VENDOR SYSTEM                       ***/
		/*************************************************************/
		void LoadGoods();     // must be called before access to vendor items, lazy loading at first call
		void ReloadGoods() { m_itemsLoaded = false; LoadGoods(); }

		CreatureItem* GetItem(uint32 slot)
		{
			if(slot>=m_vendor_items.size()) return NULL;
			return &m_vendor_items[slot];
		}
		uint8 GetItemCount() const { return m_vendor_items.size(); }
		void AddItem(uint32 item, uint32 maxcount, uint32 ptime)
		{
			m_vendor_items.push_back(CreatureItem(item,maxcount,ptime));
		}
		bool RemoveItem(uint32 item_id)
		{
			for(CreatureItems::iterator i = m_vendor_items.begin(); i != m_vendor_items.end(); ++i)
			{
				if(i->id==item_id)
				{
					m_vendor_items.erase(i);
					return true;
				}
			}
			return false;
		}
		CreatureItem* FindItem(uint32 item_id)
		{
			for(CreatureItems::iterator i = m_vendor_items.begin(); i != m_vendor_items.end(); ++i)
				if(i->id==item_id)
					return &*i;
			return NULL;
		}


		/***************************************************************/
		/***                     TRAINER SYSTEM                      ***/
		/***************************************************************/

		CreatureInfo const *GetCreatureInfo() const;


		uint16 GetNpcTextId();
		GossipItem GetNpcGossip(uint32 sequence, uint32 action); 



		void prepareGossipMenu( Player *pPlayer, uint32 gossipid );
		void sendPreparedGossip( Player* player );
		void OnGossipSelect(Player* player, uint32 sequence, uint32 option);

		uint32 GetGossipCount( uint32 gossipid );



		void Say(const char* text, const uint32 language, const uint64 TargetGuid) { MonsterSay(text, language, TargetGuid); }

		//uint32 m_spells[CREATURE_MAX_SPELLS];


		GridReference<Creature> &GetGridRef() { return m_gridRef; }

	protected:
		// vendor items
		typedef std::vector<CreatureItem> CreatureItems;
		CreatureItems m_vendor_items;
		bool m_itemsLoaded;            // vendor item loading state



		/// Timers
		time_t m_respawnTime;          // (secs) time of next respawn
		uint32 m_respawnDelay;         // (secs) delay between respawning

		bool m_gossipOptionLoaded;
		GossipOptionList m_goptions;
		uint16 m_NPCTextId;
		uint16 m_NPCTextId_seq[10];

		uint16 respawn_cord[3];

		bool   m_isPet;

		MovementGeneratorType m_defaultMovementType;
		Cell   m_currentCell;          // store current cell where creature listed

		uint32 m_DBTableGuid;

		bool m_AI_locked;

	private:
		GridReference<Creature> m_gridRef;
		bool m_puppet;

};
#endif

