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
#include "Unit.h"
#include "ItemPrototype.h"
#include "Database/DatabaseEnv.h"
#include "Cell.h"

class WorldSession;

class CreatureAI;
class Player;
class WorldSession;

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

struct CreatureInfo
{
	uint32 Entry;
	char*  Name;
	char*  ScriptName;
};

// from `creature_template` table
/*
struct CreatureInfo
{
	uint32  Entry;
	uint32  DisplayID_A;
	uint32  DisplayID_A2;
	uint32  DisplayID_H;
	uint32  DisplayID_H2;
	char*   Name;
	char*   SubName;
	uint32  minlevel;
	uint32  maxlevel;
	uint32  minhealth;
	uint32  maxhealth;
	uint32  minmana;
	uint32  maxmana;
	uint32  armor;
	uint32  faction_A;
	uint32  faction_H;
	uint32  npcflag;
	float   speed;
	uint32  rank;
	float   mindmg;
	float   maxdmg;
	uint32  dmgschool;
	uint32  attackpower;
	uint32  baseattacktime;
	uint32  rangeattacktime;
	uint32  Flags;
	uint32  dynamicflags;
	uint32  family;
	uint32  trainer_type;
	uint32  trainer_spell;
	uint32  classNum;
	uint32  race;
	float   minrangedmg;
	float   maxrangedmg;
	uint32  rangedattackpower;
	uint32  type;
	bool    civilian;
	uint32  flag1;
	uint32  lootid;
	uint32  pickpocketLootId;
	uint32  SkinLootId;
	uint32  resistance1;
	uint32  resistance2;
	uint32  resistance3;
	uint32  resistance4;
	uint32  resistance5;
	uint32  resistance6;
	uint32  spell1;
	uint32  spell2;
	uint32  spell3;
	uint32  spell4;
	uint32  mingold;
	uint32  maxgold;
	char const* AIName;
	uint32  MovementType;
	uint32  InhabitType;
	bool    RacialLeader;
	bool    RegenHealth;
	uint32  equipmentId;
	uint32  MechanicImmuneMask;
	char const* ScriptName;
};
*/


// from `creature` table
struct CreatureData
{
	uint32 id;
	uint16 mapid;
	uint8  map_npcid;
	uint16 posX;
	uint16 posY;

	uint16 spawn_posX;
	uint16 spawn_posY;

	uint16 hp;
	uint16 sp;
};

typedef std::list<GossipOption> GossipOptionList;

class LEGACY_DLL_SPEC Creature : public Unit
{
	CreatureAI *i_AI;

	public:

		explicit Creature( WorldObject *instantiator );
		virtual ~Creature();

		void AddToWorld();
		void RemoveFromWorld();

		bool Create(uint32 guidlow, uint16 mapid, uint16 x, uint16 y, uint32 Entry, uint32 team, const CreatureData *data = NULL);
		bool CreateFromProto(uint32 guidlow, uint32 Entry, uint32 team, const CreatureData *data = NULL);
		void SelectLevel(const CreatureInfo *cinfo);


		void Update( uint32 time );

		void GetRespawnCoord(uint16 &x, uint16 &y) const { x = respawn_cord[0]; y = respawn_cord[1]; }

		bool isPet() const { return m_isPet; }
		void SetRespawnCoord(uint16 x, uint16 y) { respawn_cord[0] = x; respawn_cord[1] = y; }

		void AI_SendMoveToPacket(uint16 x, uint16 y, uint32 time);
		CreatureAI* AI() { return i_AI; }

		bool HasSpell(uint16 spellID) const;

		bool AIM_Initialize();

		bool LoadFromDB(uint32 guid, uint32 InstanceId);

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


		uint32 GetGossipTextId(uint32 action, uint32 zoneid);
		uint32 GetNpcTextId();



		void prepareGossipMenu( Player *pPlayer, uint32 gossipid );
		void sendPreparedGossip( Player* player );


		uint32 GetGossipCount( uint32 gossipid );



		void Say(const char* text, const uint32 language, const uint64 TargetGuid) { MonsterSay(text, language, TargetGuid); }

		uint32 m_spells[CREATURE_MAX_SPELLS];


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

		uint16 respawn_cord[3];

		bool   m_isPet;

		MovementGeneratorType m_defaultMovementType;
		Cell   m_currentCell;          // store current cell where creature listed

		uint32 m_DBTableGuid;

	private:
		GridReference<Creature> m_gridRef;

};
#endif

