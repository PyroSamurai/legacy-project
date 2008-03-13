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
	OBJECT_FIELD_ENTRY             = 0x0003, // Size: 1 Type: INT, PUBLIC
	OBJECT_END                     = 0x0006
};

enum EItemFields
{
	ITEM_FIELD_OWNER               = OBJECT_END + 0x0000, // Size: 2, Type: LONG, PUBLIC
	ITEM_FIELD_CONTAINED           = OBJECT_END + 0x0002, // Size: 2, Type: LONG, PUBLIC

	ITEM_FIELD_STACK_COUNT         = OBJECT_END + 0x0008, // Size: 1, Type: INT, OWNER_ONLY, UNK2
	ITEM_FIELD_DURATION            = OBJECT_END + 0x0009, // Size: 1, Type: INT, OWNER_ONLY, UNK2

	ITEM_FIELD_FLAGS               = OBJECT_END + 0x000F, // Size: 1, Type: INT, PUBLIC

	ITEM_END                       = OBJECT_END + 0x0036,
};

enum EContainerFields
{
	CONTAINER_FIELD_NUM_SLOTS      = ITEM_END + 0x0000, // Size: 1, Type: INT, PUBLIC
	CONTAINER_END                  = ITEM_END + 0x004A,
};


enum EUnitFields
{
	UNIT_FIELD_ELEMENT        = OBJECT_END + 0x0000, // Size: 1 INT PUBLIC
	UNIT_FIELD_HP             = OBJECT_END + 0x0002, // Size: 2 LONG PUBLIC
	UNIT_END                  = OBJECT_END + 0x0004, // Size: 1 INT PUBLIC



	UNIT_FIELD_FLAGS          = OBJECT_END + 0x0028, // Size: 1 INT PUBLIC



	UNIT_DYNAMIC_FLAGS        = OBJECT_END + 0x009E, // Size: 1 INT OWNER_ONLY



	UNIT_NPC_FLAGS            = OBJECT_END + 0x00A2, // Size: 1 INT DYNAMIC

	PLAYER_XP                 = UNIT_END   + 0x0002, // Size: 1 INT PRIVATE
	PLAYER_END                = UNIT_END   + 0x0004
};

#endif
