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
#include "Database/DatabaseEnv.h"
#include "Opcodes.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ObjectAccessor.h"

void WorldSession::HandleGroupOpcodes( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 1);

	sLog.outDebug( "WORLD: Recvd CMSG_PLAYER_GROUP_COMMAND Message" );

	uint8 groupCommandType;
	recv_data >> groupCommandType;

	WorldPacket data;
	switch(groupCommandType)
	{
		///- Member Request
		case 0x01:
		{
			if( _player->isJoinedTeam() )
				return;   // current player already joined team

			if( _player->isBattleInProgress() || _player->GetBattleMaster() )
				return;   // current player is in battle

			CHECK_PACKET_SIZE(recv_data, 4);
			uint32 leaderId;
			recv_data >> leaderId;

			Player* leader = ObjectAccessor::FindPlayerByAccountId(leaderId);
			if( !leader ) return;

			if( leader->isJoinedTeam() )
				return;   // current leader is not a team leader

			data.Initialize( 0x0D );
			data << (uint8 ) 0x01;
			data << (uint32) GetAccountId();
			leader->GetSession()->SendPacket(&data);
		} break;

		///- Leader Response
		case 0x03:
		{
			if( _player->isJoinedTeam() )
				return;  // current player is member of team

			if( _player->isBattleInProgress() )
				return;  // current player is in battle

			CHECK_PACKET_SIZE(recv_data, 1+4);
			uint8  response;
			uint32 memberId;
			recv_data >> response;
			recv_data >> memberId;

			Player* member  = ObjectAccessor::FindPlayerByAccountId(memberId);
			if( !member ) return;

			if( member->isJoinedTeam() )
				return;  // current member already joined team

			if( member->isTeamLeader() )
				return;  // current member is already leader of a team

			if( member->isBattleInProgress() || member->GetBattleMaster() )
				return;  // current member is in battle

			///- Notify response to requester
			data.Initialize( 0x0D );
			data << (uint8 ) groupCommandType;
			data << (uint8 ) response;
			data << (uint32) GetAccountId();

			member->GetSession()->SendPacket(&data);

			switch( response )
			{
				case 0x01: // accepted
				{
					if( !_player->CanJoinTeam() )
						return; // team is full

					_player->JoinTeam(member);
					break;
				}
				case 0x02: // rejected
				case 0x03: // no response
				default:
					break;
			}
		} break;

		///- Group Member Leave/Disconnected
		case 0x04:
		{
			CHECK_PACKET_SIZE( recv_data, 1+4 );
			uint32 teamleaderId;
			recv_data >> teamleaderId;

			if( _player->isTeamLeader() )
				_player->DismissTeam();
			else
			{
				Player* leader = objmgr.GetPlayerByAccountId(teamleaderId);
				if( !leader )
					return;

				leader->LeaveTeam(_player);
			}

		} break;

		///- Grant Sub-Leader as leader
		case 0x05:
		{
			CHECK_PACKET_SIZE( recv_data, 1+4 );
			uint32 subleaderId;
			recv_data >> subleaderId;
			_player->SetSubleader(subleaderId);
		} break;

		///- Revoke Sub-Leader as leader
		case 0x06:
		{
			CHECK_PACKET_SIZE( recv_data, 1+4 );
			uint32 subleaderId;
			recv_data >> subleaderId;
			_player->UnsetSubleader(subleaderId);
		} break;

		///- Ask other to join team as leader
		case 0x07:
		{
			CHECK_PACKET_SIZE( recv_data, 1+4 );
			uint32 targetId;
			recv_data >> targetId;
		} break;

		default:
			break;
	}
}
