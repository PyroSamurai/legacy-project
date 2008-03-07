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
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "Opcodes.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "Database/DatabaseEnv.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Player.h"

void WorldSession::HandleMessagechatOpcode( WorldPacket & recv_data )
{
	sLog.outDebug( "WORLD: Recvd CMSG_CHAT Message" );
	uint8       chatType;
	uint32      target = 0;
	std::string msg;

	recv_data >> chatType;
	switch (chatType)
	{
		case CHAT_MSG_WHISPER:
			recv_data >> target;

		case CHAT_MSG_ALL:
		case CHAT_MSG_LIGHT:
		default:
			recv_data >> msg;
	}

	WorldPacket data;
//	ChatHandler::FillMessageData(&data, this, chatType, NULL, target, msg, NULL);
	sLog.outString("Message from '%s' contains '%s'", GetPlayer()->GetName(),
			msg.c_str());

	data.clear(); data.SetOpcode( 0x02 ); data.Prepare();
	data << (uint8) chatType;
	data << GetPlayer()->GetAccountId();

	for(size_t i = 0; i < msg.size(); i++)
	{
		data << (uint8) msg[i];
	}

	switch (chatType)
	{
		case CHAT_MSG_WHISPER:
		{
			Player* targetPlayer = ObjectAccessor::FindPlayer(target);
			if ( !targetPlayer )
				return;
			targetPlayer->GetSession()->SendPacket(&data);
			GetPlayer()->GetSession()->SendPacket(&data);

			break;
		}
		case CHAT_MSG_ALL:
		{
			//GetPlayer()->SendMessageToAll(&data, false);
			break;
		}
		case CHAT_MSG_LIGHT:
		default:
		{
			GetPlayer()->SendMessageToSet(&data, false);
			break;
		}
	}
}
