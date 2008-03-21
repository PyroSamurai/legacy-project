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

#ifndef __LEGACY_CREATUREAI_H
#define __LEGACY_CREATUREAI_H

#include "Common.h"
#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "Dynamic/ObjectRegistry.h"
#include "Dynamic/FactoryHolder.h"

class Unit;
class Creature;

class LEGACY_DLL_SPEC CreatureAI
{
	public:
		virtual ~CreatureAI(); 

		// Called if IsVisible(Unit *who) is true at each *who move
		virtual void MoveInLineOfSight(Unit *) = 0;

		// Called at each attack of m_creature by any victim
		virtual void AttackStart(Unit *) = 0;








		// Is unit visible for MoveInLineOfSight
		virtual bool IsVisible(Unit *) const = 0;

		// Called at World update tick
		virtual void UpdateAI(const uint32 diff) = 0;

};

struct SelectableAI : public FactoryHolder<CreatureAI>, public Permissible<Creature>
{
	SelectableAI(const char *id) : FactoryHolder<CreatureAI>(id) {}
};

template<class REAL_AI>
struct CreatureAIFactory : public SelectableAI
{
	CreatureAIFactory(const char *name) : SelectableAI(name) {}

	CreatureAI* Create(void *) const;

	int Permit(const Creature *c) const { return REAL_AI::Permissible(c); }
};

enum Permitions
{
	PERMIT_BASE_NO               = -1,
	PERMIT_BASE_IDLE             = 1,
	PERMIT_BASE_REACTIVE         = 100,
	PERMIT_BASE_PROACTIVE        = 200,
	PERMIT_BASE_FACTION_SPECIFIC = 400,
	PERMIT_BASE_SPECIAL          = 800
};

typedef FactoryHolder<CreatureAI> CreatureAICreator;
typedef FactoryHolder<CreatureAI>::FactoryHolderRegistry CreatureAIRegistry;
typedef FactoryHolder<CreatureAI>::FactoryHolderRepository CreatureAIRepository;
#endif
