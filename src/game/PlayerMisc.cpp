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

void Player::SendUnknownImportant()
{
	WorldPacket data;
	data.clear(); data.SetOpcode( 0x18 ); data.Prepare();
	data << (uint8 ) 0x07 << (uint8 ) 0x03 << (uint8 ) 0x04;
	GetSession()->SendPacket(&data);

	UpdateCurrentGold();

	data.clear(); data.SetOpcode( 0x29 ); data.Prepare();
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
/*
	data.clear(); data.SetOpcode( 0x16 ); data.Prepare();
	//data << (uint32) 0x00000104 << (uint32) 0x1C024E00 << (uint8 ) 0x02 << (uint16) 0x0000;
	
	data << (uint8 ) 0x04; // number of npc in map

	data << (uint8 ) 0x01;   // npc id
	data << (uint16) 0x0000;
	data << (uint8 ) 0x00;
	data << (uint16) 0x011A; // npc pos x 024E
	data << (uint16) 0x0163; // npc pos y 021C
	data << (uint32) 0x00000000;
	data << (uint8 ) 0x00;
	

	//data << (uint32) 0x02000000 << (uint32) 0x66000000 << (uint32) 0x0001B803 << _BLANK_4__;
	data << (uint8 ) 0x02;   // npc id
	data << (uint16) 0x0000;
	data << (uint8 ) 0x00;
	data << (uint16) 0x00B6; // pos x 0366
	data << (uint16) 0x0203; // pos y 01B8
	data << (uint32) 0x00000000;
	data << (uint8 ) 0x00;


	//data << (uint32) 0x00000003 << (uint32) 0x01CC03B6 << _BLANK_4__ << (uint32) 0x00000400;
	data << (uint8 ) 0x03;
	data << (uint16) 0x0000;
	data << (uint8 ) 0x00;
	data << (uint16) 0x01B6; // 03B6;
	data << (uint16) 0x0203; // 01CC;
	data << (uint32) 0x00000000;
	data << (uint8 ) 0x00;
*/
/*
	data << (uint8) 0x04;
	data << (uint8) 0x00;
	data << (uint32) 0x94059600 << (uint32) 0x00000002 << (uint32) 0x00050000 << (uint32) 0x01360000;
	data << (uint32) 0x00000244 << (uint32) 0x00060000 << (uint32) 0xE6000000 << (uint32) 0x00025800;
	data << _BLANK_4__;
*/
//	GetSession()->SendPacket(&data);

	UpdateMap2Npc();

	data.clear(); data.SetOpcode( 0x0B ); data.Prepare();
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
	data.clear(); data.SetOpcode( 0x16 ); data.Prepare();
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
