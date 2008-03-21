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

#ifndef __LEGACY_IDLEMOVEMENTGENERATOR_H
#define __LEGACY_IDLEMOVEMENTGENERATOR_H

#include "MovementGenerator.h"
#include "Log.h"

class LEGACY_DLL_SPEC IdleMovementGenerator : public MovementGenerator
{
	public:

		void Initialize(Unit &)
		{
			sLog.outDebug(" $$ IdleMovementGenerator::Initialize");
		}
		void Finalize(Unit &) { }
		void Reset(Unit &);
		bool Update(Unit &, const uint32 &) { return true; }
		MovementGeneratorType GetMovementGeneratorType() { return IDLE_MOTION_TYPE; }
};

extern IdleMovementGenerator si_idleMovement;
#endif
