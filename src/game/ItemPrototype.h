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

#ifndef __LEGACY_ITEMPROTOTYPE_H
#define __LEGACY_ITEMPROTOTYPE_H

#include "Common.h"

enum ItemModType
{
	ITEM_MOD_NONE          = 0,
	ITEM_MOD_MAX_HP        = 1,
	ITEM_MOD_MAX_SP        = 2,
	ITEM_MOD_INT           = 3,
	ITEM_MOD_ATK           = 4,
	ITEM_MOD_DEF           = 5,
	ITEM_MOD_HPX           = 6,
	ITEM_MOD_SPX           = 7,
	ITEM_MOD_AGI           = 8,
	ITEM_MOD_MIND          = 9,
	ITEM_MOD_ELEMENT_EARTH = 10,
	ITEM_MOD_ELEMENT_WATER = 11,
	ITEM_MOD_ELEMENT_FIRE  = 12,
	ITEM_MOD_ELEMENT_WIND  = 13,
	ITEM_MOD_SOCKET        = 14,
	ITEM_MOD_HP            = 15,
	ITEM_MOD_SP            = 16
};

#define MAX_ITEM_MOD       17

enum ItemSpelltriggerType
{
	ITEM_SPELLTRIGGER_ON_USE          = 0,
	ITEM_SPELLTRIGGER_ON_EQUIP        = 1,
	ITEM_SPELLTRIGGER_ON_NO_DELAY_USE = 2
};

#define MAX_ITEM_SPELLTRIGGER           3

enum ItemBondingType
{
	NO_BIND                 = 0,
	BIND_WHEN_PICKED_UP     = 1,
	BIND_QUEST_ITEM         = 2
};

#define MAX_BIND_TYPE         3

// masks for ITEM_FIELD_FLAGS field
enum ITEM_FLAGS
{
	ITEM_FLAGS_BINDED               = 0x00000001,
	ITEM_FLAGS_OPENABLE             = 0x00000002,
	ITEM_FLAGS_QUEST_OBJECTIVE      = 0x00000003
};

enum InventoryType
{
	INVTYPE_NON_EQUIP                           = 0,
	INVTYPE_HEAD                                = 1,
	INVTYPE_BODY                                = 2,
	INVTYPE_WEAPON                              = 3,
	INVTYPE_WRISTS                              = 4,
	INVTYPE_FEET                                = 5,
	INVTYPE_SPECIAL                             = 6,
};

#define MAX_INVTYPE                               7

enum ItemClass
{
	ITEM_CLASS_CONSUMABLE           = 0,
	ITEM_CLASS_TRADE_GOODS          = 1,
	ITEM_CLASS_MONEY                = 2,
	ITEM_CLASS_RECIPE               = 3,
	ITEM_CLASS_GENERIC              = 4,
	ITEM_CLASS_QUEST                = 5,
	ITEM_CLASS_KEY                  = 6,
	ITEM_CLASS_JUNK                 = 7
};

#define MAX_ITEM_CLASS                8

enum ItemSubclassConsumable
{
	ITEM_SUBCLASS_CONSUMABLE        = 0,
	ITEM_SUBCLASS_POTION            = 1,
	ITEM_SUBCLASS_SCROLL            = 2,
	ITEM_SUBCLASS_ITEM_ENHANCEMENT  = 3,
	ITEM_SUBCLASS_CONSUMABLE_OTHER  = 4
};

#define MAX_ITEM_SUBCLASS_CONSUMABLE  5







enum ItemSubclassTradeGoods
{
	ITEM_SUBCLASS_TRADE_GOODS        = 0
};

#define MAX_ITEM_SUBCLASS_TRADE_GOODS  1

enum ItemSubclassRecipe
{
	ITEM_SUBCLASS_TAILORING          = 0,
	ITEM_SUBCLASS_BLACKSMITHING      = 1
};

#define MAX_ITEM_SUBCLASS_RECIPE       2

enum ItemSubclassMoney
{
	ITEM_SUBCLASS_MONEY              = 0
};

#define MAX_ITEM_SUBCLASS_MONEY        1

enum ItemSubclassQuest
{
	ITEM_SUBCLASS_QUEST              = 0
};

#define MAX_ITEM_SUBCLASS_QUEST        1

enum ItemSubclassJunk
{
	ITEM_SUBCLASS_JUNK               = 0
};

#define MAX_ITEM_SUBCLASS_JUNK         1

const uint32 MaxItemSubclassValues[MAX_ITEM_CLASS] =
{
	MAX_ITEM_SUBCLASS_CONSUMABLE,
	MAX_ITEM_SUBCLASS_TRADE_GOODS,
	MAX_ITEM_SUBCLASS_RECIPE,
	MAX_ITEM_SUBCLASS_QUEST,
	MAX_ITEM_SUBCLASS_JUNK
};

struct _Damage
{
	float DamageMin;
	float DamageMax;
	uint32 DamageType;
};

struct _ItemStat
{
	uint32 ItemStatType;
	int32  ItemStatValue;
};

struct _Spell
{
	uint32 SpellId;
	uint32 SpellTrigger;
	int32  SpellCharges;
};

struct _Socket
{
	uint32 Color;
	uint32 Content;
};

struct ItemPrototype
{
	uint32 ItemId;
	uint32 modelid;
	char*  Name;
	char*  TypeDesc;
	uint32 InventoryType;
	uint32 RequiredLevel;
	char*  attribute;
	char*  slot;
	uint32 EquipmentSlot;
	uint32 contribute;
	uint32 buyprice;
	uint32 sellprice;
	uint32 MaxCount;
	uint32 Stackable;
	char*  desc;
	uint32 ContainerSlots;
	_ItemStat ItemState[10];
	_Socket Socket[3];
	uint32  GemProperties;
};

#endif
