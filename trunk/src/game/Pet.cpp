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
}

Pet::~Pet()
{
}

bool Pet::LoadPetFromDB(Unit* owner, uint32 guid)
{

	uint32 ownerid = owner->GetGUIDLow();

	QueryResult *result = CharacterDatabase.PQuery("SELECT * FROM character_pet WHERE owner = '%u' AND id = '%u'", ownerid, guid);

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


	cinfo.Entry    = f[1].GetUInt32();
	cinfo.modelid  = f[2].GetUInt16();
	cinfo.hp       = f[7].GetUInt16();
	cinfo.sp       = f[8].GetUInt16();


	cinfo.stat_int = f[9].GetUInt16();
	cinfo.stat_atk = f[10].GetUInt16();
	cinfo.stat_def = f[11].GetUInt16();
	cinfo.stat_hpx = f[12].GetUInt16();
	cinfo.stat_spx = f[13].GetUInt16();
	cinfo.stat_agi = f[14].GetUInt16();
	cinfo.level    = f[15].GetUInt8();
	cinfo.element  = f[16].GetUInt8();

	if(!Create(guid, cinfo.Entry, &cinfo))
		return false;

	SetName(f[4].GetCppString().c_str());
	SetUInt64Value(UNIT_FIELD_SUMMONEDBY, owner->GetGUID());
	SetUInt32Value(UNIT_NPC_FLAGS, 0);

	///- TODO: Calculate MAX HP & SP after hpx/spx mod applied
	SetUInt16Value(UNIT_FIELD_HP_MAX, cinfo.hp);
	SetUInt16Value(UNIT_FIELD_SP_MAX, cinfo.sp);

	owner->SetPet(this);

	SetUInt32Value(UNIT_FIELD_XP, f[5].GetUInt32());
	SetUInt32Value(UNIT_FIELD_NEXT_LEVEL_XP, f[6].GetUInt32());

	SetSkillPoint(f[17].GetUInt16());
	SetStatPoint (f[18].GetUInt16());
	SetLoyalty   (f[19].GetUInt8());

	/*
	SetSkill(0, f[19].GetUInt16());
	SetSkill(1, f[20].GetUInt16());
	SetSkill(2, f[21].GetUInt16());
	SetSkill(3, f[22].GetUInt16());

	SetSkillLevel(0, f[23].GetUInt8());
	SetSkillLevel(1, f[24].GetUInt8());
	SetSkillLevel(2, f[25].GetUInt8());
	SetSkillLevel(3, f[26].GetUInt8());
	*/

	AddSpell(f[20].GetUInt16(), f[24].GetUInt8());
	AddSpell(f[21].GetUInt16(), f[25].GetUInt8());
	AddSpell(f[22].GetUInt16(), f[26].GetUInt8());
	AddSpell(f[23].GetUInt16(), f[27].GetUInt8());

	SetBattle(f[30].GetUInt8());

	delete result;
	//sLog.outDebug("New Pet has guid %u", GetGUIDLow());

	return true;
}

bool Pet::Create(uint32 guidlow, uint32 Entry, const CreatureInfo *cinfo)
{
	Object::_Create(guidlow, HIGHGUID_UNIT);

	SetUInt32Value(OBJECT_FIELD_ENTRY, Entry);
	SetUInt16Value(UNIT_FIELD_DISPLAYID, cinfo->modelid);

	SetUInt16Value(UNIT_FIELD_HP, cinfo->hp);
	SetUInt16Value(UNIT_FIELD_SP, cinfo->sp);
	SetUInt16Value(UNIT_FIELD_INT, cinfo->stat_int);
	SetUInt16Value(UNIT_FIELD_ATK, cinfo->stat_atk);
	SetUInt16Value(UNIT_FIELD_DEF, cinfo->stat_def);
	SetUInt16Value(UNIT_FIELD_HPX, cinfo->stat_hpx);
	SetUInt16Value(UNIT_FIELD_SPX, cinfo->stat_spx);
	SetUInt16Value(UNIT_FIELD_AGI, cinfo->stat_agi);


	SetUInt8Value(UNIT_FIELD_LEVEL, cinfo->level);
	SetUInt8Value(UNIT_FIELD_ELEMENT, cinfo->element);

	return true;
}

void Pet::SetEquip(uint8 slot, Item *pItem, bool swap)
{
	if(!pItem)
		return;

	sLog.outDebug( "STORAGE: PetEquipItem slot = %u, item = %u", slot, pItem->GetEntry());

	if( !m_items[slot] )
		m_items[slot] = pItem;

	if( swap || m_items[slot] )
	{
		Item* pItem2 = m_items[slot];
		m_items[slot] = pItem;
		pItem = pItem2;
	}
	SetUInt64Value( (uint16)(PLAYER_FIELD_INV_SLOT_HEAD + (slot * 2) ), pItem->GetGUID() );
	pItem->SetUInt64Value( ITEM_FIELD_CONTAINED, GetGUID() );
	pItem->SetUInt64Value( ITEM_FIELD_OWNER, GetOwnerGUID() );
	pItem->SetSlot( slot );
	pItem->SetContainer( NULL );

	if( slot < EQUIPMENT_SLOT_END )
	{
		int VisibleBase = PLAYER_VISIBLE_ITEM_1_0 + (slot * 16);
		SetUInt32Value(VisibleBase, pItem->GetEntry());
	}
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
			sLog.outDebug(" @@ Pet '%s' Equipment slot %3u equiped '%s' (%u piece)", GetName(), slot, m_items[slot]->GetProto()->Name, m_items[slot]->GetCount());
	}

	sLog.outDebug(" ** Pet Spell Size %u", m_spells.size());
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
