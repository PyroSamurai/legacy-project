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
	m_itemSet = 0;
	m_itemSetApplied = false;
	m_tmp_xp = 0;
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

void Unit::SetLevel(uint32 lvl)
{
	SetUInt32Value(UNIT_FIELD_LEVEL, lvl);

	// group update
//	if(GetTypeId() == TYPEID_PLAYER)
//		((Player*)this)->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_LEVEL);
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
	uint16 atk = GetUInt32Value(UNIT_FIELD_ATK) + GetInt32Value(UNIT_FIELD_ATK_MOD) + 1;
	uint8  lvl = GetUInt32Value(UNIT_FIELD_LEVEL);
	return (atk + (uint16)(lvl * 0.5));
}

uint16 Unit::GetMagicPower()
{
	uint16 mag = GetUInt32Value(UNIT_FIELD_INT) + GetInt32Value(UNIT_FIELD_INT_MOD) + 1;
	uint8  lvl = GetUInt32Value(UNIT_FIELD_LEVEL);
	return (mag + (uint16)(lvl * 0.5));
}

uint16 Unit::GetDefensePower()
{
	uint16 def = GetUInt32Value(UNIT_FIELD_DEF) + GetInt32Value(UNIT_FIELD_DEF_MOD);
	uint8  lvl = GetUInt32Value(UNIT_FIELD_LEVEL);
	return (def + (uint16)(lvl * 0.5));
}

bool Unit::CanHaveSpell(Spell* spell)
{
	if( !spell )
		return false;

	uint8 el = GetUInt32Value(UNIT_FIELD_ELEMENT);
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

bool Unit::AddSpell(uint16 entry, uint8 level, SpellUpdateState state)
{
	//sLog.outDebug("UNIT: '%s' adding spell <%u> level [%u]", GetName(), entry, level);

	if( !entry )
		return false;

	const SpellInfo * sinfo = objmgr.GetSpellTemplate(entry);

	if( !sinfo )
	{
		sLog.outString("UNIT: Spell not found, incorrect spell entry");
		return false;
	}

	Spell* spell = new Spell(entry, level, state);

	if( !CanHaveSpell(spell) )
	{
		//sLog.outString("UNIT: '%s' element %s can not have spell <%s> element %s", GetName(), LookupNameElement(GetUInt32Value(UNIT_FIELD_ELEMENT), g_elementNames), sinfo->Name, LookupNameElement(sinfo->Element, g_elementNames));
		delete spell;
		return false;
	}

	//sLog.outDebug("UNIT: '%s' has spell <%-20.20s> level [%2u]", GetName(), sinfo->Name, level);

	m_spells.insert( pair<uint16, Spell*>(entry, spell) );
	return true;
}

void Unit::SetSpellLevel(uint16 entry, uint8 level)
{
	if(!entry || !level) return;

	Spell* spell = FindSpell(entry);
	
	if(!spell) return;

	sLog.outDebug("UNIT SPELL: Change spell level to %u", level);
	spell->SetLevel(level);
	spell->SetState(SPELL_CHANGED);
}

void Unit::AddSpellLevel(uint16 entry, uint8 value)
{
	if(!entry || !value) return;

	Spell* spell = FindSpell(entry);

	if(!spell) return;

	spell->AddLevel(value);
	spell->SetState(SPELL_CHANGED);
}

bool Unit::HaveSpell(uint16 entry)
{
	// must have basic spell
	if( entry == SPELL_BASIC ||
		entry == SPELL_DEFENSE ||
		entry == SPELL_CATCH ||
		entry == SPELL_CATCH2 ||
		entry == SPELL_ESCAPE )
		return true;

	if( FindSpell(entry) )
		return true;

	return false;
}

bool Unit::isSpellLevelMaxed(uint16 entry)
{
	if( !entry || entry == SPELL_BASIC ) return true;

	Spell* spell = FindSpell(entry);

	if( !spell ) return true;

	if( spell->GetLevel() >= spell->GetProto()->LevelMax )
		return true;

	return false;
}

uint32 Unit::GetSpellLearnPoint(uint16 entry)
{
	const SpellInfo* sinfo = objmgr.GetSpellTemplate(entry);

	if( !sinfo )
		return 0;

	if( GetUInt32Value(UNIT_FIELD_ELEMENT) == sinfo->Element )
		return sinfo->LearnPoint;

	return sinfo->LearnPoint * 2;
}

Spell* Unit::FindSpell(uint16 entry)
{
	SpellMap::iterator i = m_spells.find(entry);
	if( i != m_spells.end() )
		return i->second;

	return NULL;
}

uint8 Unit::GetSpellLevel(uint16 entry)
{
	if( entry == SPELL_BASIC )
		return 1;

	Spell* s = FindSpell(entry);
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

void Unit::AddKillExp(uint8 enemyLevel, bool linked, bool inTeam, uint8 lowestLevelInGroup, uint8 lowerLevelCount)
{
	//sLog.outDebug("EXPERIENCE: '%s' add for killing enemy level %u", GetName(), enemyLevel);

	uint8  level   = getLevel();
	float  xp      = 0;

	int32 diffLevel = enemyLevel - level;
	if(level - enemyLevel > 15)
		///- Unit level too high to gained more experience from enemy
		xp += 0;
	else if( linked )
		///- add 10% bonus for linked attack
		xp += (diffLevel > 0 ? diffLevel : 2) + ((diffLevel > 0 ? diffLevel : 2) * 0.1);
	else
		///- single attack, no bonus
		xp += (diffLevel > 0 ? diffLevel : 2);

	if( HaveSpell(SPELL_UNITED) && isType(TYPE_PLAYER) && inTeam )
	{
		///- 10% bonus for SPELL_UNITED if battle in group
		xp += (xp * 0.1);

		uint8 diff = abs(level - lowestLevelInGroup);
		if( HaveSpell(SPELL_TEACHING) && inTeam && diff > 10 )
		{
			///- 75% bonus from level diff * apprentince count, SPELL_TEACHING
			//   if assist > 10 level lower player
			xp += diff * 0.75 * lowerLevelCount;
		}
	}

	float  rate_xp   = sWorld.getRate(RATE_XP_KILL);
	uint32 xp_gained = (uint32) round(xp * rate_xp);
	AddExpGained(xp_gained);
}

void Unit::AddHitExp(uint8 enemyLevel, bool linked)
{
	uint8 level = getLevel();
	int32 diffLevel = enemyLevel - level;
	if(level - enemyLevel > 15)
		///- Unit level too high to gained more experience from enemy
		return;

	float rate_xp = sWorld.getRate(RATE_XP_KILL);
	uint32 xp_gained = (uint32) round(1 * rate_xp);

	///- this unit already have exp either from kill or hit
	//   ignoring any incoming hit experience
	if( m_tmp_xp )
		return;

	AddExpGained(xp_gained);
}

void Unit::AddExpGained(int32 xp)
{
	///- For reseting purpose, ex: when revived after killed 
	if( xp == 0 )
	{
		m_tmp_xp = 0;
		return;
	}

	///- For removing purpose, ex: when killed
	if( xp < 0 )
	{
		///- TODO: Fix lost experience when killed
		m_tmp_xp = xp * int32(sWorld.getRate(RATE_XP_LOST));
		return;
	}

	m_tmp_xp += xp;
}

void Unit::AddExpGained()
{
	int32 xp = m_tmp_xp;

	m_tmp_xp = 0;

	uint8  lvl = getLevel();

	uint8  birth_lvl = GetUInt32Value(UNIT_FIELD_REBORN);

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

	sLog.outDebug("EXPERIENCE: '%s' XP gain %u, TNL is %u to %u", GetName(), xp, GetUInt32Value(UNIT_FIELD_NEXT_LEVEL_XP) + xp, tnl);

	xp = GetUInt32Value(UNIT_FIELD_NEXT_LEVEL_XP) + xp;

	///- losing experience if get killed
	if( xp < 0 )
		xp = 0;

	SetUInt32Value(UNIT_FIELD_NEXT_LEVEL_XP, xp);

	while( xp >= tnl )
	{
		LevelUp();

		SetUInt32Value(UNIT_FIELD_XP, GetUInt32Value(UNIT_FIELD_XP) + tnl);
		SetUInt32Value(UNIT_FIELD_NEXT_LEVEL_XP, xp - tnl);

		tnl = (uint32) round(pow(getLevel() + 1, power) + 5);

		if( getLevel() >= 200 )
			break;
	}
}

void Unit::LevelUp()
{
	uint8 lvl = getLevel() + 1;
	SetUInt32Value(UNIT_FIELD_LEVEL, lvl);
	m_levelUp = true;

	sLog.outDebug("EXPERIENCE: Player '%s' is level up to [%u] tnl now is <%u>", GetName(), lvl, (uint32) round(pow(lvl + 1, 2.9) + 5));

	if(isType(TYPE_PLAYER))
	{
		ApplyModUInt32Value(UNIT_FIELD_SPELL_POINT, 1, true);
		ApplyModUInt32Value(UNIT_FIELD_STAT_POINT,  2, true);
	}
	else if(isType(TYPE_PET))
	{
		///- Check for Pet for maxed out spells
		SpellMap::iterator it = m_spells.begin();
		bool maxed = true;
		for(it; it != m_spells.end(); ++it)
		{
			if( !isSpellLevelMaxed(it->second->GetEntry()) )
			{
				maxed = false;
				break;
			}
		}
		if( !maxed )
			ApplyModUInt32Value(UNIT_FIELD_SPELL_POINT, 1, true);

		uint8 loyalty = ((Pet*)this)->GetLoyalty();
		if( loyalty < 100 )
			((Pet*)this)->SetLoyalty(loyalty + 1);

		///- Random stat addition.
		double dice_stat = rand_chance();
		if ( dice_stat < 20 )
			ApplyModUInt32Value(UNIT_FIELD_INT, 1, true);
		else if( dice_stat < 40 )
			ApplyModUInt32Value(UNIT_FIELD_ATK, 1, true);
		else if( dice_stat < 60 )
			ApplyModUInt32Value(UNIT_FIELD_DEF, 1, true);
		else if( dice_stat < 80 )
			ApplyModUInt32Value(UNIT_FIELD_HPX, 1, true);
		else if( dice_stat < 90 )
			ApplyModUInt32Value(UNIT_FIELD_SPX, 1, true);
		else
			ApplyModUInt32Value(UNIT_FIELD_AGI, 1, true);
	}

	SetUInt32Value(UNIT_FIELD_HP_MAX, GetHPMax());
	SetUInt32Value(UNIT_FIELD_HP, GetHPMax());
	SetUInt32Value(UNIT_FIELD_SP_MAX, GetSPMax());
	SetUInt32Value(UNIT_FIELD_SP, GetSPMax());
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
	///- TODO: Chance for elementary Creature above level 15 to drop herald item
	if( getLevel() >= 15 && GetUInt32Value(UNIT_FIELD_ELEMENT) )
	{
		double dice_herald = rand_chance();
		if( dice_herald > 59.91 )
		{
			switch( GetUInt32Value(UNIT_FIELD_ELEMENT) )
			{
			case ELEMENT_EARTH: return ITEM_ROCKY_GOLEM;
			case ELEMENT_WATER: return ITEM_WATER_GODDES;
			case ELEMENT_FIRE:  return ITEM_PHOENIX;
			case ELEMENT_WIND:  return ITEM_GREEN_DRAGON;
			}
		}
	}

	CreatureInfo const* cinfo = objmgr.GetCreatureTemplate(GetEntry());

	if( !cinfo )
		return 0;

	double dice = rand_chance();
	double drop_rate = 100 + (index * 5) - (33.33 * sWorld.getRate(RATE_DROP_ITEMS));

	//sLog.outDebug("ITEM DROPPED: Chance for item to drop is %-3.2f versus rate %-3.2f", dice, drop_rate);
	if( dice < drop_rate )
		return 0;

	switch( index )
	{
		case 0: return cinfo->drop1;
		case 1: return cinfo->drop2;
		case 2: return cinfo->drop3;
		case 3: return cinfo->drop4;
		case 4: return cinfo->drop5;
		case 5: return cinfo->drop6;
		case 6: return cinfo->drop7;
		case 7: return cinfo->drop8;
		case 8: return cinfo->drop9;
		case 9: return cinfo->drop10;
		default: return 0;
	}
}

uint32 Unit::GetExpGained()
{
	uint32 xp  = GetUInt32Value(UNIT_FIELD_XP);
	uint32 tnl = GetUInt32Value(UNIT_FIELD_NEXT_LEVEL_XP);
	return xp + tnl + 6;// + (6 * ceil(getLevel() / 2.9) );
}

uint16 Unit::GetHPMax() const
{
	uint8  level   = GetUInt32Value(UNIT_FIELD_LEVEL);
	uint16 hpx     = GetUInt32Value(UNIT_FIELD_HPX);
	uint16 hpx_mod = GetInt32Value(UNIT_FIELD_HPX_MOD);
	uint16 hp_max  = 80 + level;

	//uint16 hp_max  = ((hpx + hpx_mod) * 4) + 80 + level;

	///- TODO: Fix this formula
	hp_max += uint16(hpx * (4 * round(pow(level, 0.25))));

	//sLog.outDebug("UNIT: Calculating MAX HP level %u hpx %u hpx_mod %u = %u", level, hpx, hpx_mod, hp_max);

	return hp_max + hpx_mod;
}

uint16 Unit::GetSPMax() const
{
	uint8  level   = GetUInt32Value(UNIT_FIELD_LEVEL);
	uint16 spx     = GetUInt32Value(UNIT_FIELD_SPX);
	uint16 spx_mod = GetInt32Value(UNIT_FIELD_SPX_MOD);
	uint16 sp_max  = 60 + level;

	//uint16 sp_max  = ((spx + spx_mod) * 2) + 60 + level;

	///- TODO: Fix this formula
	sp_max += uint16(spx * (2 * round(pow(level, 0.25))));

	//sLog.outDebug("UNIT: Calculating MAX SP level %u spx %u spx_mod %u = %u", level, spx, spx_mod, sp_max);

	return sp_max;
}
