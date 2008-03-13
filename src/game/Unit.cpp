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
#include "ObjectAccessor.h"
#include "Util.h"

#include <math.h>

Unit::Unit( WorldObject *instantiator )
: WorldObject( instantiator ), i_motionMaster(this)
{
	m_objectType |= TYPE_UNIT;
	m_objectTypeId = TYPEID_UNIT;
}

Unit::~Unit()
{
}


Unit* Unit::GetUnit(WorldObject& object, uint32 accountId)
{
	return ObjectAccessor::GetUnit(object, accountId);
}


void Unit::Update( uint32 p_time )
{
	i_motionMaster.UpdateMotion(p_time);
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
