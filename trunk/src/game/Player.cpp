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
#include "Log.h"
#include "Opcodes.h"
#include "ObjectMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"

#include "GossipDef.h"
#include "BattleSystem.h"

#include "MapManager.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ObjectMgr.h"
#include "ObjectAccessor.h"
#include "Util.h"
#include "Database/DatabaseImpl.h"

#include <cmath>

Player::Player (WorldSession *session): Unit( 0 )
{
	m_session = session;

	m_objectType |= TYPE_PLAYER;
	m_objectTypeId = TYPEID_PLAYER;

	m_valuesCount = PLAYER_END;

	m_GMFlags = 0;

	memset(m_items, 0, sizeof(Item*)*PLAYER_SLOTS_COUNT);

	memset(m_pets, 0, sizeof(Pet*)*MAX_PET_SLOT);

	m_battlePet = NULL;

	m_nextSave = sWorld.getConfig(CONFIG_INTERVAL_SAVE);

	// randomize first save time in range [CONFIG_INTERVAL_SAVE] around
	// [CONFIG_INTERVAL_SAVE]
	// this must help in case next save after mass player load after server
	// startup
	m_nextSave = rand32(m_nextSave/2,m_nextSave*3/2);

	m_dontMove = false;

	///////////////////Battle System///////////////////
	PlayerTalkClass = new PlayerMenu( GetSession() );
	//PlayerBattleClass = new BattleSystem( GetSession() );
	PlayerBattleClass = NULL;
	i_battleMaster = NULL;
	///////////////////Battle System///////////////////


	//////////////////Gossip System////////////////////
	m_talkedSequence = 0;
	m_talkedCreatureGuid = 0;
	//////////////////Gossip System////////////////////

	//////////////////Pet System///////////////////////
	m_petUpdateQueueBlocked = false;
	//////////////////Pet System///////////////////////

	//////////////////Inventory System/////////////////
	m_itemUpdateQueueBlocked = false;
	//////////////////Inventory System/////////////////

	m_leaderGuid = 0;
}

Player::~Player ()
{
	///- if battle in progress, if in group assign battle engine to others
	if( isBattleInProgress() )
	{
		sLog.outDebug("PLAYER: '%s' is battle master, find new master", GetName());
		if( !PlayerBattleClass->FindNewMaster() )
		{
			delete PlayerBattleClass;
			PlayerBattleClass = NULL;
		}

		/*
		if( isTeamLeader() && !m_team.empty() )
		{
			Player* pteam = m_team.front();

			pteam->PlayerBattleClass = PlayerBattleClass;
		}
		*/
	}

	///- Leave from battle if any
	LeaveBattle();

	///- Leave from team if joined
	if(m_leaderGuid)
	{
		Player* leader = ObjectAccessor::FindPlayer(MAKE_GUID(m_leaderGuid, HIGHGUID_PLAYER));
		if( leader )
		{
			leader->LeaveTeam(this);
		}
	}

	///- Dismiss Team if team leader
	//for(TeamList::const_iterator itr = m_team.begin(); itr != m_team.end(); ++itr)
	while( !m_team.empty() )
	{
		Player* pteam = m_team.front();
		sLog.outDebug("PLAYER: '%s' as leader, dismiss team for '%s'", GetName(), pteam->GetName());
		LeaveTeam((pteam));
	}

	if( m_session && m_session->PlayerLogout() )
	{
		sLog.outDebug( "WORLD: Player '%s' is logged out", GetName());
		WorldPacket data;
		data.Initialize( 0x01 );
		data << (uint8 ) 0x01;
		data << GetAccountId();
		SendMessageToSet(&data, false);
	}
}

bool Player::Create( uint32 accountId, std::string new_name, WorldPacket& data, std::string &pass1, std::string &pass2)
{
	uint8  gender = 0;
	uint8  face   = 0;
	uint8  hair   = 0;
	uint8  reborn = 0;

	uint8  skin_color_R = 0;
	uint8  skin_color_G = 0;
	uint8  skin_color_B = 0;
	uint8  hair_color_R = 0;
	uint8  hair_color_G = 0;
	uint8  hair_color_B = 0;
	uint8  shirt_color  = 0;
	uint8  misc_color   = 0;
		
	uint8  element  = 0;
	uint8  stat_int = 0;
	uint8  stat_atk = 0;
	uint8  stat_def = 0;
	uint8  stat_hpx = 0;
	uint8  stat_spx = 0;
	uint8  stat_agi = 0;

	uint8       lenPassword1 = 0;
	std::string password1    = "";
	uint8       lenPassword2 = 0;
	std::string password2    = "";

	uint8 tmp_password1[100];
	uint8 tmp_password2[100];

	Object::_Create(accountId, HIGHGUID_PLAYER);

	m_name = new_name;

	normalizePlayerName(m_name);

	data >> gender;
	data >> face;
	data >> hair;
	data >> reborn;

	sLog.outDebug("Visualization: gender %u, face %u, hair %u, reborn %u", gender, face, hair, reborn);

	data >> hair_color_R;
	data >> hair_color_G;
	data >> hair_color_B;
	data >> skin_color_R;
	data >> skin_color_G;
	data >> skin_color_B;
	data >> shirt_color;
	data >> misc_color;

	sLog.outDebug("Coloring: hair %u %u %u, skin %u %u %u, shirt %u, misc %u", hair_color_R, hair_color_G, hair_color_B, skin_color_R, skin_color_G, skin_color_B, shirt_color, misc_color);

	data >> element;

	data >> stat_int;
	data >> stat_atk;
	data >> stat_def;
	data >> stat_hpx;
	data >> stat_spx;
	data >> stat_agi;

	data >> lenPassword1;
	if( data.size() < lenPassword1 )
	{
		m_session->SizeError(data, lenPassword1);
		return false;
	}

	for(uint8 i = 0; i < lenPassword1; i++)
	{
		data >> tmp_password1[i];
		password1 += tmp_password1[i];
	}

	pass1 = password1;
	sLog.outDebug("Password1: '%s'", pass1.c_str());

	data >> lenPassword2;
	if( data.size() < lenPassword2 )
	{
		m_session->SizeError(data, lenPassword2);
		return false;
	}

	for(uint8 i = 0; i < lenPassword2; i++)
	{
		data >> tmp_password2[i];
		password2 += tmp_password2[i];
	}
	pass2 = password2;
	sLog.outDebug("Password2: '%s'", pass2.c_str());

	SetUInt32Value(UNIT_FIELD_ELEMENT, element);
	SetUInt32Value(UNIT_FIELD_REBORN, reborn);
	SetUInt32Value(UNIT_FIELD_LEVEL, 1);

	SetMapId(10816);
	Relocate(442, 758);

	SetUInt32Value(UNIT_FIELD_INT, stat_int);
	SetUInt32Value(UNIT_FIELD_ATK, stat_atk);
	SetUInt32Value(UNIT_FIELD_DEF, stat_def);
	SetUInt32Value(UNIT_FIELD_HPX, stat_hpx);
	SetUInt32Value(UNIT_FIELD_SPX, stat_spx);
	SetUInt32Value(UNIT_FIELD_AGI, stat_agi);

	SetUInt32Value(PLAYER_GENDER, gender);
	SetUInt32Value(PLAYER_FACE, face);
	SetUInt32Value(PLAYER_HAIR, hair);
	SetUInt32Value(PLAYER_HAIR_COLOR_R, hair_color_R);
	SetUInt32Value(PLAYER_HAIR_COLOR_G, hair_color_G);
	SetUInt32Value(PLAYER_HAIR_COLOR_B, hair_color_B);
	SetUInt32Value(PLAYER_SKIN_COLOR_R, skin_color_R);
	SetUInt32Value(PLAYER_SKIN_COLOR_G, skin_color_G);
	SetUInt32Value(PLAYER_SKIN_COLOR_B, skin_color_B);
	SetUInt32Value(PLAYER_SHIRT_COLOR, shirt_color);
	SetUInt32Value(PLAYER_MISC_COLOR, misc_color);

	SetUInt32Value(UNIT_FIELD_HP_MAX, GetHPMax());
	SetUInt32Value(UNIT_FIELD_HP, GetHPMax());
	SetUInt32Value(UNIT_FIELD_SP_MAX, GetSPMax());
	SetUInt32Value(UNIT_FIELD_SP, GetSPMax());

	SetUInt32Value(UNIT_FIELD_XP, 0);
	SetUInt32Value(UNIT_FIELD_NEXT_LEVEL_XP, 0);

	SetUInt32Value(UNIT_FIELD_SPELL_POINT, 0);
	SetUInt32Value(UNIT_FIELD_STAT_POINT, 0);

	SetUInt32Value(PLAYER_GOLD_INHAND, 100000);
	SetUInt32Value(PLAYER_GOLD_INBANK, 0);
	return true;
}

void Player::Update( uint32 p_time )
{
	if(!IsInWorld())
		return;

	Unit::Update( p_time );

	if( isBattleInProgress() )
	{
		if( PlayerBattleClass->isActionTimedOut() )
		{
			PlayerBattleClass->BuildActions();
		}

		PlayerBattleClass->UpdateBattleAction();

		///- After UpdateBattleAction, possible the battle is ended
		if( PlayerBattleClass->BattleConcluded() )
		{
			PlayerBattleClass->BattleStop();
			delete PlayerBattleClass;
			PlayerBattleClass = NULL;
		}
	}

	if(m_nextSave > 0)
	{
		if(p_time >= m_nextSave)
		{
			// m_nextSave reseted in SaveToDB call
			SaveToDB();
	//		sLog.outDetail("Player '%u' '%s' Saved", GetAccountId(), GetName());
		}
		else
		{
			m_nextSave -= p_time;
		}
	}
}

void Player::SendMessageToSet(WorldPacket *data, bool self)
{
//	sLog.outString("Player::SendMessageToSet");
	MapManager::Instance().GetMap(GetMapId(), this)->MessageBroadcast(this, data, self);
}

/********************************************************************/
/***                      LOAD SYSTEM                             ***/
/********************************************************************/
bool Player::LoadFromDB( uint32 guid, SqlQueryHolder *holder)
{
	QueryResult *result = holder->GetResult(PLAYER_LOGIN_QUERY_LOADFROM);
	
	if(!result)
	{
		sLog.outError("ERROR: Player (GUID: %u) not found in table `characters`, can't load.", guid);
		return false;
	}

	Field *f = result->Fetch();

	uint32 dbAccountId = f[1].GetUInt32();

	// check if the character's account in the db and the logged in account match.
	// player should be able to load/delete only with correct account!
	if( dbAccountId != GetSession()->GetAccountId() )
	{
		sLog.outError("ERROR: Player (GUID: %u) loading from wrong account (is: %u, should be: %u)", guid, GetSession()->GetAccountId(),dbAccountId);
		delete result;
		return false;
	}

	Object::_Create( guid, HIGHGUID_PLAYER );

	if(!LoadValues( f[FD_DATA].GetString()))
	{
		sLog.outError("ERROR: Player #%d have broken data in `data` field. Can't be loaded.", GUID_LOPART(guid));
		delete result;
		return false;
	}

	///- Reset MOD stat after load values
	//   MOD stat will be re-applied with equipments
	SetInt32Value(UNIT_FIELD_INT_MOD, 0);
	SetInt32Value(UNIT_FIELD_ATK_MOD, 0);
	SetInt32Value(UNIT_FIELD_DEF_MOD, 0);
	SetInt32Value(UNIT_FIELD_HPX_MOD, 0);
	SetInt32Value(UNIT_FIELD_SPX_MOD, 0);
	SetInt32Value(UNIT_FIELD_AGI_MOD, 0);

	// overwrite possible wrong/corrupted guid
	SetUInt64Value(OBJECT_FIELD_GUID,MAKE_GUID(guid,HIGHGUID_PLAYER));

	// cleanup inventory related item value fields (its will be filled correctly in _LoadInvetory
	for(uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
	{
		if (m_items[slot])
		{
			delete m_items[slot];
			m_items[slot] = NULL;
		}
	}

	m_name      = f[FD_CHARNAME].GetCppString();

	sLog.outDebug(">> Load Basic value of player %s is: ", m_name.c_str());

	Relocate(f[FD_POSX].GetUInt16(), f[FD_POSY].GetUInt16());

	SetMapId(f[FD_MAPID].GetUInt16());


	_LoadPets(holder->GetResult(PLAYER_LOGIN_QUERY_LOADPET));

	// must be called after _LoadPet, for equiped Pet equipments
	_LoadInventory(holder->GetResult(PLAYER_LOGIN_QUERY_LOADINVENTORY));

	_LoadSpells(holder->GetResult(PLAYER_LOGIN_QUERY_LOADSPELL));

	///- Dead player walking ?
	if(isDead() || GetHealth() == 0)
	{
		SetUInt32Value(UNIT_FIELD_HP, 1);
		m_deathState = ALIVE;
	}

	///- Fix possible incorrect hp & sp max value
	SetUInt32Value(UNIT_FIELD_HP_MAX, GetHPMax());
	SetUInt32Value(UNIT_FIELD_SP_MAX, GetSPMax());

	sLog.outDebug("");
	sLog.outDebug(" - Name    : %s", GetName());
	sLog.outDebug(" - Level   : %u", GetUInt32Value(UNIT_FIELD_LEVEL));
	sLog.outDebug(" - Element : %u", GetUInt32Value(UNIT_FIELD_ELEMENT));
	sLog.outDebug(" - HP & SP : %u/%u %u/%u", GetUInt32Value(UNIT_FIELD_HP), GetUInt32Value(UNIT_FIELD_HP_MAX), GetUInt32Value(UNIT_FIELD_SP), GetUInt32Value(UNIT_FIELD_SP_MAX));
	sLog.outDebug(" - Map Id  : %u [%u,%u]", GetMapId(), GetPositionX(), GetPositionY());

	DumpPlayer("equip");
	DumpPlayer("pet");
	return true;
}

void Player::_LoadInventory(QueryResult* result)
{
	if( result )
	{
		// prevent items from being added to the queue when stored
		m_itemUpdateQueueBlocked = true;
		do
		{
			Field *f = result->Fetch();
			uint32 pet_guid  = f[1].GetUInt32();
			uint8  slot      = f[2].GetUInt8();
			uint32 item_guid = f[3].GetUInt32();
			uint32 item_id   = f[4].GetUInt32();

			sLog.outDebug("STORAGE: Loading inventory pet %u, slot %u, item_guid %u, item_id %u", pet_guid, slot, item_guid, item_id);

			ItemPrototype const * proto = objmgr.GetItemPrototype(item_id);

			if(!proto)
			{
				sLog.outError("STORAGE: Player %s has an unknown item (id: #%u) in inventory, deleted.", GetName(), item_id );
				CharacterDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_guid);
				CharacterDatabase.PExecute("DELETE FROM item_instance WHERE guid = '%u'", item_guid);
				continue;
			}

			Item *item = new Item;

			if(!item->LoadFromDB(item_guid, GetGUID(), result))
			{
				sLog.outError("STORAGE: Player %s has broken item (id: #%u) in inventory, deleted.", GetName(), item_id);
				CharacterDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_guid);
				item->FSetState(ITEM_REMOVED);
				item->SaveToDB();       // it also deletes item object !
				continue;
			}

			///- TODO not allow have in perticular state

			bool success = true;

			if (!pet_guid)
			{
				// the item is not equiped by pet
				item->SetContainer( NULL );
				item->SetSlot(slot);

				// destinated position
				uint8 dest = slot;

				if( IsInventoryPos( dest ) )
				{
					if( CanStoreItem( slot, dest, item, false ) == EQUIP_ERR_OK )
						item = StoreItem(dest, item, true);
					else
						success = false;
				}
				else if( IsEquipmentPos( dest ) )
				{
					if( CanEquipItem( slot, dest, item, false, false ) == EQUIP_ERR_OK )
						QuickEquipItem(dest, item);
					else
						success = false;
				}
				else if( IsBankPos( dest ) )
				{
					if( CanBankItem( slot, dest, item, false, false ) == EQUIP_ERR_OK )
						item = BankItem(dest, item, true);
					else
						success = false;
				}

				//sLog.outDebug("STORAGE: _LoadInventory Destination slot for %u is %u", item_guid, dest);
			}
			else
			{
				sLog.outDebug("STORAGE: Item %u equipment pet %u", item_id, pet_guid);
				Pet* pet = GetPetByGuid(pet_guid);

				if(!pet)
				{
					sLog.outDebug("STORAGE: Player %s has item (id: %u) belongs to Pet, but can't be equipped to pet, deleted.", GetName(), item_id );
					CharacterDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_guid);
					item->FSetState(ITEM_REMOVED);
					item->SaveToDB();       // it alse deletes item object !
					continue;
				}
				uint8 dest = slot;
				// the item is used by pet, equip it to pet
				if( IsEquipmentPos( dest ) )
				{
					if( CanPetEquipItem( pet, slot, dest, item, false, false ) == EQUIP_ERR_OK )
						QuickPetEquipItem( pet, dest, item);
					else
						success = false;
				}
				else
				{
					CharacterDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_guid);
					sLog.outDebug("STORAGE: Player %s has item (id: %u) belongs to Pet, but can't be equipped to pet, deleted.", GetName(), item_id );
					success = false;
				}
			}
				
			// item's state may have changed after stored
			if( success )
				item->SetState(ITEM_UNCHANGED, this);
			else
			{
				sLog.outError("STORAGE: Player %s has item (GUID: %u Entry: %u) can't be loaded to inventory (Slot: %u) by some reason", GetName(), item_guid, item_id, slot);
				CharacterDatabase.PExecute("DELETE FROM character_inventory WHERE item = '%u'", item_guid);
				//problematicItems.push_back(item);
			}

		} while (result->NextRow());

		delete result;
		m_itemUpdateQueueBlocked = false;

	}

	_ApplyAllItemMods();
				
}

void Player::_LoadPets(QueryResult* result)
{
	bool delete_result = false;
	if( !result )
	{
		QueryResult *result = CharacterDatabase.PQuery("SELECT guid, petslot FROM character_pet WHERE owner_guid = '%u' ORDER BY petslot", GetGUIDLow());
		delete_result = true;
	}

	if( !result )
		return;

	uint8 count = 0;
	// prevent pet from being added to the queue when stored
	m_petUpdateQueueBlocked = true;
	do
	{
		Field *f = result->Fetch();
		uint32 petguid = f[0].GetUInt32();
		uint8  slot    = f[1].GetUInt8();

		if( slot > MAX_PET_SLOT )
			continue;

		Pet *pet = new Pet(this);
		if(!pet->LoadPetFromDB(this, petguid))
		{
			delete pet;
			continue;
		}

		count++;

		SummonPet( slot, pet );

		//m_pets[slot] = pet;
		//pet->SetSlot(slot);


		if( pet->isBattle() )
			m_battlePet = pet;

		// pet's state may have changed after stored
		pet->SetState(PET_UNCHANGED, this);

		//sLog.outString(" - Slot %u pet Entry %u Model %u '%s' is %s", slot + 1, pet->GetEntry(), pet->GetModelId(), pet->GetName(), pet->isBattle() ? "Battle" : "Resting"); 

		if(count > MAX_PET_SLOT)
			break;

	} while( result->NextRow() );

	if (delete_result) delete result;
	m_petUpdateQueueBlocked = false;

	return;
}

void Player::_LoadSpells(QueryResult* result)
{
	bool delete_result = false;
	if( !result )
	{
		result = CharacterDatabase.PQuery("SELECT entry, level FROM character_spell WHERE owner = '%u' ORDER BY entry", GetGUIDLow());
		delete_result = true;
	}

	if( !result )
		return;

	uint8 count = 0;
	do
	{
		Field *f = result->Fetch();
		uint16 entry = f[0].GetUInt16();
		uint8  level = f[1].GetUInt8();

		if( !AddSpell(entry, level) )
		{
			sLog.outError("SPELL: Player %s has an invalid spell (id: #%u), deleted.", GetName(), entry );
			continue;
		}

		count++;

		if(count > MAX_PLAYER_SPELL)
			break;

	} while( result->NextRow() );

	if (delete_result) delete result;
}

void Player::DumpPlayer(const char* section)
{
	if( section == "equip" )
	{
		sLog.outDebug("");
		for(uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)
			if(m_items[slot])
				sLog.outDebug(" @@ Equipment slot %3u equiped [%-20.20s] (%u piece)", slot, m_items[slot]->GetProto()->Name, m_items[slot]->GetCount());
	}

	if( section == "inventory" )
	{
		sLog.outDebug("");
		for(uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
			if (m_items[slot])
				sLog.outDebug(" @@ Inventory slot %2u contain [%-20.20s] (%2u piece) GUID(%u)", slot, m_items[slot]->GetProto()->Name, m_items[slot]->GetCount(), m_items[slot]->GetGUIDLow());
	}

	if( section == "spell" )
	{
		sLog.outDebug("");
		for(SpellMap::iterator it = m_spells.begin(); it != m_spells.end(); ++it)
			sLog.outDebug( " ** Player '%s' has spell <%-20.20s> level [%2u]", GetName(), (*it).second->GetProto()->Name, (*it).second->GetLevel());
	}

	if( section == "pet" )
		for(uint8 slot = 0; slot < MAX_PET_SLOT; slot++)
			if( m_pets[slot] )
			{
				sLog.outDebug("");
				sLog.outDebug(" @@ Pet slot %3u %-10.10s is %s GUID(%u)", m_pets[slot]->GetSlot(), m_pets[slot]->GetName(), (m_pets[slot]->isBattle() ? "Battle" : "Resting"), m_pets[slot]->GetGUIDLow());
				m_pets[slot]->DumpPet();
			}

	//sLog.outDebug("");
}

void Player::_ApplyAllItemMods()
{
	sLog.outDebug("ITEM MODS: _ApplyAllItemMods start.");

	///- Player items mods
	for( uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++)
	{
		if( !m_items[i] )
			continue;

		if( m_items[i]->IsBroken() )
			continue;

		ItemPrototype const *proto = m_items[i]->GetProto();

		if( !proto )
			continue;

		_ApplyItemModsFor(this, m_items[i], true);
	}

	if( GetItemSetCount() == 5 )
		_ApplyItemSetModsFor( this, true);
	else
		_ApplyItemSetModsFor( this, false);


	///- Pet items mods
	for( uint8 slot = 0; slot < MAX_PET_SLOT; slot++ )
	{
		if( !m_pets[slot] )
			continue;

		for( uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; i++ )
		{
			if( !m_pets[slot]->GetEquip(i) )
				continue;

			if( m_pets[slot]->GetEquip(i)->IsBroken() )
				continue;

			ItemPrototype const *proto = m_pets[slot]->GetEquip(i)->GetProto();

			if( !proto )
				continue;

			_ApplyItemModsFor(m_pets[slot], m_pets[slot]->GetEquip(i), true);
		}

		if( m_pets[slot]->GetItemSetCount() == 5 )
			_ApplyItemSetModsFor( m_pets[slot], true );
		else
			_ApplyItemSetModsFor( m_pets[slot], false );

	}
}

void Player::_ApplyItemModsFor(Unit* unit, Item* item, bool apply)
{
	if(!item)
		return;

	if(!item->IsEquipped())
		return;

	ItemPrototype const *proto = item->GetProto();

	if( !proto )
		return;

	std::map<uint8, int32> itemMods;

	uint8 element = unit->GetUInt32Value(UNIT_FIELD_ELEMENT);

	uint8 flag  = 0;
	int32 stat  = 0;
	int32 value = 0;

	bool  el_bonus = false;
	int32 el_value = 0;


	///- Parse all item mod stat
	for(uint8 i = 0; i < 10; i++)
	{
		if( !proto->ItemState[i].ItemStatType )
			continue;

		flag  = 0;
		stat  = 0;
		value = proto->ItemState[i].ItemStatValue;

		switch( proto->ItemState[i].ItemStatType )
		{
			case ITEM_MOD_INT:
				flag = UNIT_FIELD_INT_MOD;
				break;

			case ITEM_MOD_ATK:
				flag = UNIT_FIELD_ATK_MOD;
				break;

			case ITEM_MOD_DEF:
				flag = UNIT_FIELD_DEF_MOD;
				break;

			case ITEM_MOD_AGI:
				flag = UNIT_FIELD_AGI_MOD;
				break;

			case ITEM_MOD_HPX:
				flag = UNIT_FIELD_HPX_MOD;
				break;

			case ITEM_MOD_SPX:
				flag = UNIT_FIELD_SPX_MOD;
				break;

			case ITEM_MOD_ELEMENT_EARTH:
				if( element != ELEMENT_EARTH )
					continue;
				else
				{
					el_bonus = true;
					el_value += value;
					if( apply )
						unit->IncItemSet();
					else
						unit->DecItemSet();
					continue;
				}

			case ITEM_MOD_ELEMENT_WATER:
				if( element != ELEMENT_WATER )
					continue;
				else
				{
					el_bonus = true;
					el_value += value;
					if( apply )
						unit->IncItemSet();
					else
						unit->DecItemSet();
					continue;
				}

			case ITEM_MOD_ELEMENT_FIRE:
				if( element != ELEMENT_FIRE )
					continue;
				else
				{
					el_bonus = true;
					el_value += value;
					if( apply )
						unit->IncItemSet();
					else
						unit->DecItemSet();
					continue;
				}

			case ITEM_MOD_ELEMENT_WIND:
				if( element != ELEMENT_WIND )
					continue;
				else
				{
					el_bonus = true;
					el_value += value;
					if( apply )
						unit->IncItemSet();
					else
						unit->DecItemSet();
					continue;
				}

			default:
				break;
		}
		
		if( !flag )
			continue;

		itemMods.insert( pair<uint8,int32>(flag, value) );

	}

	///- Apply stat mod including bonuses if any
	std::map<uint8, int32>::iterator it;
	for( it = itemMods.begin(); it != itemMods.end(); it++ )
	{
		stat = unit->GetInt32Value((*it).first);

		if( el_bonus && (*it).second > 0 )
			(*it).second += el_value;

		if( apply )
			unit->SetInt32Value((*it).first, stat + (*it).second);
		else
		{
			sLog.outDebug("EQUIP: Unequip Item mods flag %u value %u", (*it).first, (*it).second);
			unit->SetInt32Value((*it).first, stat - (*it).second);
		}
	}

	itemMods.clear();
}

void Player::_ApplyItemSetModsFor( Unit* unit, bool apply )
{
	sLog.outDebug("ITEM MODS: '%s' item set count %u, %s", unit->GetName(), unit->GetItemSetCount(), apply ? "applying" : "removing");

	///- Unapplied only after removing item set
	if( !apply && !unit->isItemSetApplied() )
		return;

	int32 bonus = 0;

	if( apply )
	{
		bonus =  5;
		unit->ItemSetApplied(true);
	}
	else
	{
		bonus = -5;
		unit->ItemSetApplied(false);
	}

	int32 stat_mod = 0;

	stat_mod = unit->GetInt32Value(UNIT_FIELD_INT_MOD);
	unit->SetInt32Value(UNIT_FIELD_INT_MOD, stat_mod + bonus);

	stat_mod = unit->GetInt32Value(UNIT_FIELD_ATK_MOD);
	unit->SetInt32Value(UNIT_FIELD_ATK_MOD, stat_mod + bonus);

	stat_mod = unit->GetInt32Value(UNIT_FIELD_DEF_MOD);
	unit->SetInt32Value(UNIT_FIELD_DEF_MOD, stat_mod + bonus);

	stat_mod = unit->GetInt32Value(UNIT_FIELD_AGI_MOD);
	unit->SetInt32Value(UNIT_FIELD_AGI_MOD, stat_mod + bonus);

}

/***************************************************/
/***                 SAVE SYSTEM                 ***/
/***************************************************/

void Player::SaveToDB()
{
	sLog.outDebug("PLAYER: '%s' Saving to database", GetName());
	// delay auto save at any saves (manual, in code, or autosave)
	m_nextSave = sWorld.getConfig(CONFIG_INTERVAL_SAVE);

	CharacterDatabase.BeginTransaction();

	CharacterDatabase.PExecute("DELETE FROM characters WHERE guid = %u", GetGUIDLow());

	std::ostringstream ss;
	ss << "INSERT INTO characters ( guid, accountid, name, mapid, pos_x, pos_y,"
	   " data ) VALUES ("
		<< GetGUIDLow() << ", "
		<< GetSession()->GetAccountId() << ", '"
		<< m_name << "', "
		<< GetMapId() << ", "
		<< GetPositionX() << ", "
		<< GetPositionY() << ", '";

	uint16 i;
	for( i = 0; i < m_valuesCount; i++ )
	{
		ss << GetUInt32Value(i) << " ";
	}

	ss << "') ";

	CharacterDatabase.Execute( ss.str().c_str() );

	_SavePets();
	_SaveInventory();
	_SaveSpells();

	CharacterDatabase.CommitTransaction();
}

void Player::_SavePets()
{
	if(m_petUpdateQueue.empty()) return;

	sLog.outDebug("PETS: Saving '%s' pets to database", GetName());

	for(size_t i = 0; i < m_petUpdateQueue.size(); i++)
	{
		Pet* pet = m_petUpdateQueue[i];
		if(!pet) continue;

		sLog.outDebug("PETS: Saving pet '%s' to database", pet->GetName());

		pet->SaveToDB();
	}
	m_petUpdateQueue.clear();
}

void Player::_SaveInventory()
{
	if(m_itemUpdateQueue.empty()) return;

	sLog.outDebug("INVENTORY: Saving '%s' inventory to database", GetName());

	for(size_t i = 0; i < m_itemUpdateQueue.size(); i++)
	{
		Item* item = m_itemUpdateQueue[i];
		if(!item) continue;

		sLog.outDebug("INVENTORY: Saving item '%s' to database", item->GetProto()->Name);

		Pet* pet = item->GetContainer();
		uint32 pet_guid = pet ? pet->GetGUIDLow() : 0;

		switch( item->GetState() )
		{
			case ITEM_NEW:
				CharacterDatabase.PExecute("INSERT INTO character_inventory (guid, pet, slot, item, entry) VALUES (%u, %u, %u, %u, %u)", GetGUIDLow(), pet_guid, item->GetSlot(), item->GetGUIDLow(), item->GetEntry());
				break;
			case ITEM_CHANGED:
				CharacterDatabase.PExecute("UPDATE character_inventory SET guid=%u, pet=%u, slot=%u, entry=%u WHERE item=%u", GetGUIDLow(), pet_guid, item->GetSlot(), item->GetEntry(), item->GetGUIDLow());
				break;
			case ITEM_REMOVED:
				CharacterDatabase.PExecute("DELETE FROM character_inventory WHERE item = %u", item->GetGUIDLow());
				break;

			case ITEM_UNCHANGED:
				break;
		}

		item->SaveToDB();
	}
	m_itemUpdateQueue.clear();
}

void Player::_SaveSpells()
{
}

void Player::UpdateBattlePet()
{
	//sLog.outDebug("PLAYER: Update Battle Pet");
	uint16 modelid = 0;

	if( m_battlePet )
		modelid = m_battlePet->GetModelId();

	WorldPacket data;
	data.Initialize( 0x13 );
	data << (uint8 ) (modelid ? 0x01 : 0x02);
	data << (uint16) modelid;
	data << (uint16) 0;

	if( m_session )
		m_session->SendPacket(&data);
}

void Player::SetBattlePet(Pet* pet)
{
	m_battlePet = pet;
}

Pet* Player::GetPetByGuid(uint32 pet_guid) const
{
	for(uint8 slot = 0; slot < MAX_PET_SLOT; slot++)
	{
		if( !m_pets[slot] )
			continue;

		if( m_pets[slot]->GetGUIDLow() == pet_guid )
			return m_pets[slot];
	}
	return NULL;
}

Pet* Player::GetPetByModelId(uint16 modelid) const
{
	for(uint8 slot = 0; slot < MAX_PET_SLOT; slot++)
	{
		if( !m_pets[slot] )
			continue;

		if( m_pets[slot]->GetModelId() == modelid )
			return m_pets[slot];
	}
	return NULL;
}

uint8 Player::GetPetSlot(Pet* pet) const
{
	for(uint8 slot = 0; slot < MAX_PET_SLOT; slot++)
	{
		if( !m_pets[slot] )
			continue;

		if( m_pets[slot] == pet )
			return slot + 1;
	}

	return 0;
}

/*Spell* Player::GetSpellById(uint16 entry) const
{
}*/

void Player::BuildUpdateBlockVisibilityPacket(WorldPacket *data)
{
	data->Initialize( 0x03, 1 );

	*data << (uint32) GetAccountId();
	*data << (uint8 ) GetUInt32Value(PLAYER_GENDER);
	*data << (uint16) 0x0000; // unknown fix from aming
	*data << (uint16) GetMapId();
	*data << (uint16) GetPositionX();
	*data << (uint16) GetPositionY();

	*data << (uint8 ) 0x00;
	*data << (uint8 ) GetUInt32Value(PLAYER_HAIR); //m_hair;
	*data << (uint8 ) GetUInt32Value(PLAYER_FACE); //m_face;
	*data << (uint8 ) GetUInt32Value(PLAYER_HAIR_COLOR_R); // m_hair_color_R;
	*data << (uint8 ) GetUInt32Value(PLAYER_HAIR_COLOR_G); //m_hair_color_G;
	*data << (uint8 ) GetUInt32Value(PLAYER_HAIR_COLOR_B); //m_hair_color_B;
	*data << (uint8 ) GetUInt32Value(PLAYER_SKIN_COLOR_R); //m_skin_color_R;
	*data << (uint8 ) GetUInt32Value(PLAYER_SKIN_COLOR_G); //m_skin_color_G;
	*data << (uint8 ) GetUInt32Value(PLAYER_SKIN_COLOR_B); //m_skin_color_B;
	*data << (uint8 ) GetUInt32Value(PLAYER_SHIRT_COLOR); //m_shirt_color;
	*data << (uint8 ) GetUInt32Value(PLAYER_MISC_COLOR); //m_misc_color;

	uint8  equip_cnt = 0;
	uint16 equip[EQUIPMENT_SLOT_END];

	if (m_items[EQUIPMENT_SLOT_HEAD])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_HEAD]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_BODY])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_BODY]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_WEAPON])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_WEAPON]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_WRISTS])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_WRISTS]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_FEET])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_FEET]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_SPECIAL])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_SPECIAL]->GetModelId();

	*data << (uint8 ) equip_cnt;
	for(uint8 i = 0; i < equip_cnt; i++) *data << equip[i];

	// unknown
	*data << (uint16) 0x00 << (uint16) 0x00 << (uint16) 0x00;
	*data << (uint8 ) GetUInt32Value(UNIT_FIELD_REBORN); //m_reborn;
	*data << m_name.c_str();

}

void Player::BuildUpdateBlockVisibilityForOthersPacket(WorldPacket *data)
{
	data->Initialize( 0x03, 1 );
	*data << (uint32) GetAccountId();
	*data << (uint8 ) GetUInt32Value(PLAYER_GENDER); //m_gender;
	*data << (uint8 ) GetUInt32Value(UNIT_FIELD_ELEMENT); //m_element;
	*data << (uint8 ) GetUInt32Value(UNIT_FIELD_LEVEL); //m_level;
	*data << (uint16) 0x0000;
	*data << (uint16) GetMapId();
	*data << (uint16) GetPositionX();
	*data << (uint16) GetPositionY();
	*data << (uint8 ) 0x00;// << (uint16) 0x0000;
	*data << (uint8 ) GetUInt32Value(PLAYER_HAIR); //m_hair;
	*data << (uint8 ) GetUInt32Value(PLAYER_FACE); //m_face;
	*data << (uint8 ) GetUInt32Value(PLAYER_HAIR_COLOR_R); //m_hair_color_R;
	*data << (uint8 ) GetUInt32Value(PLAYER_HAIR_COLOR_G); //m_hair_color_G;
	*data << (uint8 ) GetUInt32Value(PLAYER_HAIR_COLOR_B); //m_hair_color_B;
	*data << (uint8 ) GetUInt32Value(PLAYER_SKIN_COLOR_R); //m_skin_color_R;
	*data << (uint8 ) GetUInt32Value(PLAYER_SKIN_COLOR_G); //m_skin_color_G;
	*data << (uint8 ) GetUInt32Value(PLAYER_SKIN_COLOR_B); //m_skin_color_B;
	*data << (uint8 ) GetUInt32Value(PLAYER_SHIRT_COLOR); //m_shirt_color;
	*data << (uint8 ) GetUInt32Value(PLAYER_MISC_COLOR); //m_misc_color;

	uint8  equip_cnt = 0;
	uint16 equip[EQUIPMENT_SLOT_END];

	if (m_items[EQUIPMENT_SLOT_HEAD])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_HEAD]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_BODY])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_BODY]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_WEAPON])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_WEAPON]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_WRISTS])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_WRISTS]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_FEET])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_FEET]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_SPECIAL])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_SPECIAL]->GetModelId();

	*data << (uint8 ) equip_cnt;
	for(uint8 i = 0; i < equip_cnt; i++) *data << equip[i];

	*data << (uint32) 0x00000000 << (uint16) 0x0000; // << (uint8) 0x02;
	*data << (uint8 ) GetUInt32Value(UNIT_FIELD_REBORN); //m_reborn; //0x00;
	*data << (uint8 ) 0x00;
	*data << m_name.c_str();

}

void Player::SendInitialPacketsBeforeAddToMap()
{
	WorldPacket data;

	BuildUpdateBlockVisibilityPacket(&data);
	GetSession()->SendPacket(&data);

	UpdatePetCarried();
	UpdatePet();
	UpdateInventory();
	UpdateBattlePet();

	BuildUpdateBlockStatusPacket(&data);
	GetSession()->SendPacket(&data, true);

	//SendMotd();

	AllowPlayerToMove();
	SendUnknownImportant();
}

void Player::SendInitialPacketsAfterAddToMap()
{
	UpdatePlayer();
	UpdateCurrentGold();
}

void Player::UpdatePetCarried()
{
	WorldPacket data;

	data.Initialize( 0x0F );
	data << (uint8 ) 0x08;
	uint8 petcount = 0;
	for(uint8 slot = 0; slot < MAX_PET_SLOT; slot++)
	{
		Pet* pet = m_pets[slot];
		if( !pet )
			continue;

		petcount++;
		data << (uint8 ) (pet->GetSlot() + 1); //petcount;
		data << (uint16) pet->GetModelId(); // pet npc id ;
		data << (uint32) pet->GetExpGained();//0; // pet total xp 0x010D9A19

		data << (uint8 ) pet->GetUInt32Value(UNIT_FIELD_LEVEL); // pet level

		data << (uint16) pet->GetUInt32Value(UNIT_FIELD_HP);  // pet current hp
		data << (uint16) pet->GetUInt32Value(UNIT_FIELD_SP);  // pet current sp
		data << (uint16) pet->GetUInt32Value(UNIT_FIELD_INT); // pet stat int
		data << (uint16) pet->GetUInt32Value(UNIT_FIELD_ATK); // pet stat atk
		data << (uint16) pet->GetUInt32Value(UNIT_FIELD_DEF); // pet stat def
		data << (uint16) pet->GetUInt32Value(UNIT_FIELD_AGI); // pet stat agi
		data << (uint16) pet->GetUInt32Value(UNIT_FIELD_HPX); // pet stat hpx
		data << (uint16) pet->GetUInt32Value(UNIT_FIELD_SPX); // pet stat spx

		data << (uint8 ) 5;    // stat point left ?
		data << (uint8 ) pet->GetLoyalty(); // pet loyalty
		data << (uint8 ) 0x01; // unknown
		data << (uint16) pet->GetUInt32Value(UNIT_FIELD_SPELL_POINT); //pet->GetSkillPoint(); //skill point

		std::string tmpname = pet->GetName();
		data << (uint8 ) tmpname.size(); //name length
		data << tmpname.c_str();

		data << (uint8 ) pet->GetSpellLevelByPos(0); //level skill #1
		data << (uint8 ) pet->GetSpellLevelByPos(1); //level skill #2
		data << (uint8 ) pet->GetSpellLevelByPos(2); //level skill #3

		uint8 durability = 0xFF;
		//data << (uint16) pet->GetUInt32Value(UNIT_FIELD_EQ_HEAD);
		data << (uint16) pet->GetEquipModelId(EQUIPMENT_SLOT_HEAD);
		data << (uint8 ) durability;
		data << (uint32) 0 << (uint16) 0 << (uint8 ) 0; // unknown 7 byte

		//data << (uint16) pet->GetUInt32Value(UNIT_FIELD_EQ_BODY);
		data << (uint16) pet->GetEquipModelId(EQUIPMENT_SLOT_BODY);
		data << (uint8 ) durability;
		data << (uint32) 0 << (uint16) 0 << (uint8 ) 0; // unknown 7 byte

		//data << (uint16) pet->GetUInt32Value(UNIT_FIELD_EQ_WRIST);
		data << (uint16) pet->GetEquipModelId(EQUIPMENT_SLOT_WRISTS);
		data << (uint8 ) durability;
		data << (uint32) 0 << (uint16) 0 << (uint8 ) 0; // unknown 7 byte

		//data << (uint16) pet->GetUInt32Value(UNIT_FIELD_EQ_WEAPON);
		data << (uint16) pet->GetEquipModelId(EQUIPMENT_SLOT_WEAPON);
		data << (uint8 ) durability;
		data << (uint32) 0 << (uint16) 0 << (uint8 ) 0; // unknown 7 byte

		//data << (uint16) pet->GetUInt32Value(UNIT_FIELD_EQ_SHOE);
		data << (uint16) pet->GetEquipModelId(EQUIPMENT_SLOT_FEET);
		data << (uint8 ) durability;
		data << (uint32) 0 << (uint16) 0 << (uint8 ) 0; // unknown 7 byte

		//data << (uint16) pet->GetUInt32Value(UNIT_FIELD_EQ_SPECIAL);
		data << (uint16) pet->GetEquipModelId(EQUIPMENT_SLOT_SPECIAL);
		data << (uint8 ) durability;
		data << (uint32) 0 << (uint16) 0 << (uint8 ) 0; // unknown 7 byte

		data << (uint16) 0x00;
	}

	if( !petcount )
		return;

	GetSession()->SendPacket(&data);
}

void Player::BuildUpdateBlockStatusPacket(WorldPacket *data)
{

	data->Initialize( 0x05, 1 );
	*data << (uint8 ) 0x03;                   // unknown
	*data << (uint8 ) GetUInt32Value(UNIT_FIELD_ELEMENT); //m_element; // uint8
	
	*data << (uint16) GetUInt32Value(UNIT_FIELD_HP); //m_hp; // uint16
	*data << (uint16) GetUInt32Value(UNIT_FIELD_SP); //m_sp; // uint16

	*data << (uint16) GetUInt32Value(UNIT_FIELD_INT); //m_stat_int; //uint16
	*data << (uint16) GetUInt32Value(UNIT_FIELD_ATK); //m_stat_atk; //uint16
	*data << (uint16) GetUInt32Value(UNIT_FIELD_DEF); //m_stat_def; //uint16
	*data << (uint16) GetUInt32Value(UNIT_FIELD_AGI); //m_stat_agi; //uint16
	*data << (uint16) GetUInt32Value(UNIT_FIELD_HPX); //m_stat_hpx; //uint16
	*data << (uint16) GetUInt32Value(UNIT_FIELD_SPX); //m_stat_spx; //uint16

	*data << (uint8 ) getLevel(); //m_level; // uint8
	*data << (uint32) GetExpGained(); //m_xp_gain; //uint32
	*data << (uint16) GetUInt32Value(UNIT_FIELD_SPELL_POINT); //m_skill;
	*data << (uint16) GetUInt32Value(UNIT_FIELD_STAT_POINT); //m_stat;

	*data << (uint32) 0; //m_rank;

	*data << (uint16) GetUInt32Value(UNIT_FIELD_HP_MAX); //m_hp_max;
	*data << (uint16) GetUInt32Value(UNIT_FIELD_SP_MAX); //m_sp_max;
	*data << (uint16) 0;

	*data << (uint16) GetInt32Value(UNIT_FIELD_ATK_MOD); //m_atk_mod;
	*data << (uint16) GetInt32Value(UNIT_FIELD_DEF_MOD); //m_def_mod;
	*data << (uint16) GetInt32Value(UNIT_FIELD_INT_MOD); //m_int_mod;
	*data << (uint16) GetInt32Value(UNIT_FIELD_AGI_MOD); //m_agi_mod;
	*data << (uint16) GetInt32Value(UNIT_FIELD_HPX_MOD); //m_hpx_mod;
	*data << (uint16) GetInt32Value(UNIT_FIELD_SPX_MOD); //m_spx_mod;

	*data << (uint16) GetUInt32Value(UNIT_MORAL_DONGWU); //m_unk1; // Dong Wu
	*data << (uint16) GetUInt32Value(UNIT_MORAL_2); //m_unk2;
	*data << (uint16) GetUInt32Value(UNIT_MORAL_3); //m_unk3;
	*data << (uint16) GetUInt32Value(UNIT_MORAL_4); //m_unk4;
	*data << (uint16) GetUInt32Value(UNIT_MORAL_5); //m_unk5;

	*data << (uint16) 0x00;
	*data << (uint16) 0x0000; //0x0101; // auto attack data ?

	*data << (uint32) 0x00 << (uint32) 0x00 << uint32(0x00) << uint32(0x00);
	*data << (uint32) 0x00 << (uint32) 0x00 << uint32(0x00) << uint32(0x00);
	*data << (uint32) 0x00 << (uint8 ) 0x00;

	///- Try to identify skill info packet
	/*
	*data << (uint16) 0x0000 << (uint8 ) 0x00;
	*data << (uint16) 11001; // Submerge
	*data << (uint8 ) 10;
	*data << (uint16) 14001; // Investigation
	*data << (uint8 ) 1;
	*/

	/*
	for( uint8 i = 0; i < MAX_PLAYER_SPELL; i++)
	{
		if(!m_spells[i])
			break;

		*data << (uint16) m_spells[i]->GetEntry();
		*data << (uint8 ) m_spells[i]->GetLevel();
	}
	*/
	for(SpellMap::iterator itr = m_spells.begin(); itr != m_spells.end(); ++itr)
	{
		*data << (uint16) (*itr).second->GetEntry();
		*data << (uint8 ) (*itr).second->GetLevel();
	}
}

void Player::UpdatePlayer()
{

	///- TODO: Update only when changes
	//_updatePlayer( 0x1B, 1, m_stat_int );
	//_updatePlayer( 0x1C, 1, m_stat_atk );
	//_updatePlayer( 0x1D, 1, m_stat_def );
	//_updatePlayer( 0x1E, 1, m_stat_agi );
	//_updatePlayer( 0x1F, 1, m_stat_hpx );
	//_updatePlayer( 0x20, 1, m_stat_spx );

	//_updatePlayer( 0x23, 1, m_level );
	//_updatePlayer( 0x24, 1, m_xp_gain );
	//_updatePlayer( 0x25, 1, m_skill );
	//_updatePlayer( 0x26, 1, m_stat );

	//_updatePlayer( 0x40, 1, 100 ); // loyalty

	int32 val = 0;
	uint8 mod = 0;

	///- This always updated before/after engaging

	_updatePlayer( 0x19, 1, GetUInt32Value(UNIT_FIELD_HP)); //m_hp );
	_updatePlayer( 0x1A, 1, GetUInt32Value(UNIT_FIELD_SP)); //m_sp );

	val = GetInt32Value(UNIT_FIELD_HPX_MOD);
	mod = val == 0 ? 0 : (val > 0 ? 1 : 2);
	_updatePlayer( UPD_FLAG_HPX_MOD, mod, abs(val) );
	//_updatePlayer( 0xCF, 1, m_hpx_mod );
	
	val = GetInt32Value(UNIT_FIELD_SPX_MOD);
	mod = val == 0 ? 0 : (val > 0 ? 1 : 2);
	_updatePlayer( UPD_FLAG_SPX_MOD, mod, abs(val) );
	//_updatePlayer( 0xD0, 1, m_spx_mod );

	val = GetInt32Value(UNIT_FIELD_ATK_MOD);
	mod = val == 0 ? 0 : (val > 0 ? 1 : 2);
	_updatePlayer( UPD_FLAG_ATK_MOD, mod, abs(val) );
	//_updatePlayer( 0xD2, 1, m_atk_mod );

	val = GetInt32Value(UNIT_FIELD_DEF_MOD);
	mod = val == 0 ? 0 : (val > 0 ? 1 : 2);
	_updatePlayer( UPD_FLAG_DEF_MOD, mod, abs(val) );
	//_updatePlayer( 0xD3, 1, m_def_mod );

	val = GetInt32Value(UNIT_FIELD_INT_MOD);
	mod = val == 0 ? 0 : (val > 0 ? 1 : 2);
	_updatePlayer( UPD_FLAG_INT_MOD, mod, abs(val) );
	//_updatePlayer( 0xD4, 1, m_int_mod );

	val = GetInt32Value(UNIT_FIELD_AGI_MOD);
	mod = val == 0 ? 0 : (val > 0 ? 1 : 2);
	_updatePlayer( UPD_FLAG_AGI_MOD, mod, abs(val) );
	//_updatePlayer( 0xD6, 1, m_agi_mod );
}

void Player::UpdatePlayerLevel()
{
	_updatePlayer( 0x23, 1, getLevel() );
	//_updatePlayer( 0x24, 1, GetExpGained() );
	_updatePlayer( 0x25, 1, GetUInt32Value(UNIT_FIELD_SPELL_POINT));
	_updatePlayer( 0x26, 1, GetUInt32Value(UNIT_FIELD_STAT_POINT));
	resetLevelUp();
}

void Player::UpdatePetLevel(Pet* pet)
{
	uint8 slot = GetPetSlot(pet);

	_updatePet(slot, 0x23, 1, pet->getLevel());
	pet->resetLevelUp();
}

void Player::_updatePlayer(uint8 updflag, uint8 modifier, uint16 value)
{
	//sLog.outDebug("PLAYER: Update player '%s' flag %3u value %u", GetName(), updflag, value);
	WorldPacket data;
	data.Initialize( 0x08 );
	data << (uint8 ) 0x01;                  // flag status for main character
	data << (uint8 ) updflag;               // flag status fields
	data << (uint8 ) modifier;              // +/- modifier
	data << (uint16) value;                 // value modifier
	data << (uint32) 0 << (uint16) 0;

	if( m_session )
		m_session->SendPacket(&data);
}

void Player::UpdatePet()
{
	for(uint8 slot = 0; slot < MAX_PET_SLOT; slot++)
	{
		UpdatePet(slot);
	}
}

void Player::UpdatePet(uint8 slot)
{
	Pet* pet = m_pets[slot];
	if( !pet )
		return;

	int32 val = 0;
	uint8 mod = 0;

	_updatePet(slot, UPD_FLAG_HPX_MOD, 1, 0);
	_updatePet(slot, UPD_FLAG_HP, 1, pet->GetUInt32Value(UNIT_FIELD_HP));

	_updatePet(slot, UPD_FLAG_SPX_MOD, 1, 0);
	_updatePet(slot, UPD_FLAG_SP, 1, pet->GetUInt32Value(UNIT_FIELD_SP));

	val = pet->GetInt32Value(UNIT_FIELD_ATK_MOD);
	mod = val == 0 ? 0 : (val > 0 ? 1 : 2);
	_updatePet(slot, UPD_FLAG_ATK_MOD, mod, abs(val));
	//_updatePet(slot, UPD_FLAG_ATK_MOD, 1, 0);

	val = pet->GetInt32Value(UNIT_FIELD_DEF_MOD);
	mod = val == 0 ? 0 : (val > 0 ? 1 : 2);
	_updatePet(slot, UPD_FLAG_DEF_MOD, mod, abs(val));
	//_updatePet(slot, UPD_FLAG_DEF_MOD, 1, 0);

	val = pet->GetInt32Value(UNIT_FIELD_INT_MOD);
	mod = val == 0 ? 0 : (val > 0 ? 1 : 2);
	_updatePet(slot, UPD_FLAG_INT_MOD, mod, abs(val));
	//_updatePet(slot, UPD_FLAG_INT_MOD, 1, 0);

	val = pet->GetInt32Value(UNIT_FIELD_AGI_MOD);
	mod = val == 0 ? 0 : (val > 0 ? 1 : 2);
	_updatePet(slot, UPD_FLAG_AGI_MOD, mod, abs(val));
	//_updatePet(slot, UPD_FLAG_AGI_MOD, 1, 0);
}

void Player::UpdatePetBattle()
{
	Pet* bpet = GetBattlePet();
	for(uint8 slot = 0; slot < MAX_PET_SLOT; slot++)
		if( bpet == m_pets[slot])
		{
			UpdatePet(slot);
			break;
		}
}

void Player::_updatePet(uint8 slot, uint8 updflag, uint8 modifier, uint32 value)
{
	//sLog.outDebug("PLAYER: Update Pet slot %u flag %3u value %u", slot, updflag, value);
	WorldPacket data;
	data.Initialize( 0x08 );
	data << (uint8 ) 0x02 << (uint8 ) 0x04;
	data << (uint8 ) (slot+1); // pet slot number, starting from 1
	data << (uint8 ) 0x00;
	data << (uint8 ) updflag;
	data << (uint8 ) modifier;
	data << (uint32) value;
	data << (uint32) 0;

	if( m_session )
		m_session->SendPacket(&data);
}

void Player::UpdateInventory()
{
	WorldPacket data;
	data.Initialize( 0x17 );
	data << (uint8 ) 0x05;
	uint8 count = 0;
	for(uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; slot++)
	{
		if( !m_items[slot] )
			continue;

		count++;
		data << (uint8 ) (slot - INVENTORY_SLOT_ITEM_START + 1);
		data << (uint16) m_items[slot]->GetProto()->modelid;
		data << (uint8 ) m_items[slot]->GetCount();
		data << (uint32) 0;
		data << (uint32) 0;
	}

	if( !count )
		return;

	if( m_session )
		m_session->SendPacket(&data);
}

void Player::UpdateInventoryForItem(uint16 modelid, uint8 count)
{
	WorldPacket data;
	data.Initialize( 0x17 );
	data << (uint8 ) 0x06;
	data << (uint16) modelid;
	data << (uint16) count;
	data << (uint32) 0;

	if( m_session )
		m_session->SendPacket(&data);
}

void Player::UpdateCurrentEquip()
{
	WorldPacket data;
	data.Initialize( 0x05, 1 );
	data << (uint8) 0x00;
	data << GetAccountId();

	uint8  equip_cnt = 0;
	uint16 equip[EQUIPMENT_SLOT_END];

	if (m_items[EQUIPMENT_SLOT_HEAD])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_HEAD]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_BODY])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_BODY]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_WEAPON])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_WEAPON]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_WRISTS])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_WRISTS]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_FEET])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_FEET]->GetModelId();

	if (m_items[EQUIPMENT_SLOT_SPECIAL])
		equip[equip_cnt++] = m_items[EQUIPMENT_SLOT_SPECIAL]->GetModelId();

	for(uint8 i = 0; i < equip_cnt; i++) data << equip[i];

	if( m_session )
		m_session->SendPacket(&data);
}

void Player::UpdateCurrentGold()
{
	WorldPacket data;
	data.Initialize( 0x1A, 1 );
	data << (uint8 ) 0x04;
	data << (uint32) GetUInt32Value(PLAYER_GOLD_INHAND); //m_gold_hand; // gold
	data << (uint32) 0x00000000;

	if( m_session )
		m_session->SendPacket(&data);
}

void Player::Send0504()
{
	///- tell the client to wait for request
	WorldPacket data;
	data.Initialize( 0x05, 1 );
	data << (uint8) 0x04;
	
	if( m_session )
		m_session->SendPacket(&data);
}

void Player::Send0F0A()
{
	WorldPacket data;
	data.Initialize( 0x0F, 1 );
	data << (uint8) 0x0A;

	if( m_session )
		m_session->SendPacket(&data);
}

void Player::Send0602()
{
	WorldPacket data;
	data.Initialize( 0x06, 1 );
	data << (uint8) 0x02;

	if( m_session )
		m_session->SendPacket(&data);
}

void Player::Send1408()
{
	///- tell the client request is completed
	WorldPacket data;
	data.Initialize( 0x14, 1 );
	data << (uint8) 0x08;

	if( m_session )
		m_session->SendPacket(&data);
}

void Player::AllowPlayerToMove()
{
	Send0504(); Send0F0A(); //EndOfRequest();
}

void Player::EndOfRequest()
{
	Send1408();
}

void Player::BuildUpdateBlockTeleportPacket(WorldPacket* data)
{
	data->Initialize( 0x0C, 1 );
	*data << GetAccountId();
	*data << GetTeleportTo();
	*data << GetPositionX();
	*data << GetPositionY();
	*data << (uint8) 0x01 << (uint8) 0x00;
}


void Player::AddToWorld()
{
	///- Do not add/remove the player from the object storage
	///- It will crash when updating the ObjectAccessor
	///- The player should only be added when logging in
	Object::AddToWorld();

}

void Player::RemoveFromWorld()
{
	///- Do not add/remove the player from the object storage
	///- It will crash when updating the ObjectAccessor
	///- The player should only be removed when logging out
	Object::RemoveFromWorld();
}

void Player::SendDelayResponse(const uint32 ml_seconds)
{
}

void Player::UpdateVisibilityOf(WorldObject* target)
{
	/* this is working one */
	/*
	sLog.outString("Player::UpdateVisibilityOf '%s' to '%s'",
		GetName(), target->GetName());
	target->SendUpdateToPlayer(this);
	SendUpdateToPlayer((Player*) target);
	*/
	if(HaveAtClient(target))
	{
		//if(!target->isVisibleForInState(this, true))
		{
			//target->DestroyForPlayer(this);
			m_clientGUIDs.erase(target->GetGUID());

		}
	}
	else
	{
		//if(target->isVisibleForInState(this,false))
		{
			if(!target->isType(TYPE_PLAYER))
				return;

			target->SendUpdateToPlayer(this);
		}
	}
}

void Player::TeleportTo(uint16 mapid, uint16 pos_x, uint16 pos_y)
{
	sLog.outDebug( "PLAYER: Teleport Player '%s' to %u [%u,%u]", GetName(), mapid, pos_x, pos_y );

	WorldPacket data;

	SetDontMove(true);

	Map* map = MapManager::Instance().GetMap(GetMapId(), this);
	map->Remove(this, false);

	SetMapId(mapid);
	Relocate(pos_x, pos_y);

	// update player state for other and vice-versa
	CellPair p = LeGACY::ComputeCellPair(GetPositionX(), GetPositionY());
	Cell cell(p);
	MapManager::Instance().GetMap(GetMapId(), this)->EnsureGridLoadedForPlayer(cell, this, true);
}
	
void Player::SendMapChanged()
{
	WorldPacket data;
	data.Initialize( 0x0C, 1 );
	data << GetAccountId();
	data << GetMapId();
	data << GetPositionX();
	data << GetPositionY();
	data << (uint8) 0x01 << (uint8) 0x00;

	if( m_session )
		m_session->SendPacket(&data);


	Send0504();  // This is important, maybe tell the client to Wait Cursor

	Map* map = MapManager::Instance().GetMap(GetMapId(), this);
	map->Add(this);

	UpdateMap2Npc();

	SetDontMove(false);
}

void Player::UpdateRelocationToSet()
{
	///- Send Relocation Message to Set
	WorldPacket data;

	data.Initialize( 0x06, 1 );
	data << (uint8) 0x01;
	data << GetAccountId();
	data << (uint8) 0x05;
	data << GetPositionX() << GetPositionY();
	SendMessageToSet(&data, false);
}

bool Player::HasSpell(uint32 spell) const
{
	return false;
}

void Player::TalkedToCreature( uint32 entry, uint64 guid )
{
	m_talkedCreatureGuid = guid;    // store last talked to creature
	m_talkedSequence     = 0;       // reset sequence number
	PlayerTalkClass->CloseMenu();   // close any open menu
	PlayerTalkClass->InitTalking(); // send talk initial packet
}

uint8 Player::FindEquipSlot( ItemPrototype const* proto, uint32 slot, bool swap ) const
{
	sLog.outDebug("STORAGE: FindEquipSlot '%s' slot %u", proto->Name, slot);
	uint8 slots;
	slots = NULL_SLOT;
	switch( proto->InventoryType )
	{
		case INVTYPE_HEAD:
			slots = EQUIPMENT_SLOT_HEAD;
			break;
		case INVTYPE_BODY:
			slots = EQUIPMENT_SLOT_BODY;
			break;
		case INVTYPE_WEAPON:
			slots = EQUIPMENT_SLOT_WEAPON;
			break;
		case INVTYPE_WRISTS:
			slots = EQUIPMENT_SLOT_WRISTS;
			break;
		case INVTYPE_FEET:
			slots = EQUIPMENT_SLOT_FEET;
			break;
		case INVTYPE_SPECIAL:
			slots = EQUIPMENT_SLOT_SPECIAL;
			break;

		default:
			return NULL_SLOT;
	}

	if( slot != NULL_SLOT )
	{
		if( swap || !GetItemByPos( slot ) )
		{
			if ( slots == slot )
			{
				sLog.outString("STORAGE: FindEquipSlot '%s' is equipable in slot %u", proto->Name, slot);
				return slot;
			}
		}
	}
	else
	{
		if ( slots != NULL_SLOT && !GetItemByPos( slots ) )
		{
			return slots;
		}

		if ( slots != NULL_SLOT && swap )
			return slots;
	}

	sLog.outDebug("STORAGE: No Player equipment free slot position");
	return NULL_SLOT;
}

uint8 Player::FindPetEquipSlot( Pet* pet, ItemPrototype const* proto, uint32 slot, bool swap ) const
{
	sLog.outDebug("STORAGE: FindPetEquipSlot '%s' slot %u", proto->Name, slot);
	uint8 slots;
	slots = NULL_SLOT;
	switch( proto->InventoryType )
	{
		case INVTYPE_HEAD:
			slots = EQUIPMENT_SLOT_HEAD;
			break;
		case INVTYPE_BODY:
			slots = EQUIPMENT_SLOT_BODY;
			break;
		case INVTYPE_WEAPON:
			slots = EQUIPMENT_SLOT_WEAPON;
			break;
		case INVTYPE_WRISTS:
			slots = EQUIPMENT_SLOT_WRISTS;
			break;
		case INVTYPE_FEET:
			slots = EQUIPMENT_SLOT_FEET;
			break;
		case INVTYPE_SPECIAL:
			slots = EQUIPMENT_SLOT_SPECIAL;
			break;

		default:
			return NULL_SLOT;
	}

	if( slot != NULL_SLOT )
	{
		if( swap || !GetPetItemByPos( pet, slot ) )
		{
			if( slots == slot )
			{
				sLog.outDebug("STORAGE: FindPetEquipeSlot '%s' is equipable in slot %u", proto->Name, slot);
				return slot;
			}
		}
	}
	else
	{
		if( slots != NULL_SLOT && !GetPetItemByPos( pet, slots ) )
		{
			return slots;
		}

		if( slots != NULL_SLOT && swap )
			return slots;
	}

	sLog.outDebug("EQUIP: No Pet equipment free slot position");
	return NULL_SLOT;
}

//////////////////////// PET SYSTEM ///////////////////////////////////
Pet* Player::CreatePet( uint32 pet ) const
{
	CreatureInfo const *pProto = objmgr.GetCreatureTemplate( pet );
	if( pProto )
	{
		sLog.outDebug("PETS: Creature Template found <%u>, creating", pet);
		Pet *pPet = new Pet(NULL);
		if( pPet->Create(objmgr.GeneratePetNumber(), pet, const_cast<Player*>(this)) )
		{
			sLog.outDebug("PETS: Pet <%u> created", pet);
			return pPet;
		}
		else
			delete pPet;
	}
	else
		sLog.outDebug("PETS: Creature Template not found <%u>", pet);
	return NULL;
}

uint8 Player::CanSummonPet( uint8 slot, uint8 &dest, Pet *pPet, bool swap) const
{
	dest = 0;
	if( pPet )
	{
		sLog.outDebug("PETS: CanSummonPet slot = %u, pet = %u", slot, pPet->GetEntry());
		uint8 sslot = FindSummonSlot( slot, swap );
		if( sslot == NULL_PET_SLOT )
			return PET_ERR_PET_CANT_BE_SUMMONED;

		dest = sslot;
		return PET_ERR_OK;
	}
	if( !swap )
		return PET_ERR_PET_NOT_FOUND;
	else
		return PET_ERR_PET_CANT_BE_SWAPPED;
}

uint8 Player::FindSummonSlot( uint8 slot, bool swap ) const
{
	sLog.outDebug("PETS: FindSummonSlot '%u'", slot);
	uint8 slots;

	slots = NULL_PET_SLOT;
	for(uint8 i = PET_SLOT_START; i < PET_SLOT_END; i++)
	{
		if( !m_pets[i] )
		{
			slots = i;
			break;
		}
	}

	if( slot != NULL_PET_SLOT )
	{
		if( swap || !GetPet( slot ) )
		{
			if( slots == slot )
			{
				sLog.outDebug("PETS: FindSummonSlot is summonable in slot %u", slot);
				return slot;
			}
		}
	}
	else
	{
		if( slots != NULL_PET_SLOT && !GetPet( slots ) )
		{
			return slots;
		}

		if( slots != NULL_PET_SLOT && swap )
			return slots;
	}

	sLog.outDebug("PETS: No Player summon free slot position");
	return NULL_PET_SLOT;
}

Pet* Player::SummonPet( uint8 pos, Pet* pPet )
{
	if (!pPet )
		return pPet;

	sLog.outDebug("PETS: SummonPet '%s' slot %u", pPet->GetName(), pos);

	Pet* pPet2 = GetPet( pos );
	if( !pPet2 )
	{
		VisualizePet( pos, pPet );
	}
	else
	{
		sLog.outDebug("PETS: SummonPet destination is not null");
		pPet->SetOwnerGUID(GetGUID());
		pPet->SetState(PET_REMOVED, this);
		pPet2->SetState(PET_CHANGED, this);
		return pPet2;
	}
	return pPet;
}

void Player::VisualizePet( uint8 pos, Pet *pPet)
{
	if(!pPet)
		return;

	uint8 slot = pos;

	m_pets[slot] = pPet;

	pPet->SetUInt64Value( ITEM_FIELD_CONTAINED, GetGUID() );
	pPet->SetUInt64Value( ITEM_FIELD_OWNER, GetGUID() );
	pPet->SetSlot( slot );

	pPet->SetState(PET_CHANGED, this);
	//sLog.outDebug("PETS: VisualizePet '%s' slot %u", pPet->GetName(), pos);
}

void Player::ReleasePet( uint8 slot )
{
	if( slot > MAX_PET_SLOT )
		return;

	if( !m_pets[slot] )
		return;

	m_pets[slot]->SetState(PET_REMOVED);
	m_pets[slot] = NULL;

	_SavePets();

}

///////////////////// INVENTORY SYSTEM //////////////////////////////////
Item* Player::CreateItem( uint32 item, uint32 count ) const
{
	ItemPrototype const *pProto = objmgr.GetItemPrototype( item );
	if( pProto )
	{
		sLog.outDebug("STORAGE: Item Prototype found <%u>, creating", item);
		Item *pItem = new Item;
		if ( count > pProto->Stackable )
			count = pProto->Stackable;
		if ( count < 1 )
			count = 1;
		if( pItem->Create(objmgr.GenerateLowGuid(HIGHGUID_ITEM), item, const_cast<Player*>(this)) )
		{
			sLog.outDebug("STORAGE: Item <%u> created", item);
			pItem->SetCount( count );
			return pItem;
		}
		else
			delete pItem;
	}
	else
		sLog.outDebug("STORAGE: Item Prototype not found <%u>", item);
	return NULL;
}

uint8 Player::CanUnequipItems( uint32 item, uint32 count ) const
{
/*
	Item *pItem;
	uint32 tempcount = 0;

	uint8 res = EQUIP_ERR_OK;

	for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_BAG_END; i++)
	{
		pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
		if( pItem && pItem->GetEntry() == item )
		{
			uint8 ires = CanUnequipItem(INVENTORY_SLOT_BAG_0 << 8 | i, false);
			if(ires==EQUIP_ERR_OK)
			{
				tempcount += pItem->GetCount();
				if( tempcount >= count )
					return EQUIP_ERR_OK;
			}
			else
				res = ires;
		}
	}
	for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
	{
		pItem = GetItemByPos( INVENTORY_SLOT_BAG_0, i );
		if( pItem && pItem->GetEntry() == item )
		{
			tempcount += pItem->GetCount();
			if( tempcount >= count )
				return EQUIP_ERR_OK;
		}
	}
	
	// not found req. item count and have unequippable items
	return res;
*/
}

uint32 Player::GetItemCount( uint32 item, Item* eItem ) const
{
	Item *pItem;
	uint32 count = 0;
	for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
	{
		pItem = GetItemByPos(i);
		if( pItem && pItem != eItem && pItem->GetEntry() == item )
			count += pItem->GetCount();
	}

	return count;
}

uint32 Player::GetBankItemCount( uint32 item, Item* eItem ) const
{
	Item *pItem;
	uint32 count = 0;
	for(int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; i++)
	{
		pItem = GetItemByPos(i);
		if( pItem && pItem != eItem && pItem->GetEntry() == item )
			count += pItem->GetCount();
	}
	return count;
}

Item* Player::GetItemByGuid( uint64 guid ) const
{
	for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
	{
		Item *pItem = GetItemByPos( i );
		if( pItem && pItem->GetGUID() == guid )
			return pItem;
	}

	return NULL;
}

Item* Player::GetItemByPos( uint8 slot ) const
{
	if( m_items[slot] )
		return m_items[slot];
	else
		return NULL;
}

Item* Player::GetPetItemByPos( Pet* pet, uint8 slot ) const
{
	if( pet->GetEquip( slot ) )
		return pet->GetEquip( slot );
	else
		return NULL;
}

bool Player::IsInventoryPos( uint8 slot )
{
	if( slot == NULL_SLOT )
		return true;
	if( slot >= INVENTORY_SLOT_ITEM_START && slot < INVENTORY_SLOT_ITEM_END )
		return true;
	return false;
}

bool Player::IsEquipmentPos( uint8 slot )
{
	if( slot < EQUIPMENT_SLOT_END )
		return true;
	return false;
}

bool Player::IsBankPos( uint8 slot )
{
	if( slot >= BANK_SLOT_ITEM_START && slot < BANK_SLOT_ITEM_END )
		return true;
	return false;
}

bool Player::IsBagPos( uint8 slot )
{
	if( slot >= INVENTORY_SLOT_BAG_START && slot < INVENTORY_SLOT_BAG_END )
		return true;
	return false;
}

bool Player::HasItemCount( uint32 item, uint32 count ) const
{
	Item *pItem;
	uint32 tempcount = 0;
	for(int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; i++)
	{
		pItem = GetItemByPos( i );
		if( pItem && pItem->GetEntry() == item )
		{
			tempcount += pItem->GetCount();
			if( tempcount >= count )
				return true;
		}
	}
	return false;
}

uint8 Player::CanStoreNewItem( uint8 slot, uint8 &dest, uint32 item, uint32 count, bool swap ) const
{
	dest = 0;
	Item *pItem = CreateItem( item, count );
	if( pItem )
	{
		uint8 result = CanStoreItem( slot, dest, pItem, swap );
		delete pItem;
		return result;
	}
	if( !swap )
		return EQUIP_ERR_ITEM_NOT_FOUND;
	else
		return EQUIP_ERR_ITEMS_CANT_BE_SWAPPED;
}

uint8 Player::CanTakeMoreSimilarItems(Item* pItem) const
{
	ItemPrototype const *pProto = pItem->GetProto();
	if( !pProto )
		return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;

	// no maximum
	if(pProto->MaxCount == 0)
		return EQUIP_ERR_OK;

	uint32 curcount = GetItemCount(pProto->ItemId,pItem) + GetBankItemCount(pProto->ItemId,pItem);

	if( curcount + pItem->GetCount() > pProto->MaxCount )
		return EQUIP_ERR_CANT_CARRY_MORE_OF_THIS;

	return EQUIP_ERR_OK;
}

uint8 Player::CanStoreItem( uint8 slot, uint8 &dest, Item *pItem, bool swap ) const
{
	dest = 0;
	if( pItem )
	{
		sLog.outDebug("STORAGE: CanStoreItem slot = %u, item = %u, count = %u", slot, pItem->GetEntry(), pItem->GetCount());
		ItemPrototype const *pProto = pItem->GetProto();
		if( pProto )
		{
			sLog.outDebug("STORAGE: CanStoreItem for '%s'", pProto->Name);
			Item *pItem2;
			uint16 pos;
			//if(pItem->IsBindedNotWith(GetGUID()))
			//	return EQUIP_ERR_DONT_OWN_THAT_ITEM;

			// check count of items (skip for auto move for same player from bank)
			uint8 res = CanTakeMoreSimilarItems(pItem);
			if(res != EQUIP_ERR_OK)
				return res;
			
			// search stack for merge to
			if( pProto->Stackable > 1 )
			{
				for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
				{
					pos = i;
					pItem2 = GetItemByPos( pos );
					if( pItem2 && pItem2->GetEntry() == pItem->GetEntry() && pItem2->GetCount() + pItem->GetCount() <= pProto->Stackable )
					{
						sLog.outDebug("STORAGE: CanStoreItem still stackable %u <= %u", pItem2->GetCount() + pItem->GetCount(), pProto->Stackable);
						dest = pos;
						return EQUIP_ERR_OK;
					}
				}
			}

			// search free slot
			for(int i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++)
			{
				pItem2 = GetItemByPos( i );
				if( !pItem2 )
				{
					dest = i;
					return EQUIP_ERR_OK;
				}
			}

			sLog.outDebug("STORAGE: CanStoreItem Inventory is full");
			return EQUIP_ERR_INVENTORY_FULL;
		}
	}
	sLog.outDebug("STORAGE: CanStoreItem pItem is NULL");
	if( !swap )
		return EQUIP_ERR_ITEM_NOT_FOUND;
	else
		return EQUIP_ERR_ITEMS_CANT_BE_SWAPPED;
}

////////////////////////////////////////////////////////////////////////////
uint8 Player::CanStoreItems( Item **pItems, int count) const
{
	return EQUIP_ERR_OK;
}

///////////////////////////////////////////////////////////////////////////
uint8 Player::CanEquipNewItem( uint8 slot, uint8 &dest, uint32 item, uint32 count, bool swap ) const
{
	dest = 0;
	Item *pItem = CreateItem( item, count );
	if( pItem )
	{
		uint8 result = CanEquipItem(slot, dest, pItem, swap );
		delete pItem;
		return result;
	}

	return EQUIP_ERR_ITEM_NOT_FOUND;
}

uint8 Player::CanEquipItem( uint8 slot, uint8 &dest, Item *pItem, bool swap, bool not_loading ) const
{
	dest = 0;
	if( pItem )
	{
		sLog.outDebug("STORAGE: CanEquipItem slot = %u, item = %u, count = %u", slot, pItem->GetEntry(), pItem->GetCount());
		ItemPrototype const *pProto = pItem->GetProto();

		if( pProto )
		{
			//if(pItem->IsBindedNotWith(GetGUID()))
			//	return EQUIP_ERR_DONT_OWN_THAT_ITEM;

			// check count of items (skip for auto move for same player from bank)
			uint8 res = CanTakeMoreSimilarItems(pItem);
			if(res != EQUIP_ERR_OK)
				return res;

			uint8 eslot = FindEquipSlot( pProto, slot, swap );
			if( eslot == NULL_SLOT )
				return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;

			uint8 msg = CanUseItemFor((Unit*) this, pItem );
			if( msg != EQUIP_ERR_OK )
				return msg;
		
			dest = eslot;
			return EQUIP_ERR_OK;
		}
	}
	if( !swap )
		return EQUIP_ERR_ITEM_NOT_FOUND;
	else
		return EQUIP_ERR_ITEMS_CANT_BE_SWAPPED;
}

uint8 Player::CanPetEquipItem( Pet* pet, uint8 slot, uint8 &dest, Item *pItem, bool swap, bool not_loading ) const
{
	dest = 0;
	if( pItem )
	{
		sLog.outDebug("STORAGE: CanPetEquipItem slot = %u, item = %u, count = %u", slot, pItem->GetEntry(), pItem->GetCount());
		ItemPrototype const *pProto = pItem->GetProto();

		if( pProto )
		{
			uint8 res = CanTakeMoreSimilarItems(pItem);
			if(res != EQUIP_ERR_OK)
				return res;

			uint8 eslot = FindPetEquipSlot( pet, pProto, slot, swap );
			if( eslot == NULL_SLOT )
				return EQUIP_ERR_ITEM_CANT_BE_EQUIPPED;

			uint8 msg = CanUseItemFor( pet, pItem );
			if( msg != EQUIP_ERR_OK )
				return msg;

			dest = eslot;
			return EQUIP_ERR_OK;
		}
	}
	if( !swap )
		return EQUIP_ERR_ITEM_NOT_FOUND;
	else
		return EQUIP_ERR_ITEMS_CANT_BE_SWAPPED;
}

uint8 Player::CanUseItemFor( Unit* unit, Item* pItem ) const
{
	ASSERT( unit );

	if( pItem )
	{
		sLog.outDebug("STORAGE: CanUseItem for '%s' item = %u", unit->GetName(), pItem->GetEntry());

		ItemPrototype const *pProto = pItem->GetProto();
		if( pProto )
		{
			if( unit->getLevel() < pProto->RequiredLevel )
				return EQUIP_ERR_CANT_EQUIP_LEVEL_I;

			return EQUIP_ERR_OK;
		}
	}
	return EQUIP_ERR_ITEM_NOT_FOUND;
}

uint8 Player::CanUnequipItem( uint8 pos, bool swap ) const
{
	// Applied only to equipped items
	if(!IsEquipmentPos(pos))
		return EQUIP_ERR_OK;

	Item* pItem = GetItemByPos(pos);

	// Applied only to existed equipped item
	if( !pItem )
		return EQUIP_ERR_OK;

	sLog.outDebug( "STORAGE: CanUnequipItem slot = %u, item = %u, count = %u", pos, pItem->GetEntry(), pItem->GetCount());

	ItemPrototype const *pProto = pItem->GetProto();
	if( !pProto )
		return EQUIP_ERR_ITEM_NOT_FOUND;

	return EQUIP_ERR_OK;
}

uint8 Player::CanPetUnequipItem( Pet* pet, uint8 pos, bool swap ) const
{
	// Applied only to equipped items
	if(!IsEquipmentPos(pos))
		return EQUIP_ERR_OK;

	Item* pItem = pet->GetEquip(pos);

	// Applied only to existed equipped item
	if( !pItem )
		return EQUIP_ERR_OK;

	sLog.outDebug("STORAGE: CanPetUnequipItem slot = %u, item = %u, count = %u", pos, pItem->GetEntry(), pItem->GetCount());

	ItemPrototype const *pProto = pItem->GetProto();
	if( !pProto )
		return EQUIP_ERR_ITEM_NOT_FOUND;

	return EQUIP_ERR_OK;
}

uint8 Player::CanBankItem( uint8 slot, uint8 &dest, Item *pItem, bool swap, bool not_loading ) const
{
	return EQUIP_ERR_OK;
}

uint8 Player::CanUseItem( Item *pItem, bool not_loading ) const
{
	return EQUIP_ERR_OK;
}

bool Player::CanUseItem( ItemPrototype const *pProto )
{
	return true;
}

// Return stored item (if stored to stack, it can diff. from pItem). And pItem can be deleted in this case.
Item* Player::StoreNewItem( uint8 pos, uint32 item, uint32 count, bool update, int32 randomPropertyId)
{
	Item *pItem = CreateItem( item, count );
	if( pItem )
	{
		Item * retItem = StoreItem( pos, pItem, update );

		return retItem;
	}
	return NULL;
}

// Return stored item (if stored to stack, it can diff. from pItem). And pItem can be deleted in this case.
Item* Player::StoreItem( uint8 pos, Item *pItem, bool update )
{
	if( pItem )
	{
		uint8 slot = pos ;

		//sLog.outDebug( "STORAGE: StoreItem slot = %u, item = %u, count = %u", slot, pItem->GetEntry(), pItem->GetCount());

		Item *pItem2 = GetItemByPos( slot );

		if( !pItem2 )
		{
			m_items[slot] = pItem;
//			SetUInt64Value( (uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2) ), pItem->GetGUID() );
			pItem->SetUInt64Value( ITEM_FIELD_CONTAINED, GetGUID() );
			pItem->SetUInt64Value( ITEM_FIELD_OWNER, GetGUID() );

			pItem->SetSlot( slot );
			pItem->SetContainer( NULL );

			if( IsInWorld() && update )
			{
				pItem->AddToWorld();
				//pItem->SendUpdateToPlayer( this );
			}

			pItem->SetState(ITEM_CHANGED, this);
		}
		else
		{
			pItem2->SetCount( pItem2->GetCount() + pItem->GetCount() );
			//if( IsInWorld() && update )
			//	pItem2->SendUpdateToPlayer( this );

			if( IsInWorld() && update )
			{
				pItem->RemoveFromWorld();
				pItem->DestroyForPlayer( this );
			}

			pItem->SetOwnerGUID(GetGUID());  // prevent error at next SetState in case trade/buy from vendor
			pItem->SetState(ITEM_REMOVED, this);
			pItem2->SetState(ITEM_CHANGED, this);

			return pItem2;
		}
	}

	return pItem;
}

Item* Player::EquipNewItem( uint8 pos, uint32 item, uint32 count, bool update )
{
	Item *pItem = CreateItem( item, count );
	if( pItem )
	{
		Item * retItem = EquipItem( pos, pItem, update );

		return retItem;
	}
	return NULL;
}

Item* Player::EquipItem( uint8 pos, Item *pItem, bool update )
{
	if( !pItem )
		return pItem;

	sLog.outDebug("STORAGE: EquipItem '%s' slot %u", pItem->GetProto()->Name, pos);
	Item* pItem2 = GetItemByPos( pos );
	if( !pItem2 )
	{
		VisualizeItem( pos, pItem );
	}
	else
	{
		//sLog.outDebug("STORAGE: EquipItem destination is not null");
		pItem2->SetCount( pItem2->GetCount() + pItem->GetCount() );
		pItem->SetOwnerGUID(GetGUID());
		pItem->SetState(ITEM_REMOVED, this);
		pItem2->SetState(ITEM_CHANGED, this);
		return pItem2;
	}
	return pItem;
}

Item* Player::PetEquipItem( Pet* pet, uint8 pos, Item *pItem, bool update )
{
	if( !pItem )
		return pItem;

	sLog.outDebug("STORAGE: EquipItem '%s' item '%s' slot %u", pet->GetName(), pItem->GetProto()->Name, pos);
	Item* pItem2 = pet->GetEquip( pos );
	if( !pItem2 )
	{
		VisualizePetItem( pet, pos, pItem );
	}
	else
	{
		pItem2->SetCount( pItem2->GetCount() + pItem->GetCount() );
		pItem->SetOwnerGUID(GetGUID());
		pItem->SetState(ITEM_REMOVED, this);
		pItem2->SetState(ITEM_CHANGED, this);
		return pItem2;
	}
	return pItem;
}

void Player::RemoveItem( uint8 slot )
{
	// note: remove item does not actually change the item
	// it only takes the item out of storage temporarily
	Item *pItem = GetItemByPos( slot );
	if( !pItem )
		return;

	sLog.outDebug("STORAGE: RemoveItem slot = %u, item = %u", slot, pItem->GetEntry());

	if( slot < EQUIPMENT_SLOT_END )
		_ApplyItemModsFor(this, pItem, false);

	m_items[slot] = NULL;
	pItem->SetSlot( NULL_SLOT );
	pItem->SetUInt64Value( ITEM_FIELD_CONTAINED, 0 );

}

void Player::PetRemoveItem( Pet* pet, uint8 slot )
{
	// note: pet remove item does not actually change the item
	// it only takes the item out of storage temporarily
	Item *pItem = pet->GetEquip( slot );
	if( !pItem )
		return;

	sLog.outDebug("STORAGE: PetRemoveItem slot = %u, item = %u", slot, pItem->GetEntry());

	if( slot < EQUIPMENT_SLOT_END )
		_ApplyItemModsFor(pet, pItem, false);

	pet->SetEquip(slot, NULL );
	pItem->SetSlot( NULL_SLOT );
	pItem->SetUInt64Value( ITEM_FIELD_CONTAINED, 0 );

}

void Player::QuickEquipItem( uint8 pos, Item *pItem)
{
	if( pItem )
	{

		VisualizeItem( pos, pItem );

		if( IsInWorld() )
		{
			pItem->AddToWorld();
			pItem->SendUpdateToPlayer( this );
		}
	}
}

void Player::QuickPetEquipItem( Pet* pet, uint8 pos, Item *pItem )
{
	if( pItem )
	{

		VisualizePetItem( pet, pos, pItem );

		if( IsInWorld() )
		{
			pItem->AddToWorld();
			pItem->SendUpdateToPlayer( this );
		}
	}
}

void Player::VisualizeItem( uint8 pos, Item *pItem)
{
	if(!pItem)
		return;

	uint8 slot = pos;

	//sLog.outDebug( "STORAGE: EquipItem slot = %u, item = %u", slot, pItem->GetEntry());
	
	m_items[slot] = pItem;
//	SetUInt64Value( (uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2) ), pItem->GetGUID() );
	pItem->SetUInt64Value( ITEM_FIELD_CONTAINED, GetGUID() );
	pItem->SetUInt64Value( ITEM_FIELD_OWNER, GetGUID() );
	pItem->SetSlot( slot );
	pItem->SetContainer( NULL );
/*
	if( slot < EQUIPMENT_SLOT_END )
	{
		int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * 16);
		SetUInt32Value(VisibleBase, pItem->GetEntry());

		SetUInt32Value(VisibleBase + 8, uint32(pItem->GetItemRandomPropertyId()));
	}
*/

	pItem->SetState(ITEM_CHANGED, this);
}

void Player::VisualizePetItem( Pet* pet, uint8 pos, Item *pItem )
{
	if( !pItem )
		return;

	if( !pet )
		return;

	uint8 slot = pos;

	sLog.outDebug( "STORAGE: EquipPetItem slot = %u, item = %u", slot, pItem->GetEntry());

	pet->SetEquip( slot, pItem );
	pItem->SetState(ITEM_CHANGED, this);
}




// Return stored item (if stored to stack, it can diff. from pItem). And pItem can be deleted in this case.
Item* Player::BankItem( uint8 pos, Item *pItem, bool update )
{
	return StoreItem( pos, pItem, update );
}

bool Player::AddNewInventoryItem(uint32 modelid, uint32 count)
{
	uint8 dest;

	uint32 itemid = objmgr.GetItemEntryByModelId(modelid);

	Item *item;

	if( CanStoreNewItem( NULL_SLOT, dest, itemid, count, false ) == EQUIP_ERR_OK )
	{
		sLog.outDebug("STORAGE: Add new inventory item <%u>", itemid);
		item = StoreNewItem( dest, itemid, count, false );
		return true;
	}

	sLog.outDebug("STORAGE: Can not add new item <%u> to inventory", itemid);
	return false;
}

//////////////////////////////////////////////////////////////////////////////
bool Player::isBattleInProgress()
{
	return (PlayerBattleClass != NULL ? true : false);
}

void Player::LeaveBattle()
{
	Player* battleMaster = GetBattleMaster();
	if( battleMaster )
		if( battleMaster->PlayerBattleClass )
			battleMaster->PlayerBattleClass->LeaveBattle(this);

	PlayerBattleClass = NULL;
	i_battleMaster    = NULL;
}

void Player::Engage(Creature* enemy)
{
	if( isBattleInProgress() )
		return;

	PlayerBattleClass = new BattleSystem(this);
	PlayerBattleClass->Engage( this, enemy );
}

void Player::Engage(Player* ally)
{
}

Player* Player::GetBattleMaster()
{
	if( i_battleMaster )
	{
		//sLog.outDebug("PLAYER: GetBattleEngine for '%s' is self", GetName());
		return i_battleMaster;
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////////////

bool Player::CanJoinTeam()
{
	if( !isTeamLeader() )
		return false;

	return (m_team.size() < 4 ? true : false);
}

void Player::JoinTeam(Player* member)
{
	sLog.outDebug("GROUP: '%s' is joining", member->GetName());
	member->SetLeader(GetGUIDLow());
	m_team.push_back(member);

	WorldPacket data;
	data.Initialize( 0x0D );
	data << (uint8 ) 0x05;
	data << (uint32) GetAccountId(); // leader id
	data << (uint32) member->GetAccountId(); // member id

	SendMessageToSet(&data, true);

}

void Player::LeaveTeam(Player* member)
{
	sLog.outDebug("GROUP: '%s' is leaving", member->GetName());
	m_team.remove(member);
	member->SetLeader(0);

	WorldPacket data;
	data.Initialize( 0x0D );
	data << (uint8 ) 0x04;
	data << (uint32) member->GetAccountId();

	SendMessageToSet(&data, true);
}

bool Player::isTeamLeader()
{
	if( isJoinedTeam() )
		return false;

	return true;
}

bool Player::isJoinedTeam()
{
	return (m_leaderGuid ? true : false);
}

void Player::SetSubleader(uint32 acc_id)
{
	Player* sub = ObjectAccessor::FindPlayerByAccountId(acc_id);
	if( !sub )
		return;

	for(TeamList::const_iterator itr = m_team.begin(); itr != m_team.end(); ++itr)
	{
		if( sub == (*itr) )
		{
			m_subleaderGuid = sub->GetGUIDLow();
			sLog.outDebug("GROUP: Promote '%s' as Sub Leader", sub->GetName());
		}
	}
}

uint32 Player::GetTeamGuid(uint8 index) const
{
	uint8 count = 0;
	for(TeamList::const_iterator itr = m_team.begin(); itr != m_team.end(); ++itr)
	{
		count++;

		if( count == index )
			return (*itr)->GetGUIDLow();

	}

	return 0;
}
