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

	Field *f = result->Fetch();
	CreatureInfo cinfo;

	cinfo.Entry = f[1].GetUInt32();
	cinfo.modelid = f[2].GetUInt16();
	cinfo.Name  = (char*) f[4].GetCppString().c_str();;
	cinfo.hp    = f[6].GetUInt16();
	cinfo.sp    = f[7].GetUInt16();

	cinfo.stat_int = f[8].GetUInt16();
	cinfo.stat_atk = f[9].GetUInt16();
	cinfo.stat_def = f[10].GetUInt16();
	cinfo.stat_hpx = f[11].GetUInt16();
	cinfo.stat_spx = f[12].GetUInt16();
	cinfo.stat_agi = f[13].GetUInt16();
	cinfo.level    = f[14].GetUInt8();
	cinfo.element  = f[15].GetUInt8();

	if(!Create(guid, cinfo.Entry, &cinfo))
		return false;

	SetUInt64Value(UNIT_FIELD_SUMMONEDBY, owner->GetGUID());
	SetUInt32Value(UNIT_NPC_FLAGS, 0);

	SetUInt16Value(UNIT_FIELD_EQ_HEAD,      f[27].GetUInt16());
	SetUInt16Value(UNIT_FIELD_EQ_BODY,      f[28].GetUInt16());
	SetUInt16Value(UNIT_FIELD_EQ_WRIST,     f[29].GetUInt16());
	SetUInt16Value(UNIT_FIELD_EQ_WEAPON,    f[30].GetUInt16());
	SetUInt16Value(UNIT_FIELD_EQ_SHOE,      f[31].GetUInt16());
	SetUInt16Value(UNIT_FIELD_EQ_SPECIAL,   f[32].GetUInt16());

	///- TODO: Calculate MAX HP & SP after hpx/spx mod applied
	SetUInt16Value(UNIT_FIELD_HP_MAX, cinfo.hp);
	SetUInt16Value(UNIT_FIELD_SP_MAX, cinfo.sp);

	owner->SetPet(this);

	SetSkillPoint(f[16].GetUInt16());
	SetStatPoint(f[17].GetUInt16());
	SetLoyalty(f[18].GetUInt8());

	SetSkill(0, f[19].GetUInt16());
	SetSkill(1, f[20].GetUInt16());
	SetSkill(2, f[21].GetUInt16());
	SetSkill(3, f[22].GetUInt16());

	SetSkillLevel(0, f[23].GetUInt8());
	SetSkillLevel(1, f[24].GetUInt8());
	SetSkillLevel(2, f[25].GetUInt8());
	SetSkillLevel(3, f[26].GetUInt8());

	delete result;
	sLog.outDebug("New Pet has guid %u", GetGUIDLow());


	return true;
}

bool Pet::Create(uint32 guidlow, uint32 Entry, const CreatureInfo *cinfo)
{
	Object::_Create(guidlow, HIGHGUID_UNIT);

	SetUInt32Value(OBJECT_FIELD_ENTRY, Entry);
	SetUInt16Value(UNIT_FIELD_DISPLAYID, cinfo->modelid);

	SetName(cinfo->Name);
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

