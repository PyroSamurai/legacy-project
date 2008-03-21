/* * Copyright (C) 2008-2008 LeGACY <http://code.google.com/p/legacy-project/> *
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

#include "Creature.h"
#include "MapManager.h"
#include "Opcodes.h"
#include "RandomMovementGenerator.h"

template<>
void
RandomMovementGenerator<Creature>::Initialize(Creature &creature)
{
	sLog.outDebug(" $$ RandomMovementGenerator::Initialize");
	uint16 x, y;

	creature.GetRespawnCoord(x, y);

	i_nextMove = 1;
	i_waypoints[0][0] = x;
	i_waypoints[0][1] = y;

	for(uint8 idx=1; idx < MAX_RAND_WAYPOINTS+1; ++idx)
	{
		i_waypoints[idx][0] = x + (urand(100, 150) * (urand(0,1) * -1));
		i_waypoints[idx][1] = y + (urand(100, 150) * (urand(0,1) * -1));
		//i_waypoints[idx][0] = x + ((rand() % 3) - 150);
		//i_waypoints[idx][1] = y + ((rand() % 3) - 150);
	}
	i_nextMoveTime.Reset(urand(5000, 6000-1));
	creature.StopMoving();
}

template<>
void
RandomMovementGenerator<Creature>::Reset(Creature &creature)
{
	i_nextMove = 1;
	i_nextMoveTime.Reset(urand(0,10000-1));
	creature.StopMoving();
}

template<>
bool
RandomMovementGenerator<Creature>::Update(Creature &creature, const uint32 &diff)
{
	if(!&creature)
		return true;

	i_nextMoveTime.Update(diff);

	if(i_nextMoveTime.Passed())
	{
		if(creature.IsStopped())
		{
			assert( i_nextMove <= MAX_RAND_WAYPOINTS );
			uint16 x = i_waypoints[i_nextMove][0];
			uint16 y = i_waypoints[i_nextMove][1];
			creature.addUnitState(UNIT_STATE_ROAMING);
			creature.SendMonsterMove(x, y, 0);
			sLog.outDebug("****** RandomMovementGenerator<Creature>::Update *******");
			i_nextMoveTime.Reset(urand(10000, 12000-1));
		}
		else
		{
			creature.StopMoving();

			++i_nextMove;
			if(i_nextMove == MAX_RAND_WAYPOINTS)
			{
				i_nextMove = 0;
				i_nextMoveTime.Reset(urand(0, 3000-1));
			}
			else
			{
				i_nextMoveTime.Reset(urand(0, 7000-1));
			}
		}
	}
	return true;
}
