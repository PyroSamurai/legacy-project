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

#include "MotionMaster.h"
//#include "Creature.h"
#include <cassert>

inline bool isStatic(MovementGenerator *mv)
{
//	return (mv == &si_idleMovement);
}

void
MotionMaster::Initialize()
{
	// clear ALL movement generators (including default)
	/*
	while (!empty())
	{
		MovementGenerator *curr = top();
		curr->Finalize(*i_owner);
		pop();
		if( !isStatic( curr ) )
			delete curr;
	}
	*/
}

MotionMaster::~MotionMaster()
{
}

void
MotionMaster::UpdateMotion(const uint32 &diff)
{
//	if (!top()->Update(*i_owner, diff))
//		MovementExpired();
}
