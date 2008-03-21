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

#ifndef __LEGACY_RANDOMMOTIONGENERATOR_H
#define __LEGACY_RANDOMMOTIONGENERATOR_H

#include "MovementGenerator.h"
#include "Timer.h"

#define MAX_RAND_WAYPOINTS 8

template<class T>
class LEGACY_DLL_DECL RandomMovementGenerator
: public MovementGeneratorMedium< T, RandomMovementGenerator<T> >
{
	public:
//		RandomMovementGenerator(const Unit &) : i_nextMoveTime(0) {}
		RandomMovementGenerator(const Creature &) : i_nextMoveTime(0) {}

		void Initialize(T &);
		void Finalize(T &) {}
		void Reset(T &);
		bool Update(T &, const uint32 &);

		MovementGeneratorType GetMovementGeneratorType() { return RANDOM_MOTION_TYPE; }

	private:
		TimeTracker i_nextMoveTime;
		uint16 i_waypoints[MAX_RAND_WAYPOINTS+1][3];
		uint32 i_nextMove;
};
#endif
