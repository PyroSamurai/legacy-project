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









PlayerMenu::PlayerMenu( WorldSession *Session )
{
	pSession = Session;
}

void PlayerMenu::ClearMenus()
{
}

void PlayerMenu::SendGossipMenu( uint16 textId, uint8 mapNpcId )
{
	// Send Gossip Option if available
	


	// else send default gossip text id
	SendTalking( textId, mapNpcId );
}

void PlayerMenu::SendTalking( uint16 textId, uint8 mapNpcId )
{
	WorldPacket data;
	data.Initialize( 0x06, 1 );
	data << (uint8) 0x02;
	pSession->SendPacket(&data);

	data.Initialize( 0x14, 1 );
	data << (uint8) 0x01;
	data << (uint16) 0x0000;
	data << (uint16) 0x0001;
	data << (uint16) 0x0301;
	data << mapNpcId;
	data << (uint16) 0x0000;
	data << (uint16) 0x0000;
	data << (uint16) 0x0000;
	data << textId;
	pSession->SendPacket(&data);

	data.Initialize( 0x05, 1 );
	data << (uint32) 0x025FFE02 << (uint16) 0x3900 << (uint8) 0x4A;
	//pSession->SendPacket(&data);

}
