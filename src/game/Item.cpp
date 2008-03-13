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

#include "Common.h"
#include "Item.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"
#include "Database/DatabaseEnv.h"

void AddItemsSetItem(Player*player, Item*item)
{
}
void RemoveItemsSetItem(Player*player,ItemPrototype const *proto)
{
}

Item::Item()
{
	m_objectType |= TYPE_ITEM;
	m_objectTypeId = TYPEID_ITEM;

	m_updateFlag = (UPDATEFLAG_LOWGUID | UPDATEFLAG_HIGHGUID);

	m_valuesCount = ITEM_END;
	m_slot = 0;
	uState = ITEM_NEW;
	uQueuePos = -1;
	m_container = NULL;
	mb_in_trade = false;
}

bool Item::Create(uint32 guidlow, uint32 itemid, Player* owner)
{
	Object::_Create(guidlow, HIGHGUID_ITEM);

	SetUInt32Value(OBJECT_FIELD_ENTRY, itemid);

	SetUInt64Value(ITEM_FIELD_OWNER, owner ? owner->GetGUID() : 0);
	SetUInt64Value(ITEM_FIELD_CONTAINED, owner ? owner->GetGUID() : 0);

	ItemPrototype const *itemProto = objmgr.GetItemPrototype(itemid);
	if(!itemProto)
		return false;

	SetUInt32Value(ITEM_FIELD_STACK_COUNT, 1);

	SetUInt32Value(ITEM_FIELD_FLAGS, itemProto->Flags);
	SetUInt32Value(ITEM_FIELD_DURATION, abs(itemProto->Duration));

	return true;
}

void Item::SaveToDB()
{
	uint32 guid =GetGUIDLow();
	switch (uState)
	{
		default:
			break;
	}
}

bool Item::LoadFromDB(uint32 guid, uint64 owner_guid, QueryResult *result)
{
	return true;
}

void Item::DeleteFromDB()
{
}

void Item::DeleteFromInventoryDB()
{
}

ItemPrototype const *Item::GetProto() const
{
	return objmgr.GetItemPrototype(GetUInt32Value(OBJECT_FIELD_ENTRY));
}

Player* Item::GetOwner() const
{
	return objmgr.GetPlayer(GetOwnerGUID());
}

uint8 Item::GetBagSlot() const
{
	return m_container ? m_container->GetSlot() : uint8(INVENTORY_SLOT_BAG_0);
}

bool Item::IsEquipped() const
{
	return !IsInBag() && m_slot < EQUIPMENT_SLOT_END;
}

bool Item::CanBeTraded() const
{
}

bool Item::CanGoIntoBag(ItemPrototype const *pBagProto)
{
}

// Though the client has the information in the item's data field
// we have to send SMSG_TIME_UPDATE to display the remaining
// time
void Item::SendTimeUpdate(Player* owner)
{
}
