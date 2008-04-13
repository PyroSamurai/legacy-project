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
	sLog.outDebug("ITEM: Create guidlow %u, itemid %u, owner %u", guidlow, itemid, owner->GetGUID());

	Object::_Create(guidlow, HIGHGUID_ITEM);

	SetUInt32Value(OBJECT_FIELD_ENTRY, itemid);

	SetUInt64Value(ITEM_FIELD_OWNER, owner ? owner->GetGUID() : 0);
	SetUInt64Value(ITEM_FIELD_CONTAINED, owner ? owner->GetGUID() : 0);

	ItemPrototype const *itemProto = objmgr.GetItemPrototype(itemid);

	if(!itemProto)
		return false;

	SetUInt32Value(ITEM_FIELD_STACK_COUNT, 1);

	//SetUInt32Value(ITEM_FIELD_FLAGS, itemProto->Flags);
	//SetUInt32Value(ITEM_FIELD_DURATION, abs(itemProto->Duration));

	return true;
}

void Item::SetState(ItemUpdateState state, Player *forplayer)
{
	if (uState == ITEM_NEW && state == ITEM_REMOVED)
	{
		// pretend the item never existed
		//RemoveFromUpdateQueueOf(forplayer);
		delete this;
		return;
	}

	if (state != ITEM_UNCHANGED)
	{
		// new items must stay in the new state until saved
		if (uState != ITEM_NEW) uState = state;
		//AddToUpdateQueueOf(forplayer);
	}
	else
	{
		// unset in queue
		// the item must be removed from the queue manually
		uQueuePos = -1;
		uState = ITEM_UNCHANGED;
	}
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
	//sLog.outDebug("Item::LoadFromDB guid %u, owner %u", guid, owner_guid);
	bool delete_result = false;
	if(!result)
	{
		result = CharacterDatabase.PQuery("SELECT entry, 0,0,0,0, item_instance.* FROM item_instance WHERE guid = '%u'", guid);
		delete_result = true;
	}

	if(!result)
	{
		sLog.outError("ERROR: Item (GUID: %u owner: %u) not found in table `item_instance`, can't load.", guid, GUID_LOPART(owner_guid));
		return false;
	}

	Field *f = result->Fetch();

	_Create(guid, HIGHGUID_ITEM);

	// Load item values
	SetUInt32Value(OBJECT_FIELD_ENTRY, f[0].GetUInt32());

	SetUInt32Value(ITEM_FIELD_STACK_COUNT, f[8].GetUInt32());

	// overwrite possible wrong/corrupted guid
	SetUInt64Value(OBJECT_FIELD_GUID,MAKE_GUID(guid,HIGHGUID_ITEM));

	if(delete_result) delete result;

	if(owner_guid != 0)
		SetOwnerGUID(owner_guid);

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
	//return m_container ? m_container->GetSlot() : uint8(INVENTORY_SLOT_BAG_0);
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

void Item::DumpItem()
{
	const ItemPrototype* proto = GetProto();

	sLog.outDebug("Item: %s, id %u, modelid %u, type %s, InvType %u, EquipSlot %u", proto->Name, proto->ItemId, proto->modelid, proto->TypeDesc, proto->InventoryType, proto->EquipmentSlot);
}
