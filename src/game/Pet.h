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

#ifndef __LEGACY_PET_H
#define __LEGACY_PET_H

#include "ObjectDefines.h"
#include "Creature.h"
//#include "Unit.h"
//#include "Spell.h"

class Unit;

enum PetType
{
	BATTLE_PET    = 0,
	BAG_PET       = 1,
	RIDING_PET    = 2
};

enum PetUpdateState
{
	PET_UNCHANGED       = 0,
	PET_CHANGED         = 1,
	PET_NEW             = 2,
	PET_REMOVED         = 3
};

#define MAX_PET_SKILL 4
#define MAX_PET_ITEMS 6

enum PetChangeFailure
{
	PET_ERR_OK                         = 0,
	PET_ERR_PET_NOT_FOUND              = 1,
	PET_ERR_PET_CANT_BE_SWAPPED        = 2,
	PET_ERR_PET_CANT_BE_SUMMONED       = 3,
	PET_ERR_PET_ALREADY_SUMMONED       = 4,
};

class Pet : public Creature
{
	public:
		explicit Pet(WorldObject *instantiator, PetType type = BATTLE_PET); 
		virtual ~Pet();

		bool LoadPetFromDB(Unit* owner, uint32 guid);
		bool Create(uint32 guid, uint32 entry, Player* owner);
		bool Create(uint32 guid, uint32 entry, const CreatureInfo *cinfo);
		virtual void SaveToDB();
		virtual void DeleteFromDB();

		PetType getPetType() const { return m_petType; }
		void setPetType(PetType type) { m_petType = type; }

		uint8 GetSlot() { return m_slot; }
		void SetSlot(uint8 slot) { m_slot = slot; }

		bool isBattle() { return m_battle == 1; }
		void SetBattle(uint8 mode) { m_battle = mode; }

		uint8 GetLoyalty() const { return m_loyalty; }
		void SetLoyalty(uint8 loyalty) { m_loyalty = loyalty; }

		//uint16 GetSkillPoint() const { return m_skillPoint; }
		//void SetSkillPoint(uint16 point) { m_skillPoint = point; }

		//uint16 GetStatPoint() const { return m_statPoint; }
		//void SetStatPoint(uint16 point) { m_statPoint = point; }

		//uint16 GetSkill(uint8 index) { return m_skill[ index ]; }
		//void SetSkill(uint8 index, uint16 skill) { m_skill[ index ] = skill; }

		//uint8 GetSkillLevel(uint8 index) { return m_skillLevel[ index ]; }
		//void SetSkillLevel(uint8 index, uint8 level) { m_skillLevel[ index ] = level; }

		void SetEquip(uint8 slot, Item *pItem);
		Item* GetEquip(uint8 slot) { return m_items[slot]; }
		uint16 GetEquipModelId(uint8 slot) const;

		void SetOwnerGUID(uint64 guid) { SetUInt64Value(ITEM_FIELD_OWNER, guid);}
		uint32 GetOwnerAccountId() const;

		// Update States
		PetUpdateState GetState() const { return uState; }
		void SetState(PetUpdateState state, Player *forplayer = NULL);
		void AddToUpdateQueueOf(Player *player);
		void RemoveFromUpdateQueueOf(Player *player);
		bool IsInUpdateQueue() const { return uQueuePos != -1; }
		uint16 GetQueuePos() const { return uQueuePos; }
		void FSetState(PetUpdateState state) // forced
		{
			uState = state;
		}

		void DumpPet();
	protected:

	private:

		Item* m_items[MAX_PET_ITEMS];

		PetType m_petType;
		uint8   m_loyalty;
		uint8   m_slot;
		uint8   m_battle;

		//uint16  m_skillPoint;
		//uint16  m_statPoint;

		uint16  m_skill[MAX_PET_SKILL];
		uint8   m_skillLevel[MAX_PET_SKILL];

		PetUpdateState uState;
		int16   uQueuePos;

};

#endif
