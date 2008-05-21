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
//#include "Spell.h"
#include "WorldSession.h"
#include "Pet.h"
#include "Util.h"

#include <string>
#include <vector>

enum PetSlots
{
	PET_SLOT_START       = 0,
	PET_SLOT_1           = 0,
	PET_SLOT_2           = 1,
	PET_SLOT_3           = 2,
	PET_SLOT_4           = 3,
	PET_SLOT_END         = 4,

	PET_SLOT_BANK_START  = 4,
	PET_SLOT_BANK_1      = 4,
	PET_SLOT_BANK_2      = 5,
	PET_SLOT_BANK_3      = 6,
	PET_SLOT_BANK_4      = 7,
	PET_SLOT_BANK_5      = 8,
	PET_SLOT_BANK_6      = 9,
	PET_SLOT_BANK_END    = 10
};

#define MAX_PET_SLOT  PET_SLOT_BANK_END // 0 = empty slot do not use

#define MAX_PLAYER_SPELL 254

class Creature;
class PlayerMenu;
class BattleSystem;

enum CharacterFields
{
	FD_GUID              = 0,
	FD_ACCOUNTID,
	FD_DATA,
	FD_CHARNAME,
	FD_MAPID,
	FD_POSX,
	FD_POSY,
	FD_ONLINE_STATUS,
};

class Item;
class WorldSession;

enum PlayerSlots
{
	// fist slot for item stored (in any way in player m_items data)
	PLAYER_SLOT_START            = 0,
	// last+1 slot for item stored (in any way in player m_items data)
	PLAYER_SLOT_END              = 145,
	PLAYER_SLOTS_COUNT            = (PLAYER_SLOT_END - PLAYER_SLOT_START)
};

enum EquipmentSlots
{
	EQUIPMENT_SLOT_START         = 0,
	EQUIPMENT_SLOT_HEAD          = 0,
	EQUIPMENT_SLOT_BODY          = 1,
	EQUIPMENT_SLOT_WEAPON        = 2,
	EQUIPMENT_SLOT_WRISTS        = 3,
	EQUIPMENT_SLOT_FEET          = 4,
	EQUIPMENT_SLOT_SPECIAL       = 5,
	EQUIPMENT_SLOT_END           = 6,
	EQUIPMENT_SLOT_PAD_1         = 7,  // not use, padding only
	EQUIPMENT_SLOT_PAD_2         = 8,  // not use, padding only
	EQUIPMENT_SLOT_PAD_3         = 9,  // not use, padding only
	EQUIPMENT_SLOT_PAD_4         = 10, // not use, padding only
};

enum InventorySlots
{
	INVENTORY_SLOT_ITEM_START    = 10,
	INVENTORY_SLOT_ITEM_1        = 10,
	INVENTORY_SLOT_ITEM_2        = 11,
	INVENTORY_SLOT_ITEM_3        = 12,
	INVENTORY_SLOT_ITEM_4        = 13,
	INVENTORY_SLOT_ITEM_5        = 14,
	INVENTORY_SLOT_ITEM_6        = 15,
	INVENTORY_SLOT_ITEM_7        = 16,
	INVENTORY_SLOT_ITEM_8        = 17,
	INVENTORY_SLOT_ITEM_9        = 18,
	INVENTORY_SLOT_ITEM_10       = 19,
	INVENTORY_SLOT_ITEM_11       = 20,
	INVENTORY_SLOT_ITEM_12       = 21,
	INVENTORY_SLOT_ITEM_13       = 22,
	INVENTORY_SLOT_ITEM_14       = 23,
	INVENTORY_SLOT_ITEM_15       = 24,
	INVENTORY_SLOT_ITEM_16       = 25,
	INVENTORY_SLOT_ITEM_17       = 26,
	INVENTORY_SLOT_ITEM_18       = 27,
	INVENTORY_SLOT_ITEM_19       = 28,
	INVENTORY_SLOT_ITEM_20       = 30,
	INVENTORY_SLOT_ITEM_21       = 31,
	INVENTORY_SLOT_ITEM_22       = 32,
	INVENTORY_SLOT_ITEM_23       = 33,
	INVENTORY_SLOT_ITEM_24       = 34,
	INVENTORY_SLOT_ITEM_END      = 35,
	INVENTORY_SLOT_ITEM_PAD_0    = 36, // not use, padding only
	INVENTORY_SLOT_ITEM_PAD_1    = 37, // not use, padding only
	INVENTORY_SLOT_ITEM_PAD_2    = 38, // not use, padding only
	INVENTORY_SLOT_ITEM_PAD_3    = 39, // not use, padding only
	INVENTORY_SLOT_ITEM_PAD_4    = 40, // not use, padding only

	///- Must have pet Si Ransel
	INVENTORY_SLOT_BAG_START     = 40,
	INVENTORY_SLOT_BAG_1         = 40,
	INVENTORY_SLOT_BAG_2         = 41,
	INVENTORY_SLOT_BAG_3         = 42,
	INVENTORY_SLOT_BAG_4         = 43,
	INVENTORY_SLOT_BAG_5         = 44,
	INVENTORY_SLOT_BAG_6         = 45,
	INVENTORY_SLOT_BAG_7         = 46,
	INVENTORY_SLOT_BAG_8         = 47,
	INVENTORY_SLOT_BAG_9         = 48,
	INVENTORY_SLOT_BAG_10        = 49,
	INVENTORY_SLOT_BAG_11        = 50,
	INVENTORY_SLOT_BAG_12        = 51,
	INVENTORY_SLOT_BAG_13        = 52,
	INVENTORY_SLOT_BAG_14        = 53,
	INVENTORY_SLOT_BAG_15        = 54,
	INVENTORY_SLOT_BAG_16        = 55,
	INVENTORY_SLOT_BAG_17        = 56,
	INVENTORY_SLOT_BAG_18        = 57,
	INVENTORY_SLOT_BAG_19        = 58,
	INVENTORY_SLOT_BAG_20        = 59,
	INVENTORY_SLOT_BAG_21        = 60,
	INVENTORY_SLOT_BAG_22        = 61,
	INVENTORY_SLOT_BAG_23        = 62,
	INVENTORY_SLOT_BAG_24        = 63,
	INVENTORY_SLOT_BAG_25        = 64,
	INVENTORY_SLOT_BAG_END       = 65,
	INVENTORY_SLOT_BAG_PAD_1     = 66,
	INVENTORY_SLOT_BAG_PAD_2     = 67,
	INVENTORY_SLOT_BAG_PAD_3     = 68,
	INVENTORY_SLOT_BAG_PAD_4     = 69,
	INVENTORY_SLOT_BAG_PAD_5     = 70,
};

enum BankSlots
{
	BANK_SLOT_ITEM_START         = 70,
	BANK_SLOT_ITEM_1             = 70,
	BANK_SLOT_ITEM_2             = 71,
	BANK_SLOT_ITEM_3             = 72,
	BANK_SLOT_ITEM_4             = 73,
	BANK_SLOT_ITEM_5             = 74,
	BANK_SLOT_ITEM_6             = 75,
	BANK_SLOT_ITEM_7             = 76,
	BANK_SLOT_ITEM_8             = 77,
	BANK_SLOT_ITEM_9             = 78,
	BANK_SLOT_ITEM_10            = 79,
	BANK_SLOT_ITEM_11            = 80,
	BANK_SLOT_ITEM_12            = 81,
	BANK_SLOT_ITEM_13            = 82,
	BANK_SLOT_ITEM_14            = 83,
	BANK_SLOT_ITEM_15            = 84,
	BANK_SLOT_ITEM_16            = 85,
	BANK_SLOT_ITEM_17            = 86,
	BANK_SLOT_ITEM_18            = 87,
	BANK_SLOT_ITEM_19            = 88,
	BANK_SLOT_ITEM_20            = 89,
	BANK_SLOT_ITEM_21            = 90,
	BANK_SLOT_ITEM_22            = 91,
	BANK_SLOT_ITEM_23            = 92,
	BANK_SLOT_ITEM_24            = 93,
	BANK_SLOT_ITEM_25            = 94,
	BANK_SLOT_ITEM_26            = 95,
	BANK_SLOT_ITEM_27            = 96,
	BANK_SLOT_ITEM_28            = 97,
	BANK_SLOT_ITEM_29            = 98,
	BANK_SLOT_ITEM_30            = 99,
	BANK_SLOT_ITEM_31            = 100,
	BANK_SLOT_ITEM_32            = 101,
	BANK_SLOT_ITEM_33            = 102,
	BANK_SLOT_ITEM_34            = 103,
	BANK_SLOT_ITEM_35            = 104,
	BANK_SLOT_ITEM_36            = 105,
	BANK_SLOT_ITEM_37            = 106,
	BANK_SLOT_ITEM_38            = 107,
	BANK_SLOT_ITEM_39            = 108,
	BANK_SLOT_ITEM_40            = 109,
	BANK_SLOT_ITEM_41            = 110,
	BANK_SLOT_ITEM_42            = 111,
	BANK_SLOT_ITEM_43            = 112,
	BANK_SLOT_ITEM_44            = 113,
	BANK_SLOT_ITEM_45            = 114,
	BANK_SLOT_ITEM_46            = 115,
	BANK_SLOT_ITEM_47            = 116,
	BANK_SLOT_ITEM_48            = 117,
	BANK_SLOT_ITEM_49            = 118,
	BANK_SLOT_ITEM_50            = 119,
	BANK_SLOT_ITEM_END           = 120,
///- TODO fix when know how may slot in bank
//
	BANK_SLOT_ARMY_START         = 120,
	BANK_SLOT_ARMY_1             = 120,
	BANK_SLOT_ARMY_2             = 121,
	BANK_SLOT_ARMY_3             = 122,
	BANK_SLOT_ARMY_4             = 123,
	BANK_SLOT_ARMY_5             = 124,
	BANK_SLOT_ARMY_6             = 125,
	BANK_SLOT_ARMY_7             = 126,
	BANK_SLOT_ARMY_8             = 127,
	BANK_SLOT_ARMY_9             = 128,
	BANK_SLOT_ARMY_10            = 129,
	BANK_SLOT_ARMY_11            = 130,
	BANK_SLOT_ARMY_12            = 131,
	BANK_SLOT_ARMY_13            = 132,
	BANK_SLOT_ARMY_14            = 133,
	BANK_SLOT_ARMY_15            = 134,
	BANK_SLOT_ARMY_16            = 135,
	BANK_SLOT_ARMY_17            = 136,
	BANK_SLOT_ARMY_18            = 137,
	BANK_SLOT_ARMY_19            = 138,
	BANK_SLOT_ARMY_20            = 139,
	BANK_SLOT_ARMY_21            = 140,
	BANK_SLOT_ARMY_22            = 141,
	BANK_SLOT_ARMY_23            = 142,
	BANK_SLOT_ARMY_24            = 143,
	BANK_SLOT_ARMY_25            = 144,
	BANK_SLOT_ARMY_END           = 145
};


enum TradeSlots
{
	TRADE_SLOT_COUNT             = 7,
	TRADE_SLOT_TRADED_COUNT      = 6,
	TRADE_SLOT_NONTRADED         = 6
};

// used at player loading query list preparing, and later result selection
enum PlayerLoginQueryIndex
{
	PLAYER_LOGIN_QUERY_LOADFROM                        = 0,
	PLAYER_LOGIN_QUERY_LOADPET                         = 1,
	PLAYER_LOGIN_QUERY_LOADINVENTORY                   = 2,
	PLAYER_LOGIN_QUERY_LOADSPELL                       = 3,
};

#define MAX_PLAYER_LOGIN_QUERY                           4

class LEGACY_DLL_SPEC Player : public Unit
{
	friend class WorldSession;
	friend void Item::AddToUpdateQueueOf(Player *player);
	friend void Item::RemoveFromUpdateQueueOf(Player *player);
	friend void Pet::AddToUpdateQueueOf(Player *player);
	friend void Pet::RemoveFromUpdateQueueOf(Player *player);
	public:
		explicit Player (WorldSession *session);
		~Player();

		void AddToWorld();
		void RemoveFromWorld();

		typedef std::set<uint64> ClientGUIDs;
		ClientGUIDs m_clientGUIDs;

		bool HaveAtClient(WorldObject const* u) { return u==this || m_clientGUIDs.find(u->GetGUID())!=m_clientGUIDs.end(); }

		uint32 GetAccountId() { return m_session->GetAccountId(); };

		bool Create ( uint32 guidlow, std::string new_name, WorldPacket &data, std::string &pass1, std::string &pass2 );
	
		void Update( uint32 time );

		WorldSession* GetSession() const { return m_session; }

		void BuildUpdateBlockStatusPacket(WorldPacket *data);

		void BuildUpdateBlockVisibilityPacket(WorldPacket *data);
		void BuildUpdateBlockVisibilityForOthersPacket(WorldPacket *data);
		void BuildUpdateBlockTeam(WorldPacket *data);

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
		/***                    PET SYSTEM                     ***/
		/*********************************************************/
		Pet* CreatePet( uint32 pet ) const;
		uint8 CanSummonPet( uint8 slot, uint8 &dest, Pet *pet, bool swap) const;
		uint8 FindSummonSlot( uint8 slot, bool swap ) const;
		Pet* SummonPet( uint8 pos, Pet* pet );
		void VisualizePet( uint8 pos, Pet* pet);
		void ReleasePet( uint8 slot );

		/*********************************************************/
		/***                 STORAGE SYSTEM                    ***/
		/*********************************************************/

		uint8 FindEquipSlot( ItemPrototype const* proto, uint32 slot, bool swap ) const;
		uint8 FindPetEquipSlot( Pet* pet, ItemPrototype const* proto, uint32 slot, bool swap ) const;
		Item* CreateItem( uint32 item, uint32 count ) const;
		uint32 GetItemCount( uint32 item, Item* eItem = NULL ) const;
		uint32 GetBankItemCount( uint32 item, Item* eItem = NULL ) const;
		Item* GetItemByGuid( uint64 guid ) const;
		Item* GetItemByPos( uint8 slot ) const;
		Item* GetPetItemByPos( Pet* pet, uint8 slot ) const;

		static bool IsInventoryPos( uint8 slot );
		static bool IsEquipmentPos( uint8 slot );
		static bool IsBagPos( uint8 slot );
		static bool IsBankPos( uint8 slot );

		bool HasItemCount( uint32 item, uint32 count ) const;


		Item* GetItemOrItemWithGemEquipped( uint32 item ) const;
		uint8 CanTakeMoreSimilarItems(Item* pItem) const;
		uint8 CanStoreNewItem( uint8 slot, uint8 &dest, uint32 item, uint32 count, bool swap ) const;
		uint8 CanStoreItem( uint8 slot, uint8 &dest, Item *pItem, bool swap ) const;
		uint8 CanStoreItems( Item **pItem, int count) const;
		uint8 CanEquipNewItem( uint8 slot, uint8 &dest, uint32 item, uint32 count, bool swap ) const;
		uint8 CanEquipItem( uint8 slot, uint8 &dest, Item *pItem, bool swap, bool not_loading = true ) const;
		uint8 CanPetEquipItem( Pet* pet, uint8 slot, uint8 &dest, Item *pItem, bool swap, bool not_loading = true ) const;
		uint8 CanUseItemFor( Unit* unit, Item* item ) const;
		uint8 CanUnequipItems( uint32 item, uint32 count ) const;
		uint8 CanUnequipItem( uint8 src, bool swap ) const;
		uint8 CanPetUnequipItem( Pet* pet, uint8 src, bool swap ) const;
		uint8 CanBankItem( uint8 slot, uint8 &dest, Item *pItem, bool swap, bool not_loading = true ) const;
		uint8 CanUseItem( Item *pItem, bool not_loading = true ) const;
		bool CanUseItem( ItemPrototype const *pItem );
		Item* StoreNewItem( uint8 pos, uint32 item, uint32 count, bool update, int32 randomPropertyId = 0 );
		Item* StoreItem( uint8 pos, Item *pItem, bool update );
		Item* EquipNewItem( uint8 pos, uint32 item, uint32 count, bool update );
		Item* EquipItem( uint8 pos, Item *pItem, bool update );
		Item* PetEquipItem( Pet* pet, uint8 pos, Item *pItem, bool update );

		bool  AddNewInventoryItem(uint32 entry, uint32 count);
		bool  ConsumeInventoryItemFor(uint8 target, uint8 invslot, uint8 amount);

		void QuickEquipItem( uint8 pos, Item *pItem);
		void VisualizeItem( uint8 pos, Item *pItem);

		void QuickPetEquipItem( Pet* pet, uint8 pos, Item *pItem);
		void VisualizePetItem( Pet* pet, uint8 pos, Item *pItem);

		Item* BankItem( uint8 pos, Item *pItem, bool update );
		void RemoveItem( uint8 slot );
		void PetRemoveItem( Pet* pet, uint8 slot );
		void DestroyItem( uint8 slot );

		void DestroyItem( uint8 slot, bool update );
		void DestroyItemCount( uint32 item, uint32 count, bool update, bool unequip_check = false);
		void DestroyItemCount( Item* item, uint32& count, bool update);


		void SplitItem( uint8 src, uint8 dst );
		void SwapItem( uint8 src, uint8 dst );



		void BuyItemFromVendor(uint64 vendorguid, uint32 item, uint8 count, uint8 slot);

		void DumpPlayer(const char* section="all");
		/*********************************************************/
		/***                  LOAD SYSTEM                      ***/
		/*********************************************************/
		bool LoadFromDB(uint32 accountId, SqlQueryHolder *holder);


		/*********************************************************/
		/***                  SAVE SYSTEM                      ***/
		/*********************************************************/
		void SaveToDB();

		void SendMessageToSet(WorldPacket *data, bool self); // overwrite Object::SendMessageToSet
		void SendMessageToAll(WorldPacket *data, bool self);

		void SendDelayResponse(const uint32);







		void _ApplyAllItemMods();
		void _ApplyItemModsFor(Unit* unit, Item* item, bool apply);
		void _ApplyItemSetModsFor(Unit* unit, bool apply);

		/*********************************************************/
		/***                  GROUP SYSTEM                     ***/
		/*********************************************************/
		bool CanJoinTeam();
		void JoinTeam(Player* member);
		void DismissTeam();
		void LeaveTeam(Player* member);
		bool isTeamLeader();
		bool isJoinedTeam();
		void SetLeader(uint32 guid) { m_leaderGuid = guid; }
		void SetSubleader(uint32 acc_id);
		void UnsetSubleader(uint32 acc_id);
		void UpdateTeamSub();
		//void UpdateTeamToSet();


		/********************************************************/
		/***                BATTLE SYSTEM                     ***/
		/********************************************************/
		//bool CanJoinBattle();
		bool isBattleInProgress();
		void LeaveBattle();
		void Engage(Creature* enemy);
		void Engage(Player* player);
		Player* GetBattleMaster();
		void SetBattleMaster(Player* master) { i_battleMaster = master; }
		
		Pet* GetPet(uint8 slot) const { return m_pets[slot]; }
		Pet* GetPetByGuid(uint32 pet_guid) const;
		Pet* GetBattlePet() const { return m_battlePet; }
		Pet* GetPetByModelId(uint16 modelid) const;
		uint8 GetPetSlot(Pet* pet) const;
		void SetBattlePet(Pet* pet);

		void SetDontMove(bool dontMove) { m_dontMove = dontMove; }
		bool GetDontMove() const { return m_dontMove; }

		void SetTeleportTo(uint16 target) { m_teleportMapId = target; }
		uint16 GetTeleportTo() { return m_teleportMapId; }

		uint32 GetTeam() const { return 0;/*m_team;*/} // used by grid notifier

		uint32 GetTeamGuid(uint8 index) const;    // used by battle system

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
		void UpdatePlayerLevel();
		void _updatePlayer(uint8 flagStatus, uint8 modifier, uint32 value, uint16 spell_entry=0);

		void UpdatePet();
		void UpdatePet(uint8 slot);
		void UpdatePetBattle();
		void UpdatePetLevel(Pet* pet);
		void _updatePet(uint8 slot, uint8 flagStatus, uint8 modifier, uint32 value, uint16 spell_entry=0);

		void UpdateInventory();
		void UpdateInventoryForItem(uint16 modelid, uint8 count);
		void UpdateCurrentEquip();
		void UpdateCurrentGold();
		void UpdateGold(int32 value);
		void UpdatePetCarried();
		void UpdateBattlePet();




		PlayerMenu* PlayerTalkClass;
		BattleSystem* PlayerBattleClass;


		void TalkedToCreature( uint32 entry, uint64 guid);
		uint64 GetTalkedCreatureGuid() const { return m_talkedCreatureGuid; }
		void IncTalkedSequence() { m_talkedSequence++; };
		uint32 GetTalkedSequence() { return m_talkedSequence; }



		template<class T>
			void UpdateVisibilityOf(T* target, UpdateData& data, UpdateDataMapType& data_updates, std::set<WorldObject*>& visibleNow);

		bool HasSpell(uint32 spell) const;

		/**********************************************************/
		/***                   MISC SYSTEM                      ***/
		/**********************************************************/
		void SetOrientation(uint8 orientation) { m_orientation = orientation; }
		uint8 GetOrientation() { return m_orientation; }
		void SetExpression(uint8 type, uint8 code)
		{
			m_exprType = type;
			m_exprCode = code;
		}
		void ResetExpression() { m_exprType = 0; m_exprCode = 0; }
		void BuildUpdateBlockExpression(WorldPacket* packet);

		GridReference<Player> &GetGridRef() { return m_gridRef; }
		void ResetSpells();
	protected:

		/**********************************************************/
		/***                    LOAD SYSTEM                     ***/
		/**********************************************************/
		void _LoadPets(QueryResult *result);
		void _LoadInventory(QueryResult *result);
		void _LoadSpells(QueryResult *result);

		/**********************************************************/
		/***                    SAVE SYSTEM                     ***/
		/**********************************************************/
		void _SavePets();
		void _SaveInventory();
		void _SaveSpells();

		bool _removeSpell(uint16 entry);

		uint16 m_lastPositionX;
		uint16 m_lastPositionY;

		WorldSession *m_session;

		bool   m_dontMove;
		uint16 m_teleportMapId;

		//uint32  m_team;  // used by grid notifier
		typedef std::list<Player*> TeamList;
		TeamList m_team;
		uint32 m_leaderGuid;
		uint32 m_subleaderGuid;

		uint32 m_nextSave;

		std::vector<Item*> m_itemUpdateQueue;
		bool m_itemUpdateQueueBlocked;

		std::vector<Pet*>  m_petUpdateQueue;
		bool m_petUpdateQueueBlocked;

		uint32 m_GMFlags;

		Item*  m_items[PLAYER_SLOTS_COUNT];
		Pet*   m_pets[MAX_PET_SLOT];
		Pet*   m_battlePet;
/*
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

		uint16 m_hp, m_sp;

		uint16 m_stat_int, m_stat_atk, m_stat_def, m_stat_agi, m_stat_hpx, m_stat_spx;
		uint8  m_level;
		uint32 m_rank;
		uint32 m_xp_gain;
		uint16 m_skill;
		uint16 m_stat;

		uint16 m_hp_max;
		uint16 m_sp_max;
*/
		///- TODO: Fix
		//   We will need this to be signed
		//   modifiers can have negative value
//		uint32 m_atk_mod, m_def_mod, m_int_mod, m_agi_mod, m_hpx_mod, m_spx_mod;

//		uint32 m_gold_hand;
//		uint32 m_gold_bank;
/*
		uint16 m_unk1;
		uint16 m_unk2;
		uint16 m_unk3;
		uint16 m_unk4;
		uint16 m_unk5;
*/
	private:

		GridReference<Player> m_gridRef;

		uint64 m_talkedCreatureGuid;

		uint32 m_talkedSequence;

		//Spell* m_spells[MAX_PLAYER_SPELL];
//		SpellMap m_spells;

		Player* i_battleMaster;

		uint8 m_exprType;
		uint8 m_exprCode;
		uint8 m_orientation;
};

#endif
