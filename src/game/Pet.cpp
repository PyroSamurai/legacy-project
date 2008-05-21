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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "ObjectMgr.h"
#include "Pet.h"
#include "MapManager.h"
#include "Unit.h"

Pet::Pet(WorldObject *instantiator, PetType type) : Creature( instantiator )
{
	m_objectType |= TYPE_PET;
	m_objectTypeId = TYPEID_PET;

	m_valuesCount = PLAYER_END;

	memset(m_items, 0, sizeof(Item*)*MAX_PET_ITEMS);

	uState = PET_NEW;
	uQueuePos = -1;
}

Pet::~Pet()
{
}

bool Pet::LoadPetFromDB(Unit* owner, uint32 guid)
{

	uint32 owner_id   = owner->GetGUIDLow();
	uint64 owner_guid = owner->GetGUID();

	QueryResult *result = CharacterDatabase.PQuery("SELECT * FROM character_pet WHERE owner_guid = '%u' AND guid = '%u'", owner_id, guid);

	if(!result)
		return false;

	// cleanup inventory related item value fields (its will be filled correctlyy in Player::_LoadInventory
	for(uint8 slot = 0; slot < MAX_PET_ITEMS; slot++)
	{
		if( m_items[slot] )
		{
			delete m_items[slot];
			m_items[slot] = NULL;
		}
	}

	Field *f = result->Fetch();
	CreatureInfo cinfo;

	if(!LoadValues(f[3].GetString()))
	{
		sLog.outError("ERROR: Pet #%d have broken data in `data` field. Can't be loaded.", guid);
		return false;
	}

	CreatureInfo const *petProto = objmgr.GetCreatureTemplate(GetEntry());

	if(!petProto)
		return false;

	cinfo.Entry    = GetEntry(); //f[1].GetUInt32();
	cinfo.modelid  = GetUInt32Value(UNIT_FIELD_DISPLAYID); //f[2].GetUInt16();
	cinfo.hp       = GetUInt32Value(UNIT_FIELD_HP); //f[7].GetUInt16();
	cinfo.sp       = GetUInt32Value(UNIT_FIELD_SP); //f[8].GetUInt16();

	cinfo.stat_int = GetUInt32Value(UNIT_FIELD_INT); //f[9].GetUInt16();
	cinfo.stat_atk = GetUInt32Value(UNIT_FIELD_ATK); //f[10].GetUInt16();
	cinfo.stat_def = GetUInt32Value(UNIT_FIELD_DEF); //f[11].GetUInt16();
	cinfo.stat_hpx = GetUInt32Value(UNIT_FIELD_HPX); //f[12].GetUInt16();
	cinfo.stat_spx = GetUInt32Value(UNIT_FIELD_SPX); //f[13].GetUInt16();
	cinfo.stat_agi = GetUInt32Value(UNIT_FIELD_AGI); //f[14].GetUInt16();
	cinfo.level    = getLevel(); //f[15].GetUInt8();
	cinfo.element  = GetUInt32Value(UNIT_FIELD_ELEMENT); //f[16].GetUInt8();

	cinfo.spell1   = petProto->spell1;
	cinfo.spell2   = petProto->spell2;
	cinfo.spell3   = petProto->spell3;
	cinfo.spell4   = petProto->spell4;
	cinfo.spell5   = petProto->spell5;

	if(!Create(guid, cinfo.Entry, &cinfo))
		return false;

	// overwrite possible wrong/corrupted guid
	SetUInt64Value(OBJECT_FIELD_GUID, MAKE_GUID(guid, HIGHGUID_UNIT));

	if(owner_guid != 0)
		SetOwnerGUID(owner_guid);

	SetName(f[2].GetCppString().c_str());
	SetUInt64Value(UNIT_FIELD_SUMMONEDBY, owner->GetGUID());

	owner->SetPet(this);

	SetLoyalty(f[4].GetUInt8());

	SetSpellLevel(cinfo.spell1, f[5].GetUInt8());
	SetSpellLevel(cinfo.spell2, f[6].GetUInt8());
	SetSpellLevel(cinfo.spell3, f[7].GetUInt8());
	SetSpellLevel(cinfo.spell4, f[8].GetUInt8());
	SetSpellLevel(cinfo.spell5, f[9].GetUInt8());

	SetBattle(f[12].GetUInt8());

	SetUInt32Value(UNIT_NPC_FLAGS, 0);

	///- Default MAX HP & SP before hpx/spx mod applied
	SetUInt32Value(UNIT_FIELD_HP_MAX, GetHPMax());
	SetUInt32Value(UNIT_FIELD_SP_MAX, GetSPMax());

	delete result;

	return true;
}

///- Create from default prototype
bool Pet::Create(uint32 guidlow, uint32 entry, Player* owner)
{
	sLog.outDebug("PET: Create guidlow %u, entry %u, owner %u", guidlow, entry, owner->GetGUID());

	CreatureInfo const *petProto = objmgr.GetCreatureTemplate(entry);

	if(!petProto)
		return false;

	if( !Create(guidlow, entry, petProto) )
		return false;

	///- Reset all to default prototype
	SetName(petProto->Name);

	SetUInt64Value(UNIT_FIELD_SUMMONEDBY, owner->GetGUID());
	SetUInt32Value(UNIT_NPC_FLAGS, 0);

	SetUInt32Value(UNIT_FIELD_HP, GetHPMax());
	SetUInt32Value(UNIT_FIELD_HP_MAX, GetHPMax());

	SetUInt32Value(UNIT_FIELD_SP, GetSPMax());
	SetUInt32Value(UNIT_FIELD_SP_MAX, GetSPMax());

	SetUInt32Value(UNIT_FIELD_LEVEL, 1);
	SetUInt32Value(UNIT_FIELD_XP, 0);
	SetUInt32Value(UNIT_FIELD_NEXT_LEVEL_XP, 0);

	return true;
}

///- Create from database
bool Pet::Create(uint32 guidlow, uint32 entry, const CreatureInfo *cinfo)
{
	Object::_Create(guidlow, HIGHGUID_UNIT);

	SetUInt32Value(OBJECT_FIELD_ENTRY, entry);
	SetUInt32Value(UNIT_FIELD_DISPLAYID, cinfo->modelid);

	SetUInt32Value(UNIT_FIELD_HP, cinfo->hp);
	SetUInt32Value(UNIT_FIELD_SP, cinfo->sp);
	SetUInt32Value(UNIT_FIELD_INT, cinfo->stat_int);
	SetUInt32Value(UNIT_FIELD_ATK, cinfo->stat_atk);
	SetUInt32Value(UNIT_FIELD_DEF, cinfo->stat_def);
	SetUInt32Value(UNIT_FIELD_HPX, cinfo->stat_hpx);
	SetUInt32Value(UNIT_FIELD_SPX, cinfo->stat_spx);
	SetUInt32Value(UNIT_FIELD_AGI, cinfo->stat_agi);

	SetUInt32Value(UNIT_FIELD_LEVEL, cinfo->level);
	SetUInt32Value(UNIT_FIELD_ELEMENT, cinfo->element);

	AddSpell(cinfo->spell1, 1, SPELL_UNCHANGED);
	AddSpell(cinfo->spell2, 1, SPELL_UNCHANGED);
	AddSpell(cinfo->spell3, 1, SPELL_UNCHANGED);
	AddSpell(cinfo->spell4, 1, SPELL_UNCHANGED);
	AddSpell(cinfo->spell5, 1, SPELL_UNCHANGED);

	return true;
}

void Pet::SetState(PetUpdateState state, Player *forplayer)
{
	if( uState == PET_NEW && state == PET_REMOVED)
	{
		// pretend the pet never existed
		RemoveFromUpdateQueueOf(forplayer);
		delete this;
		return;
	}

	if(state != PET_UNCHANGED)
	{
		// new pets must stay in the new state until saved
		if( uState != PET_NEW ) uState = state;
		AddToUpdateQueueOf(forplayer);
	}
	else
	{
		// unset in queue
		// the pet must be removed from the queue manually
		uQueuePos = -1;
		uState = PET_UNCHANGED;
	}
}

void Pet::AddToUpdateQueueOf(Player *player)
{
	if(IsInUpdateQueue()) return;

	if(!player)
	{
		player = (Player*) GetOwner();
		if(!player)
		{
			sLog.outError("Pet::AddToUpdateQueueOf - GetPlayer didn't find a player matching owner's guid (%u)!", GUID_LOPART(GetOwnerGUID()));
			return;
		}
	}

	if( player->GetGUID() != GetOwnerGUID())
	{
		sLog.outError("Pet::AddToUpdateQueueOf - Owner's guid (%u) and player's guid (%u) don't match!", GUID_LOPART(GetOwnerGUID()), player->GetGUIDLow());
		return;
	}

	if(player->m_petUpdateQueueBlocked) return;

	player->m_petUpdateQueue.push_back(this);
	uQueuePos = player->m_petUpdateQueue.size()-1;
	sLog.outDebug("Pet::AddToUpdateQueueOf - Owner: %s, Pet: %s add to update queue %i", player->GetName(), GetName(), uQueuePos);
}

void Pet::RemoveFromUpdateQueueOf(Player *player)
{
	if(!IsInUpdateQueue()) return;

	if(!player)
	{
		player = (Player*) GetOwner();
		if(!player)
		{
			sLog.outError("Pet::RemoveFromUpdateQueueOf - GetPlayer didn't find a player matching owner's guid (%u)!", GUID_LOPART(GetOwnerGUID()));
			return;
		}
	}

	if( player->GetGUID() != GetOwnerGUID())
	{
		sLog.outError("Pet::RemoveFromUpdateQueueOf = Owner's guid (%u) and player's guid (%u) don't match!", GUID_LOPART(GetOwnerGUID()), player->GetGUIDLow());
		return;
	}

	if( player->m_petUpdateQueueBlocked) return;

	player->m_petUpdateQueue[uQueuePos] = NULL;
	uQueuePos = -1;
}

void Pet::SaveToDB()
{
	uint32 guid = GetGUIDLow();

	switch (uState)
	{
		case PET_NEW:
		{
			CharacterDatabase.PExecute("DELETE FROM character_pet WHERE guid = %u", guid);

			std::ostringstream ss;
			ss << "INSERT INTO character_pet (guid, owner_guid, name, data, loyalty, spell1_level, spell2_level, spell3_level, spell4_level, spell5_level, petslot, mode_battle) VALUES ("
				<< guid << ", " << GUID_LOPART(GetOwnerGUID()) << ", '"
				<< m_name.c_str() << "', '";

			for(uint16 i = 0; i < m_valuesCount; i++)
			{
				ss << GetUInt32Value(i) << " ";
			}

			ss << "', " << uint32(m_loyalty) << ", ";

			for(uint8 i = 0; i < CREATURE_MAX_SPELLS; i++)
			{
				ss << uint32(GetSpellLevelByPos(i)) << ", ";
			}

			ss << uint32(GetSlot()) << ", " << uint32(isBattle() ? 1 : 0)
				<< " )";

			CharacterDatabase.Execute( ss.str().c_str() );
		} break;

		case PET_CHANGED:
		{
			std::ostringstream ss;
			ss << "UPDATE character_pet SET owner_guid = "
				<< GUID_LOPART(GetOwnerGUID()) << ", data = '";
			
			uint16 i;
			for( i = 0; i < m_valuesCount; i++ )
			{
				ss << GetUInt32Value(i) << " ";
			}

			ss << "', ";

			for(uint8 i = 0; i < CREATURE_MAX_SPELLS; i++)
			{
				ss << "spell" << uint32(i+1) << "_level = "
					<< uint32(GetSpellLevelByPos(i)) << ", ";
			}

			ss << " mode_battle = " << uint32(isBattle() ? 1 : 0);
			ss << " WHERE guid = " << guid;
			CharacterDatabase.Execute( ss.str().c_str() );
		} break;

		case PET_REMOVED:
		{
			CharacterDatabase.PExecute("DELETE FROM character_pet WHERE guid = %u", guid);
			delete this;
			return;
		};

		case PET_UNCHANGED:
			break;
	}
	SetState(PET_UNCHANGED);
}

void Pet::DeleteFromDB()
{
}

void Pet::SetEquip(uint8 slot, Item *pItem)
{
	if(!pItem)
	{
		///- if !pItem means unequip previous equipment
		m_items[slot] = pItem;
		return;
	}

	///- Equip if equipment slot is empty, otherwise abort
	if(m_items[slot])
		return;

	sLog.outDebug( "STORAGE: PetEquipItem slot = %u, item = %u", slot, pItem->GetEntry());

	m_items[slot] = pItem;

//	SetUInt64Value( (uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2) ), pItem->GetGUID() );
	pItem->SetUInt64Value( ITEM_FIELD_CONTAINED, GetGUID() );
	pItem->SetUInt64Value( ITEM_FIELD_OWNER, GetOwnerGUID() );
	pItem->SetSlot( slot );
	pItem->SetContainer( this );
/*
	if( slot < EQUIPMENT_SLOT_END )
	{
		int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * 16);
		SetUInt32Value(VisibleBase, pItem->GetEntry());
	}
*/
}

uint16 Pet::GetEquipModelId(uint8 slot) const
{
	if( m_items[slot] )
		return m_items[slot]->GetModelId();

	return 0;
}

void Pet::DumpPet()
{
	for(uint8 slot = EQUIPMENT_SLOT_START; slot < MAX_PET_ITEMS; slot++)
	{
		if(m_items[slot])
			sLog.outDebug(" @@ Pet '%s' Equipment slot %2u equiped [%-20.20s] (%u piece)", GetName(), slot, m_items[slot]->GetProto()->Name, m_items[slot]->GetCount());
	}

	//sLog.outDebug(" ** Pet Spell Size %u", m_spells.size());
	for(SpellMap::iterator it = m_spells.begin(); it != m_spells.end(); ++it)
		sLog.outDebug( " ** Pet '%s' has spell <%-20.20s> level [%2u]", GetName(), (*it).second->GetProto()->Name, (*it).second->GetLevel());
}

uint32 Pet::GetOwnerAccountId() const
{
	uint64 owner_guid = GetUInt64Value(UNIT_FIELD_SUMMONEDBY);
	Player* owner = ObjectAccessor::FindPlayer(owner_guid);
	if( !owner )
		return 0;

	return owner->GetAccountId();
}
