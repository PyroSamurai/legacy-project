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

#include "GossipDef.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Util.h"
#include "Creature.h"

PlayerMenu::PlayerMenu( WorldSession *Session )
{
	pSession = Session;
	m_menuOpen = false;
}

void PlayerMenu::InitTalking()
{
	WorldPacket data;
	data.Initialize( 0x06 );
	data << (uint8 ) 0x02;
	pSession->SendPacket(&data); // sent only at 1st gossip hello
}

void PlayerMenu::SendGossipMenu( uint8 map_npcid, uint16 textId )
{
	SendTalking(map_npcid, textId);
}

void PlayerMenu::ClearMenus()
{
}

void PlayerMenu::SendTalking( uint8 map_npcid, uint16 textId, uint8 dialog_type)
{
	// 1A -- Opcode money related
	// 1  -- 1 byte +/-
	// value -- 4 byte money value

	WorldPacket data;
	pSession->SetLogging(true);
	data.Initialize( dialog_type ); //0x1B;
	data << (uint8 ) textId; //0x01;
	//pSession->SendPacket(&data);
	pSession->SetLogging(false);
	data.Initialize( 0x14 );
	data << (uint8 ) 0x09;
	//pSession->SendPacket(&data);
	//return;

	switch( dialog_type )
	{
		case 0x02:
		{
			data.Initialize( 0x1B );
			data << (uint8 ) 0x03;
			pSession->SendPacket(&data);
			data.Initialize( 0x14 );
			data << (uint8 ) 0x09;
			pSession->SendPacket(&data);
			break;
		}

		case 0x01:
		case 0x06:
		default:
		{
			pSession->GetPlayer()->IncTalkedSequence();
			data.Initialize( 0x14 );
			data << (uint8 ) 0x01;
			data << (uint16) 0x0000;
			data << (uint8 ) 0x00;
			data << (uint8 ) 0x01; // 01
			data << (uint8 ) dialog_type; // 01 Plain Dialog, 06 Select Dialog
			data << (uint8 ) 0x03; // 03
			data << (uint8 ) map_npcid;
			data << (uint16) 0x0000;
			data << (uint16) 0x0000;
			data << (uint16) 0x0000;
			data << (uint16) textId;
			pSession->SendPacket(&data);
			break;
		}
	}
}

void PlayerMenu::SendMenu(uint8 map_npcid, uint16 textId)
{
	m_menuOpen = true;
	SendTalking(map_npcid, textId, GOSSIP_TYPE_SELECT);
}

void PlayerMenu::SendSellMenu(uint8 map_npcid, uint16 textId)
{
	m_menuOpen = true;
	SendTalking(map_npcid, textId, GOSSIP_TYPE_INVENTORY);
}
