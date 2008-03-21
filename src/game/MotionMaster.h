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

#ifndef __LEGACY_MOTIONMASTER_H
#define __LEGACY_MOTIONMASTER_H

#include "Common.h"
#include <stack>

class MovementGenerator;
class Unit;

enum MovementGeneratorType
{
	IDLE_MOTION_TYPE     = 0, // IdleMovementGenerator.h

	RANDOM_MOTION_TYPE   = 1, // RandomMovementGenerator.h

	MAX_DB_MOTION_TYPE   = 2, // *** this & below motion can't be set in DB

	ANIMAL_RANDOM_MOTION_TYPE = MAX_DB_MOTION_TYPE, // void

	TARGETED_MOTION_TYPE = 3, // TargetedMovementGenerator.h

	HOME_MOTION_TYPE     = 4  // HomeMovementGenerator.h
};

class LEGACY_DLL_SPEC MotionMaster : private std::stack<MovementGenerator *>
{
	private:
		typedef std::stack<MovementGenerator *> Impl;

	public:

		explicit MotionMaster(Unit *unit) : i_owner(unit) {}
		~MotionMaster();

		void Initialize();

		MovementGenerator* operator->(void) { return top(); }

		using Impl::top;
		using Impl::empty;
		using Impl::size;

		typedef Impl::container_type::const_iterator const_iterator;
		const_iterator begin() const { return Impl::c.begin(); }
		const_iterator end() const { return Impl::c.end(); }

		void UpdateMotion(const uint32 &diff);

		void Clear(bool reset = true);

		void MovementExpired(bool reset = true);

		void Idle(void);

		void TargetedHome();

		void Mutate(MovementGenerator *m);


	private:
		Unit *i_owner;
};
#endif
