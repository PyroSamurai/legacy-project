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

#ifndef __LEGACY_UPDATEFIELDS_H
#define __LEGACY_UPDATEFIELDS_H

enum EObjectFields
{
	OBJECT_FIELD_GUID              = 0x0000, // Size: 2 Type: LONG, PUBLIC
	OBJECT_FIELD_TYPE              = 0x0002, // Size: 1 Type: INT, PUBLIC
	OBJECT_END                     = 0x0006
};





enum EUnitFields
{
	UNIT_FIELD_ELEMENT        = OBJECT_END + 0x0000, // Size: 1 INT PUBLIC
	UNIT_FIELD_HP             = OBJECT_END + 0x0002, // Size: 2 LONG PUBLIC
	UNIT_END                  = OBJECT_END + 0x0004, // Size: 1 INT PUBLIC

	PLAYER_XP                 = UNIT_END   + 0x0002, // Size: 1 INT PRIVATE
	PLAYER_END                = UNIT_END   + 0x0004
};

#endif
