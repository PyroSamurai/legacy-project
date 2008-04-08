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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "Opcodes.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Item.h"
#include "ObjectAccessor.h"

void WorldSession::HandleUseItemOpcodes( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE( recv_data, 1 );

	uint8 useitemtype;
	uint8 tmp_invslot;
	uint8 invslot;
	uint8 amount;
	uint8 target;
	uint8 unk1;

	recv_data >> useitemtype;

	switch( useitemtype )
	{
		case 0x0B: // equip to player
		{
			CHECK_PACKET_SIZE( recv_data, 1 );
			recv_data >> tmp_invslot;

			invslot = tmp_invslot + INVENTORY_SLOT_ITEM_START - 1;

			Item* item = _player->GetItemByPos( invslot );

			if( !item )
				return;

			uint8 slot = item->GetProto()->EquipmentSlot;

			bool can_swap = true;

			item->DumpItem();
			uint8 dest = slot;
			if( _player->IsEquipmentPos( dest ) )
			{
				if( _player->CanEquipItem( slot, dest, item, can_swap, false ) == EQUIP_ERR_OK )
				{
					_player->RemoveItem( invslot );
					Item *pItem2 = _player->GetItemByPos( dest );
					if( pItem2 )
					{
						_player->RemoveItem( dest );
						_player->StoreItem( invslot, pItem2, false );
					}
					_player->EquipItem(dest, item, can_swap);
				}
				else
					return;
			}
			else
				return;

			WorldPacket data;
			data.Initialize( 0x17 );
			data << (uint8 ) 0x11;
			data << (uint8 ) tmp_invslot;
			SetLogging(true);
			SendPacket(&data);
			SetLogging(false);

			_player->DumpPlayer();
			break;
		}

		case 0x0F: // consumable item
		{
			CHECK_PACKET_SIZE( recv_data, 1+1+1+1 );
			recv_data >> invslot;
			recv_data >> amount;
			recv_data >> target; // 0 to player
			recv_data >> unk1;
			break;
		}

		case 0x11: // equip to pet
		{
			CHECK_PACKET_SIZE( recv_data, 1+1 );
			recv_data >> target;
			recv_data >> invslot;

			break;
		}

	}
}


