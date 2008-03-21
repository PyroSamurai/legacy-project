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

#ifndef __LEGACY_UNIT_H
#define __LEGACY_UNIT_H

#include "Common.h"
#include "Object.h"
#include "Opcodes.h"
#include "Util.h"
#include "SharedDefines.h"
#include "Utilities/EventProcessor.h"
#include "MotionMaster.h"
#include <list>

class Creature;
class GameObject;
class Item;
class Pet;

enum DeathState
{
	ALIVE     = 0,
	JUST_DIED = 1,
	DEAD      = 2
};

enum UnitState
{
	UNIT_STATE_STOPPED   = 0,
	UNIT_STATE_DIED      = 1,
	UNIT_STATE_IN_BATTLE = 2,
	UNIT_STATE_STUNED    = 4,
	UNIT_STATE_CYCLONED  = 6,
	UNIT_STATE_ROOT      = 8,
	UNIT_STATE_DISABLED  = (UNIT_STATE_STUNED | UNIT_STATE_CYCLONED | UNIT_STATE_ROOT),
	UNIT_STATE_CONFUSED  = 32,
	UNIT_STATE_ROAMING   = 64,
	UNIT_STATE_CHASE     = 128,
	UNIT_STATE_SEARCHING = 256,
	UNIT_STATE_FLEEING   = 512,
	UNIT_STATE_MOVING    = (UNIT_STATE_ROAMING | UNIT_STATE_CHASE | UNIT_STATE_SEARCHING | UNIT_STATE_FLEEING),
	UNIT_STATE_FOLLOW    = 1024,
	UNIT_STATE_ALL_STATE = 0xFFFF
};

#define CREATURE_MAX_SPELLS 4

class LEGACY_DLL_SPEC Unit : public WorldObject
{
	public:



		virtual ~Unit();

		virtual void Update( uint32 time );

		static Unit* GetUnit(WorldObject& object, uint32 accountId);

		void SendMonsterMove(uint16 NewPosX, uint16 NewPosY, uint32 Time);

		bool isAlive() const { return (m_deathState == ALIVE); }
		bool isDead() const { return (m_deathState == DEAD); }

		DeathState getDeathState() { return m_deathState; }
		virtual void setDeathState(DeathState s); // overwrited in Creature/Player/Pet

		void addUnitState(uint32 f) { m_state |= f; }
		bool hasUnitState(const uint32 f) const { return (m_state & f); }
		void clearUnitState(uint32 f) { m_state &= ~f; }
		bool isDisabled() const
		{
			return hasUnitState(
					UNIT_STATE_CONFUSED |
					UNIT_STATE_ROOT |
					UNIT_STATE_STUNED);
		}

		uint32 getLevel() const { return GetUInt8Value(UNIT_FIELD_LEVEL); }


		uint64 const& GetOwnerGUID() const { return GetUInt64Value(UNIT_FIELD_SUMMONEDBY); }
		uint64 GetPetGUID() const { return GetUInt64Value(UNIT_FIELD_SUMMON); }


		Unit* GetOwner() const;
		Pet* GetPet() const;

		void SetPet(Pet* pet);

		MotionMaster* GetMotionMaster() { return &i_motionMaster; }

		// Event handler
		EventProcessor m_Events;

		bool IsStopped() const { return !(hasUnitState(UNIT_STATE_MOVING)); }
		void StopMoving();

		// virtual functions for all world objects type
//		bool isVisibleForInState(Player const* u, bool inVisibleList) const;


		bool isVendor()          const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_VENDOR); }
		bool isTrainer()         const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_TRAINER); }
		bool isQuestGiver()      const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER); }
		bool isGossip()          const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP); }
		bool isBanker()          const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BANKER); }
		bool isInnKeeper()       const { return HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_INNKEEPER); }
		bool isServiceProvider() const
		{
			return HasFlag( UNIT_NPC_FLAGS,
				UNIT_NPC_FLAG_VENDOR |
				UNIT_NPC_FLAG_TRAINER |
				UNIT_NPC_FLAG_BANKER |
				UNIT_NPC_FLAG_INNKEEPER );
		}

	protected:
		explicit Unit( WorldObject *instantiator );

		DeathState m_deathState;

		MotionMaster i_motionMaster;

	private:

		uint32 m_state;

};

#endif
