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

#ifndef __LEGACY_SPELL_H
#define __LEGACY_SPELL_H

#include "GridDefines.h"

class WorldSession;
class Unit;
class Player;
class GameObject;

enum SpellCastTargetFlags
{
	TARGET_FLAG_HURT              = 0x8000
};

enum SpellDamageType
{
	SPELL_DAMAGE_SCHOOL           = 0, // Can be pure ATK or pure INT 
	SPELL_DAMAGE_ATK_INT          = 1, // Full ATK + 0.5 INT
	SPELL_DAMAGE_INT              = 2, // Full INT
	SPELL_DAMAGE_MECH             = 3, // Full ATK ignore DEFENSE

};

enum SpellModType
{
	SPELL_MOD_NONE                = 0,
	SPELL_MOD_BUF                 = 1,
	SPELL_MOD_HURT                = 2,
	SPELL_MOD_BUF_HURT            = (SPELL_MOD_BUF | SPELL_MOD_HURT),
	SPELL_MOD_HEAL                = 6,
};

enum SpellNumberEntry
{
	SPELL_BASIC                   = 10000,
	SPELL_BERSERKER               = 13013,
	SPELL_TEACHING                = 14027,   // Guru pembimbing
	SPELL_UNITED                  = 14028,   // Bersatu
};
/*
class SpellCastTargets
{
	public:
		SpellCastTargets() {}
		~SpellCastTargets() {}
};
*/
// from `spell_template` table
struct SpellInfo
{
	uint32 Entry;
	char*  Name;
	uint32 SP;
	uint32 Element;
	uint32 hit;
	uint32 point;
	uint32 LevelMax;
	uint32 Type;
	uint32 DamageMod;
	uint32 Reborn;
};

class Spell
{
	public:
		Spell(uint16 entry, uint8 level) : m_entry(entry), m_level(level) {}
		~Spell() {}

		uint16 GetEntry() { return m_entry; }
		uint8 GetLevel() { return m_level; }
		SpellInfo const* GetProto() const;
		bool LoadSpellFromDB();

	private:
		uint16 m_entry;
		uint8  m_level;
};

typedef std::map<uint16, Spell*> SpellMap;
#endif
