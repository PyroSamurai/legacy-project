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
#include "MotionMaster.h"
#include <list>

class LEGACY_DLL_SPEC Unit : public WorldObject
{
	public:



		virtual ~Unit();

		virtual void Update( uint32 time );

		static Unit* GetUnit(WorldObject& object, uint32 accountId);

		MotionMaster* GetMotionMaster() { return &i_motionMaster; }

		// virtual functions for all world objects type
//		bool isVisibleForInState(Player const* u, bool inVisibleList) const;

	protected:
		explicit Unit( WorldObject *instantiator );

		MotionMaster i_motionMaster;

};

#endif
