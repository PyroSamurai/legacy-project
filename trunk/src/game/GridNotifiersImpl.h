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

#ifndef __LEGACY_GRIDNOTIFIERSIMPL_H
#define __LEGACY_GRIDNOTIFIERSIMPL_H

#include "GridNotifiers.h"
#include "WorldPacket.h"
#include "Player.h"
#include "UpdateData.h"
#include "CreatureAI.h"


///template<class T>



inline void
LeGACY::PlayerRelocationNotifier::Visit(PlayerMapType &m)
{
	for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
	{
		if(&i_player==iter->getSource())
			continue;

		// visibility for players updated by ObjectAccessor::UpdateVisibilityFor calls in appropriate places
		

	}
}

inline void PlayerCreatureRelocationWorker(Player* pl, Creature* c)
{
	// update creature visibility at player/creature move
	pl->UpdateVisibilityOf(c);

	// Creature AI reaction
	if(!c->hasUnitState(UNIT_STATE_CHASE | UNIT_STATE_SEARCHING | UNIT_STATE_FLEEING))
	{
		//if( c->AI() && c->AI()->isVisible(pl) && !c->IsInEvadeMode() )
			c->AI()->MoveInLineOfSight(pl);
	}
}

inline void
LeGACY::PlayerRelocationNotifier::Visit(CreatureMapType &m)
{
	if(!i_player.isAlive()) // || i_player.isInFlight())
		return;

	for(CreatureMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
		if( iter->getSource()->isAlive())
			PlayerCreatureRelocationWorker(&i_player, iter->getSource());
}


inline void
LeGACY::ObjectUpdater::Visit(CreatureMapType &m)
{
	for(CreatureMapType::iterator iter=m.begin(); iter != m.end(); ++iter) {
//		sLog.outString( "GridNotifierImpl::LeGACY::ObjectUpdater" );
		iter->getSource()->Update(i_timeDiff);
	}
}

#endif
