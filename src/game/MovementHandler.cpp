/*
 * Copyright (C) 2008-2008 LeGACY <http://www.legacy-project.org/>
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
#include "WorldSocket.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "Player.h"
#include "MapManager.h"

void WorldSession::HandleMovementOpcodes( WorldPacket & recv_data )
{
	sLog.outDebug( "WORLD: Recvd CMSG_PLAYER_MOVE Message" );
	// TODO: CHECK_PACKET_SIZE
	
	if(GetPlayer()->GetDontMove())
		return;

//	GetSocket()->LogPacket( recv_data, 1 );
	/* extract packet */

	uint8  sub_opcode;
	uint8  unknown1;
	uint16 pos_x;
	uint16 pos_y;
	uint16 unknown2;
	recv_data >> sub_opcode;
	recv_data >> unknown1;
	recv_data >> pos_x;
	recv_data >> pos_y;
	recv_data >> unknown2;

	sLog.outDetail("Player '%s' (%u) move to %u, %u", GetPlayer()->GetName(),
			GetPlayer()->GetAccountId(), pos_x, pos_y);

	sLog.outDetail("SubOpcode = %u", sub_opcode);
	sLog.outDetail("Unknown 1 = %u", unknown1);
	sLog.outDetail("Unknown 2 = %u", unknown2);

	WorldPacket data(1);
	data.clear();
	data.SetOpcode(0x06); data.Prepare();
	data << sub_opcode;
	data << GetPlayer()->GetAccountId();
	data << unknown1;
	data << pos_x << pos_y;

	GetPlayer()->SendMessageToSet(&data, false);
	GetPlayer()->Relocate(pos_x, pos_y);
}
