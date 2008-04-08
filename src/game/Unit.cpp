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
#include "Log.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Unit.h"
#include "Player.h"
#include "Creature.h"
#include "MapManager.h"
#include "CreatureAI.h"
#include "Pet.h"
#include "ObjectAccessor.h"
#include "MovementGenerator.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "Util.h"
#include "NameTables.h"

#include <math.h>

Unit::Unit( WorldObject *instantiator )
: WorldObject( instantiator ), i_motionMaster(this)
{
	m_objectType |= TYPE_UNIT;
	m_objectTypeId = TYPEID_UNIT;

	m_state = 0;
	m_deathState = ALIVE;
	m_levelUp = false;
}

Unit::~Unit()
{
}


Unit* Unit::GetUnit(WorldObject& object, uint64 guid)
{
	return ObjectAccessor::GetUnit(object, guid);
}

Unit* Unit::GetOwner() const
{
	uint64 ownerid = GetOwnerGUID();
	if(!ownerid)
		return NULL;
	return ObjectAccessor::GetUnit(*this, ownerid);
}

Pet* Unit::GetPet() const
{
	uint64 pet_guid = GetPetGUID();
	if(pet_guid)
	{
		Pet* pet = ObjectAccessor::GetPet(pet_guid);
		if(!pet)
		{
			sLog.outError("Unit::GetPet: Pet %u not exist.", GUID_LOPART(pet_guid));
			const_cast<Unit*>(this)->SetPet(0);
			return NULL;
		}
		return pet;
	}

	return NULL;
}

void Unit::SetPet(Pet* pet)
{
	SetUInt64Value(UNIT_FIELD_SUMMON, pet ? pet->GetGUID() : 0);

}

void Unit::Update( uint32 p_time )
{
	m_Events.Update( p_time );
	i_motionMaster.UpdateMotion( p_time );
}
/*
bool Unit::isVisibleForInState( Player const* u, bool inVisibleList ) const
{
//	return isVisibleForOrDetect(u, false, inVisibleList);
	return true;
}

bool Unit::isVisibleForOrDetect(Unit const* u, bool detect, bool inVisibleList) const
{
	if(!u)
		return false;

	// Always can see self
	if (u==this)
		return true;

	if(!IsInWorld() || !u->IsInWorld())
		return false;

	// Grid dead/alive checks
	if( u->GetTypeId()==TYPEID_PLAYER)
	{
		// if player is dead then he can't detect anyone in anycases
		if(!u->isAlive())
			detect = false;
	}
	else
	{
		// all dead creatures/players not visible for any creatures
		if(!u-isAlive() || !isAlive())
			return false;
	}

	return true;
}
*/

void Unit::setDeathState(DeathState s)
{
	m_deathState = s;
}

void Unit::StopMoving()
{
	clearUnitState(UNIT_STATE_MOVING);
	SendMonsterMove(GetPositionX(), GetPositionY(), 0);
}

void Unit::SendMonsterMove(uint16 NewPosX, uint16 NewPosY, uint32 Time)
{
	uint8 map_npcid = ((Creature*)this)->GetMapNpcId();
	//sLog.outString("SendMonsterMove for %s GUID(%u) mapnpcid %u to %u,%u", GetName(), GetGUIDLow(), map_npcid, NewPosX, NewPosY);
	WorldPacket data;
	data.Initialize( 0x16 );
	data << (uint8 ) 0x02;
	data << (uint8 ) map_npcid;
	data << (uint8 ) 0x00;
	data << (uint16) NewPosX;
	data << (uint16) NewPosY;
	SendMessageToSet(&data,false);
}

uint16 Unit::GetAttackPower()
{
	uint16 atk = GetUInt16Value(UNIT_FIELD_ATK) + 1;
	uint8  lvl = GetUInt8Value(UNIT_FIELD_LEVEL);
	return (atk + (uint16)(lvl * 0.5));
}

uint16 Unit::GetMagicPower()
{
	uint16 mag = GetUInt16Value(UNIT_FIELD_INT) + 1;
	uint8  lvl = GetUInt8Value(UNIT_FIELD_LEVEL);
	return (mag + (uint16)(lvl * 0.5));
}

uint16 Unit::GetDefensePower()
{
	uint16 def = GetUInt16Value(UNIT_FIELD_DEF);
	uint8  lvl = GetUInt8Value(UNIT_FIELD_LEVEL);
	return (def + (uint16)(lvl * 0.5));
}

bool Unit::CanHaveSpell(Spell* spell)
{
	if( !spell )
		return false;

	uint8 el = GetUInt8Value(UNIT_FIELD_ELEMENT);
	switch( el )
	{
		case ELEMENT_EARTH:
			if(spell->GetProto()->Element == ELEMENT_FIRE)
				return false;
			else
				return true;

		case ELEMENT_WATER:
			if(spell->GetProto()->Element == ELEMENT_WIND)
				return false;
			else
				return true;

		case ELEMENT_FIRE:
			if(spell->GetProto()->Element == ELEMENT_EARTH)
				return false;
			else
				return true;

		case ELEMENT_WIND:
			if(spell->GetProto()->Element == ELEMENT_WATER)
				return false;
			else
				return true;

		default:
			return true;
	}
}

bool Unit::AddSpell(uint16 entry, uint8 level)
{
	//sLog.outDebug("UNIT: '%s' adding spell <%u> level [%u]", GetName(), entry, level);

	if( !entry )
		return false;

	Spell* spell = new Spell(entry, level);
	if( !spell->LoadSpellFromDB() )
	{
		sLog.outString("UNIT: Spell not found, incorrect spell entry");
		delete spell;
		return false;
	}

	const SpellInfo * sinfo = objmgr.GetSpellTemplate(entry);

	if( !CanHaveSpell(spell) )
	{
		sLog.outString("UNIT: '%s' element %s can not have spell <%s> element %s", GetName(), LookupNameElement(GetUInt8Value(UNIT_FIELD_ELEMENT), g_elementNames), sinfo->Name, LookupNameElement(sinfo->Element, g_elementNames));
		delete spell;
		return false;
	}

	//sLog.outDebug("UNIT: '%s' has spell <%-20.20s> level [%2u]", GetName(), sinfo->Name, level);

	m_spells.insert( pair<uint16, Spell*>(entry, spell) );
	return true;
}

bool Unit::HaveSpell(uint16 entry)
{
	if(entry == SPELL_BASIC) return true; // must have basic attack spell

	if( FindSpell(entry) )
		return true;

	return false;
}

Spell* Unit::FindSpell(uint16 entry)
{
	SpellMap::iterator i = m_spells.find(entry);
	if( i != m_spells.end() )
		return i->second;

	return NULL;
}

uint8 Unit::GetSpellLevel(const SpellInfo *sinfo)
{
	if( sinfo->Entry == SPELL_BASIC )
		return 10;

	Spell* s = FindSpell(sinfo->Entry);
	if( !s )
		return 1;

	return s->GetLevel();
}

Spell* Unit::GetSpellByPos(uint8 pos)
{
	uint8 index = 0;
	for(SpellMap::iterator it = m_spells.begin(); it != m_spells.end(); ++it)
		if( index == pos )
			return (*it).second;
		else
			index++;

	return NULL;
}

uint8 Unit::GetSpellLevelByPos(uint8 pos)
{
	uint8 index = 0;
	for(SpellMap::iterator it = m_spells.begin(); it != m_spells.end(); ++it)
		if( index == pos )
			return (*it).second->GetLevel();
		else
			index++;

	return 0;
}

uint16 Unit::GetRandomSpell(uint8 ai_difficulty)
{
	if( ai_difficulty > 90 ) // extreme difficulty, Boss level
	{
	}
	else
	{
	}

	uint8 spell_cnt = m_spells.size();

	Spell* spell = GetSpellByPos(urand(0, spell_cnt));

	if( !spell )
		return SPELL_BASIC;

	return spell->GetEntry();
}

void Unit::AddKillExp(uint8 enemyLevel, bool linked)
{
	//sLog.outDebug("EXPERIENCE: '%s' add for killing enemy level %u", GetName(), enemyLevel);

	uint32 xp = GetUInt32Value(UNIT_FIELD_NEXT_LEVEL_XP);
	xp = xp + 1;

	uint8  lvl = getLevel();

	uint8  birth_lvl = GetUInt8Value(UNIT_FIELD_REBORN);

	double power = 2.9;
	switch( birth_lvl )
	{
		case 0: // normal
			power = 2.9;
			break;
		case 1: // evo
			power = 3.0;
			break;
		case 2: // revo
			power = 3.1;
			break;
		default:
			power = 2.9;
			break;
	}

	uint32 tnl = (uint32) round(pow(lvl + 1, power) + 5);

	sLog.outDebug("EXPERIENCE: '%s' XP gain %u, TNL is %u", GetName(), xp, tnl);
	//return;

	//SetUInt32Value(UNIT_FIELD_NEXT_LEVEL_XP, xp + (6 * getLevel()));
	SetUInt32Value(UNIT_FIELD_NEXT_LEVEL_XP, xp);

	if( xp >= tnl )
	{
		LevelUp();

		//tnl = (uint32) round(pow(lvl + 1, 2.9) + 5);
		SetUInt32Value(UNIT_FIELD_XP, xp);
		SetUInt32Value(UNIT_FIELD_NEXT_LEVEL_XP, 0);
	}

}

void Unit::LevelUp()
{
	uint8 lvl = getLevel() + 1;
	SetUInt8Value(UNIT_FIELD_LEVEL, lvl);
	m_levelUp = true;

	sLog.outDebug("EXPERIENCE: Player '%s' is level up to [%u] tnl now is <%u>", GetName(), lvl, (uint32) round(pow(lvl + 1, 2.9) + 5));

	uint16 spell_point = GetUInt32Value(UNIT_FIELD_SPELL_POINT);
	uint16 stat_point  = GetUInt32Value(UNIT_FIELD_STAT_POINT);

	SetUInt16Value(UNIT_FIELD_SPELL_POINT, spell_point + 2);
	SetUInt16Value(UNIT_FIELD_STAT_POINT,  stat_point  + 2);
}

bool Unit::isLevelUp()
{
	return m_levelUp;
}

void Unit::resetLevelUp()
{
	m_levelUp = false;
}

void Unit::AddKillItemDropped(uint16 itemId)
{
}

uint16 Unit::GetItemDropped(uint8 index)
{
	//return 49001;
	if( rand_chance() < 50 )
		return 26001;
	else
		return 34011;
}

uint32 Unit::GetExpGained()
{
	uint32 xp  = GetUInt32Value(UNIT_FIELD_XP);
	uint32 tnl = GetUInt32Value(UNIT_FIELD_NEXT_LEVEL_XP);
	return xp + tnl;// + (6 * ceil(getLevel() / 2.9) );
}
