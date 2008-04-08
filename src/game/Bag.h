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

#ifndef __LEGACY_BAG_H
#define __LEGACY_BAG_H

// Maximum 25 Slots
#define MAX_BAG_SIZE 25

#include "Object.h"
#include "ItemPrototype.h"
#include "Unit.h"
#include "Creature.h"
#include "Item.h"

class Bag : public Item
{
	public:

		Bag();
		~Bag();




		Item* GetItemByPos( uint8 slot ) const {}
		uint32 GetItemCount( uint32 item, Item* eItem = NULL ) const {}



		bool IsEmpty() const {}

	protected:

		// Bag Storage space
		Item* m_bagslot[MAX_BAG_SIZE];
};
/*
inline Item* NewItemOrBag(ItemPrototype const * proto)
{
	return (proto->InventoryType == INVTYPE_BAG) ? new Bag : new Item;
}
*/
inline Item* NewItem(ItemPrototype const * proto)
{
	return new Item;
}

#endif
