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

	uint8 useitemtype = 0;
	uint8 tmp_invslot = 0;
	uint8 tmp_eqslot = 0;
	uint8 invslot = 0;
	uint8 eqslot = 0;
	uint8 amount = 0;
	uint8 target = 0;
	uint8 unk1 = 0;

	bool  can_swap = false;

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

			can_swap = true;

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
					_player->_ApplyItemModsFor(_player, item, true);
				}
				else
					return;
			}
			else
				return;

			if( _player->GetItemSetCount() == 5 )
				_player->_ApplyItemSetModsFor( _player, true );
			else
				_player->_ApplyItemSetModsFor( _player, false );

			_player->UpdatePlayer();

			WorldPacket data;
			data.Initialize( 0x17 );
			data << (uint8 ) 0x11;
			data << (uint8 ) tmp_invslot;

			SendPacket(&data, true);

			_player->DumpPlayer( "equip" );
			_player->DumpPlayer( "inventory" );
			break;
		}

		case 0x0C: // player unequip item
		{
			CHECK_PACKET_SIZE( recv_data, 1+1 );
			recv_data >> tmp_eqslot;
			recv_data >> tmp_invslot;

			eqslot  = tmp_eqslot + EQUIPMENT_SLOT_START - 1;
			invslot = tmp_invslot + INVENTORY_SLOT_ITEM_START - 1;

			can_swap = false;

			if( _player->CanUnequipItem( eqslot, can_swap ) != EQUIP_ERR_OK )
				return;

			Item* item = _player->GetItemByPos( eqslot );

			if( !item )
				return;

			uint8 dest = invslot;

			if( _player->IsInventoryPos( dest ) )
			{
				if( _player->CanStoreItem( invslot, dest, item, can_swap ) == EQUIP_ERR_OK )
				{
					_player->RemoveItem( eqslot );
					_player->StoreItem( dest, item, true );
				}
			}
			else
				return;

			if( _player->GetItemSetCount() == 5 )
				_player->_ApplyItemSetModsFor( _player, true );
			else
				_player->_ApplyItemSetModsFor( _player, false );

			_player->UpdatePlayer();

			WorldPacket data;
			data.Initialize( 0x17 );
			data << (uint8 ) 0x10;
			data << (uint8 ) tmp_eqslot;
			data << (uint8 ) tmp_invslot;

			SendPacket(&data, true);

			_player->DumpPlayer( "equip" );
			_player->DumpPlayer( "inventory" );
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
			recv_data >> target; // pet slot
			recv_data >> tmp_invslot;

			invslot = tmp_invslot + INVENTORY_SLOT_ITEM_START - 1;

			Item* item = _player->GetItemByPos( invslot );
			if( !item )
				return;

			if( target - 1 < 0 )
				return;

			Pet* pet = _player->GetPet(target - 1);

			if( !pet )
				return;

			uint8 slot = item->GetProto()->EquipmentSlot;

			can_swap = true;

			uint8 dest = slot;


			if( _player->IsEquipmentPos( dest ) )
			{
				if( _player->CanPetEquipItem( pet, slot, dest, item, can_swap, false ) == EQUIP_ERR_OK )
				{
					_player->RemoveItem( invslot );
					Item *pItem2 = pet->GetEquip( dest );
					if( pItem2 )
					{
						_player->PetRemoveItem( pet, dest );
						_player->StoreItem( invslot, pItem2, false );
					}

					_player->PetEquipItem( pet, dest, item, can_swap );
					_player->_ApplyItemModsFor(pet, item, true);
				}
				else
					return;
			}
			else
				return;

			_player->DumpPlayer( "pet" );
			_player->DumpPlayer( "inventory" );

			_player->UpdatePet(target - 1);

			WorldPacket data;
			data.Initialize( 0x17 );
			data << (uint8 ) 0x17;
			data << (uint8 ) target;
			data << (uint8 ) tmp_invslot;

			SendPacket(&data);
			break;
		}

		case 0x12: // pet unequip item
		{
			CHECK_PACKET_SIZE( recv_data, 1+1+1 );
			recv_data >> target; // pet slot
			recv_data >> tmp_eqslot;
			recv_data >> tmp_invslot;

			if( target - 1 < 0 )
				return;

			Pet* pet = _player->GetPet(target - 1);

			if( !pet )
				return;

			eqslot  = tmp_eqslot + EQUIPMENT_SLOT_START - 1;
			invslot = tmp_invslot + INVENTORY_SLOT_ITEM_START - 1;

			can_swap = false;

			if( _player->CanPetUnequipItem( pet, eqslot, can_swap ) != EQUIP_ERR_OK )
				return;

			Item* item = pet->GetEquip( eqslot );

			if( !item )
				return;

			uint8 dest = invslot;

			if( _player->IsInventoryPos( dest ) )
			{
				if( _player->CanStoreItem( invslot, dest, item, can_swap ) == EQUIP_ERR_OK )
				{
					_player->PetRemoveItem( pet, eqslot );
					_player->StoreItem( dest, item, true );
				}
			}
			else
				return;

			_player->DumpPlayer( "pet" );
			_player->DumpPlayer( "inventory" );

			_player->UpdatePet(target - 1);

			WorldPacket data;
			data.Initialize( 0x17 );
			data << (uint8 ) 0x16;
			data << (uint8 ) target;
			data << (uint8 ) tmp_eqslot;
			data << (uint8 ) tmp_invslot;

			SendPacket(&data);
			break;
		}

	}
}


