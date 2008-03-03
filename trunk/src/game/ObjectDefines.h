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

#ifndef __LEGACY_OBJECTDEFINES_H
#define __LEGACY_OBJECTDEFINES_H

#include "Platform/Define.h"


enum HighGuid
{
	HIGHGUID_ITEM        = 0x40000000,
	HIGHGUID_UNIT        = 0xF0070000,
	HIGHGUID_PLAYER      = 0x00000000,
	HIGHGUID_GAMEOBJECT  = 0xF0060000
};

#define GUID_HIPART(x)   (uint32)(uint64(x) >> 32)
#define GUID_LOPART(x)   (uint32)(uint64(x) & 0xFFFFFFFFULL)
#define MAKE_GUID(l, h)  uint64( uint32(l) | ( uint64(h) << 32 ) )

#define IS_CREATURE_GUID(Guid)       ( GUID_HIPART(Guid) == HIGHGUID_UNIT )
#define IS_PLAYER_GUID(Guid)         ( GUID_HIPART(Guid) == HIGHGUID_PLAYER )
#define IS_ITEM_GUID(Guid)           ( GUID_HIPART(Guid) == HIGHGUID_ITEM )
#define IS_GAMEOBJECT_GUID(Guid)     ( GUID_HIPART(Guid) == HIGHGUID_GAMEOBJECT )
//#define IS_DYNAMICOBJECT_GUID(Guid)  ( GUID_HIPART(Guid) == HIGHGUID_DYNAMICOBJECT )
//#define IS_CORPSE_GUID(Guid)         ( GUID_HIPART(Guid) == HIGHGUID_CORPSE )
//#define IS_PLAYER_CORPSE_GUID(Guid)  ( GUID_HIPART(Guid) == HIGHGUID_PLAYER_CORPSE )
//#define IS_TRANSPORT(Guid)           ( GUID_HIPART(Guid) == HIGHGUID_TRANSPORT )
//#define IS_MO_TRANSPORT(Guid)        ( GUID_HIPART(Guid) == HIGHGUID_MO_TRANSPORT )

#endif
