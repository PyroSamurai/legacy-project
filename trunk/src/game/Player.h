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

#ifndef __LEGACY_PLAYER_H
#define __LEGACY_PLAYER_H

#include "Common.h"
#include "ItemPrototype.h"
#include "Unit.h"
#include "Item.h"

#include "Database/DatabaseEnv.h"
#include "NPCHandler.h"
#include "Bag.h"
#include "WorldSession.h"
#include "Pet.h"
#include "Util.h"

#include <string>
#include <vector>

#define MAX_PET_SLOT 4

class Creature;
class PlayerMenu;
class BattleSystem;

// used at player loading query list preparing, and later result selection
enum PlayerLoginQueryIndex
{
	PLAYER_LOGIN_QUERY_LOADFROM                        = 0,
//	PLAYER_LOGIN_QUERY_LOADGROUP                       = 1,
//	PLAYER_LOGIN_QUERY_LOADINVENTORY                   = 2,
};

enum CharacterFields
{
	FD_GUID = 0,
	FD_ACCOUNTID, 
	FD_PSSWD,
	FD_GENDER,
	FD_ELEMENT,
	FD_FACE,
	FD_HAIR,
	FD_CHARNAME,
	FD_REBORN,
	FD_LEVEL,
	FD_RANK,
	FD_MAPID,
	FD_POSX,
	FD_POSY,
	FD_HP,
	FD_SP,
	FD_XP_GAIN,
	FD_XP_TNL,

	FD_ST_INT,
	FD_ST_ATK,
	FD_ST_DEF,
	FD_ST_HPX,
	FD_ST_SPX,
	FD_ST_AGI,

	FD_SKILL_GAIN,
	FD_STAT_GAIN,

	FD_EQ_HEAD,
	FD_EQ_BODY,
	FD_EQ_WRIST,
	FD_EQ_WEAPON,
	FD_EQ_SHOE,
	FD_EQ_SPECIAL,

	FD_HAIR_COLOR_R,
	FD_HAIR_COLOR_G,
	FD_HAIR_COLOR_B,
	FD_SKIN_COLOR_R,
	FD_SKIN_COLOR_G,
	FD_SKIN_COLOR_B,
	FD_SHIRT_COLOR,
	FD_MISC_COLOR,

	FD_ONLINE_STATUS,

	FD_GOLD_IN_HAND,
	FD_GOLD_IN_BANK,

	FD_SECURITY_LEVEL,

	FD_UNK1,
	FD_UNK2,
	FD_UNK3,
	FD_UNK4,
	FD_UNK5
};

enum EquipmentSlots
{
	EQUIPMENT_SLOT_START                            = 0,
	EQUIPMENT_SLOT_HEAD                             = 0,
	EQUIPMENT_SLOT_BODY                             = 1,
	EQUIPMENT_SLOT_END                              = 10
};

enum InventorySlots
{
	INVENTORY_SLOT_BAG_0                            = 255,
	INVENTORY_SLOT_BAG_START                        = 19,
	INVENTORY_SLOT_BAG_1                            = 19,
	INVENTORY_SLOT_BAG_END                          = 23,

	INVENTORY_SLOT_ITEM_START                       = 23,
	INVENTORY_SLOT_ITEM_1                           = 23,

	INVENTORY_SLOT_ITEM_END                         = 39
};

enum BankSlots
{
	BANK_SLOT_ITEM_START                            = 39,
	BANK_SLOT_ITEM_1                                = 39,
	
	BANK_SLOT_ITEM_END                              = 67
};

#define MAX_PLAYER_LOGIN_QUERY                           1

class WorldSession;

class LEGACY_DLL_SPEC Player : public Unit
{
	friend class WorldSession;

	public:
		explicit Player (WorldSession *session);
		~Player();

		void AddToWorld();
		void RemoveFromWorld();

		typedef std::set<uint64> ClientGUIDs;
		ClientGUIDs m_clientGUIDs;

		bool HaveAtClient(WorldObject const* u) { return u==this || m_clientGUIDs.find(u->GetGUID())!=m_clientGUIDs.end(); }

		uint32 GetAccountId() { return m_session->GetAccountId(); };
		Pet* GetPet(uint8 slot)
		{
			return m_petSlot[slot];
		}

		bool Create ( uint32 guidlow, WorldPacket &data );
	
		void Update( uint32 time );

		WorldSession* GetSession() const { return m_session; }

		void BuildUpdateBlockStatusPacket(WorldPacket *data);

		void BuildUpdateBlockVisibilityPacket(WorldPacket *data);
		void BuildUpdateBlockVisibilityForOthersPacket(WorldPacket *data);

		void BuildUpdateBlockTeleportPacket(WorldPacket *data);

		void TeleportTo(uint16 mapid, uint16 pos_x, uint16 pos_y);
		void SendInitialPacketsBeforeAddToMap();
		void SendInitialPacketsAfterAddToMap();
		void SendMotd();
		void SendVoucherInfo();
		void SendMapChanged();

		void AllowPlayerToMove();
		void EndOfRequest();

		void Send0504();
		void Send0F0A();
		void Send0602();
		void Send1408();

		void UpdateMap2Npc();
		void SendUnknownImportant();

		/*********************************************************/
		/***                  LOAD SYSTEM                      ***/
		/*********************************************************/
		bool LoadFromDB(uint32 accountId, SqlQueryHolder *holder);
		bool LoadPet();


		/*********************************************************/
		/***                  SAVE SYSTEM                      ***/
		/*********************************************************/
		void SaveToDB();

		void SendMessageToSet(WorldPacket *data, bool self); // overwrite Object::SendMessageToSet

		void SendDelayResponse(const uint32);

		/*********************************************************/
		/***                  GROUP SYSTEM                     ***/
		/*********************************************************/


		void SetDontMove(bool dontMove) { m_dontMove = dontMove; }
		bool GetDontMove() const { return m_dontMove; }

		void SetTeleportTo(uint16 target) { m_teleportMapId = target; }
		uint16 GetTeleportTo() { return m_teleportMapId; }

		uint32 GetTeam() const { return m_team; }

		uint32 GetTeamGuid(uint8 index) const { return 0; }

		/*********************************************************/
		/***                VARIOUS SYSTEM                     ***/
		/*********************************************************/
		///- Tools for mapping door position
		void SetLastPosition(uint16 x, uint16 y)
		{
			m_lastPositionX = x;
			m_lastPositionY = y;
		}
		uint16 GetLastPositionX() { return m_lastPositionX; }
		uint16 GetLastPositionY() { return m_lastPositionY; }
		void UpdateVisibilityOf(WorldObject* target);
		void UpdateRelocationToSet();
		void UpdatePlayer();
		void _updatePlayer(uint8 flagStatus, uint8 modifier, uint16 value);
		void UpdatePet();
		void UpdatePet(uint8 slot);
		void _updatePet(uint8 slot, uint8 flagStatus, uint8 modifier, uint32 value);

		void UpdateCurrentEquipt();
		void UpdateCurrentGold();
		void UpdatePetCarried();




		PlayerMenu* PlayerTalkClass;
		BattleSystem* PlayerBattleClass;


		void TalkedToCreature( uint32 entry, uint64 guid);

		template<class T>
			void UpdateVisibilityOf(T* target, UpdateData& data, UpdateDataMapType& data_updates, std::set<WorldObject*>& visibleNow);

		bool HasSpell(uint32 spell) const;

		GridReference<Player> &GetGridRef() { return m_gridRef; }
	protected:
		uint16 m_lastPositionX;
		uint16 m_lastPositionY;

		WorldSession *m_session;

		bool   m_dontMove;
		uint16 m_teleportMapId;

		uint32 m_team;
		uint32 m_nextSave;

		uint32 m_GMFlags;

		uint8  m_reborn;
		uint8  m_element;
		uint8  m_gender;
		uint8  m_face;
		uint8  m_hair;
		uint8  m_hair_color_R;
		uint8  m_hair_color_G;
		uint8  m_hair_color_B;
		uint8  m_skin_color_R;
		uint8  m_skin_color_G;
		uint8  m_skin_color_B;
		uint8  m_shirt_color;
		uint8  m_misc_color;

		uint16 m_eq_head, m_eq_body, m_eq_wrist, m_eq_weapon, m_eq_shoe, m_eq_accsr;

		uint16 m_hp, m_sp;

		uint16 m_stat_int, m_stat_atk, m_stat_def, m_stat_agi, m_stat_hpx, m_stat_spx;
		uint8  m_level;
		uint32 m_rank;
		uint32 m_xp_gain;
		uint16 m_skill;
		uint16 m_stat;

		uint16 m_hp_max;
		uint16 m_sp_max;

		///- TODO: Fix
		//   We will need this to be signed
		//   modifiers can have negative value
		uint32 m_atk_mod, m_def_mod, m_int_mod, m_agi_mod, m_hpx_mod, m_spx_mod;

		uint32 m_gold_hand;
		uint32 m_gold_bank;

		uint16 m_unk1;
		uint16 m_unk2;
		uint16 m_unk3;
		uint16 m_unk4;
		uint16 m_unk5;

	private:
		GridReference<Player> m_gridRef;
		Pet* m_petSlot[MAX_PET_SLOT];
};

#endif
