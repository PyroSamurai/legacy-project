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

#ifndef __LEGACY_ITEM_H
#define __LEGACY_ITEM_H

#include "Common.h"
#include "Object.h"
#include "ItemPrototype.h"

struct SpellEntry;
class Bag;
class QueryResult;

typedef struct
{
	uint32 setid;
	uint32 item_count;
	SpellEntry const *spells[8];
} ItemSetEffect;

enum InventoryChangeFailure
{
	EQUIP_ERR_OK                                         = 0,
	EQUPE_ERR_CANT_EQUIP_LEVEL_I                         = 1
};

enum BuyFailure
{
	BUY_ERR_CANT_FIND_ITEM                               = 0,
	BUY_ERR_ITEM_ALREADY_SOLD                            = 1,
	BUY_ERR_NOT_ENOUGH_MONEY                             = 2,
	BUY_ERR_DISTANCE_TOO_FAR                             = 3,
	BUY_ERR_CANT_CARRY_MORE                              = 4
};

enum SellFailure
{
	SELL_ERR_CANT_FIND_ITEM                              = 0,
	SELL_ERR_CANT_SELL_ITEM                              = 1,
	SELL_ERR_CANT_FIND_VENDOR                            = 2,
	SELL_ERR_YOU_DONT_OWN_THAT_ITEM                      = 3
};

enum ItemUpdateState
{
	ITEM_UNCHANGED                                 = 0,
	ITEM_CHANGED                                   = 1,
	ITEM_NEW                                       = 2,
	ITEM_REMOVED                                   = 3
};

class LEGACY_DLL_SPEC Item : public Object
{
	public:
		Item();

		virtual bool Create(uint32 guidlow, uint32 itemid, Player* owner);

		ItemPrototype const* GetProto() const;

		uint64 const& GetOwnerGUID() const { return GetUInt64Value(ITEM_FIELD_OWNER); }
		void SetOwnerGUID(uint64 guid) { SetUInt64Value(ITEM_FIELD_OWNER, guid); }

		Player* GetOwner() const;

		virtual void SaveToDB();
		virtual bool LoadFromDB(uint32 guid, uint64 owner_guid, QueryResult *result = NULL);
		virtual void DeleteFromDB();
		void DeleteFromInventoryDB();

		bool IsBag() const { return GetProto()->InventoryType == INVTYPE_BAG; }

		bool CanBeTraded() const;

		uint32 GetEntry() const { return GetUInt32Value(OBJECT_FIELD_ENTRY); }
		uint32 GetCount() const { return GetUInt32Value(ITEM_FIELD_STACK_COUNT); }
		uint32 GetMaxStackCount() const { return GetProto()->Stackable ? GetProto()->Stackable : 1; }

		uint8 GetSlot() const { return m_slot; }
		Bag *GetContainer() { return m_container; }
		uint8 GetBagSlot() const;
		void SetSlot(uint8 slot) { m_slot = slot; }
		uint16 GetPos() const { return uint16(GetBagSlot()) << 8 | GetSlot(); }
		void SetContainer(Bag *container) { m_container = container; }
		bool CanGoIntoBag(ItemPrototype const *pBagProto);

		bool IsInBag() const { return m_container != NULL; }
		bool IsEquipped() const;


		void SendTimeUpdate(Player* owner);
	private:
		uint8 m_slot;
		Bag  *m_container;
		ItemUpdateState uState;
		int16 uQueuePos;
		bool  mb_in_trade;          // true if item is currently in trade-window
};
#endif
