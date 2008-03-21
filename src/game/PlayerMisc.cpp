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
#include "Player.h"
#include "WorldPacket.h"

#define _BLANK_4__ (uint32) 0x00000000

void Player::SendMotd()
{
	WorldPacket data;

	/* MOTD */
	// 89 233 len-int16 175 166 173 173 173 173 getStr("Motd")
	std::string motd;
	motd = "Welcome to LeGACY - MMORPG Server Object.";
	data.Initialize( 0x02, 1 );
	data << uint8 (0x0B);
	data << uint32(0x00000000);
	data << motd;
	GetSession()->SendPacket(&data);

	motd = "This server is still under heavy development.";
	data.Initialize( 0x02, 1 );
	data << uint8 (0x0B);
	data << uint32(0x00000000);
	data << motd;
	GetSession()->SendPacket(&data);

	SendVoucherInfo();
}

void Player::SendVoucherInfo()
{
	WorldPacket data;
	/* Voucher Point */
	// 89 233 183 173 142 169 Point2-uint32 173 173 173 173
	// 149 49 141 173 173 173 173 173 78 235 102 173 173 173 89 237
	uint32 point;
	point = 1000;
	data.Initialize( 35, 1 );
	data << uint8(4);
	data << uint32(point);
	data << uint32(0);
	data << ENCODE(uint8(149)) << ENCODE(uint8(49)) << ENCODE(uint8(141));
	data << uint32(0);
	data << uint8(0);
	data << ENCODE(uint8(78)) << ENCODE(uint8(235)) << ENCODE(uint8(102));
	data << uint16(0);
	data << uint8(0);
	data << ENCODE(uint8(89)) << ENCODE(uint8(237));
	GetSession()->SendPacket(&data);
}

void Player::SendUnknownImportant()
{
	WorldPacket data;
	data.Initialize( 0x18, 1 );
	data << (uint8 ) 0x07 << (uint8 ) 0x03 << (uint8 ) 0x04;
	GetSession()->SendPacket(&data);

	UpdateCurrentGold();

	data.Initialize( 0x29, 1 );
	data << (uint8 ) 0x05 << (uint8 ) 0x01 << (uint8 ) 0x01;
	data << _BLANK_4__ << _BLANK_4__;
	data << _BLANK_4__ << _BLANK_4__ << _BLANK_4__ << _BLANK_4__;
	data << _BLANK_4__ << _BLANK_4__ << _BLANK_4__ << _BLANK_4__;
	data << (uint32) 0x02000000 << (uint32) 0x00000001 << _BLANK_4__ << _BLANK_4__;
	data << _BLANK_4__ << _BLANK_4__ << _BLANK_4__ << _BLANK_4__;
	data << _BLANK_4__ << _BLANK_4__ << _BLANK_4__ << _BLANK_4__;
	data << (uint32) 0x00000103 << _BLANK_4__ << _BLANK_4__ << _BLANK_4__;
	data << _BLANK_4__ << _BLANK_4__ << _BLANK_4__ << _BLANK_4__;
	data << _BLANK_4__ << _BLANK_4__ << _BLANK_4__ << (uint32) 0x00010400;
	data << _BLANK_4__ << _BLANK_4__ << _BLANK_4__ << _BLANK_4__;
	data << _BLANK_4__ << _BLANK_4__ << _BLANK_4__ << _BLANK_4__;
	data << _BLANK_4__ << _BLANK_4__ << (uint32) 0x01050000 << _BLANK_4__;
	data << _BLANK_4__ << _BLANK_4__ << _BLANK_4__ << _BLANK_4__;
	data << _BLANK_4__ << _BLANK_4__ << _BLANK_4__ << _BLANK_4__;
	data << _BLANK_4__ << (uint16) 0x00 << (uint8 ) 0x00;
	GetSession()->SendPacket(&data);

	UpdateMap2Npc();

	data.Initialize( 0x0B, 1 );
	data << (uint32) 0xF24B0204 <<(uint32)  0x00000001 << (uint8 ) 0x00;
	GetSession()->SendPacket(&data);

//	Send0504();
//	Send0F0A();
}

void Player::UpdateMap2Npc()
{
	uint16 mapid = GetMapId();
	QueryResult *result = WorldDatabase.PQuery("select map_npcid, position_x, position_y from creature where mapid = '%u'", mapid);

	if( !result )
	{
		sLog.outString("No Npc position define in map %u", mapid);
		return;
	}

	int count = 0;

	WorldPacket data;
	data.Initialize( 0x16, 1 );
	data << (uint8) 0x04;
	do
	{
		Field *f = result->Fetch();
		uint8  map_npcid = f[0].GetUInt8();
		uint16 npc_pos_x = f[1].GetUInt16();
		uint16 npc_pos_y = f[2].GetUInt16();

		data << map_npcid;
		data << (uint16) 0x0000 << (uint8) 0x00;     // unknown fields
		data << npc_pos_x << npc_pos_y;
		data << (uint32) 0x00000000 << (uint8) 0x00; // unknown fields;
		count++;
	} while( result->NextRow() );

	sLog.outString("Player::UpdateMap2Npc Updating %u npc's", count);
	GetSession()->SendPacket(&data);
}
