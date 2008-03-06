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
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Chat.h"
#include "MapManager.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"

bool ChatHandler::load_command_table = true;
/*
ChatCommand * ChatHandler::getCommandTable()
{
	static ChatCommand lookupCommandTable[] = {
	{"area", SEC_MODERATOR, &ChatHandler::HandleLookupAreaCommand, "", NULL },
	{"door", SEC_MODERATOR, &ChatHandler::HandleLookupDoorCommand, "", NULL },
	{"tele", SEC_MODERATOR, &ChatHandler::HandleLookupTeleCommand, "", NULL }
	};
}
*/

// Note: target_guid used only in CHAT_MSG_WHISPER_INFORM mode (in this case channelName ignored)
void ChatHandler::FillMessageData( WorldPacket *data, WorldSession* session, uint8 type, const char *channelName, uint64 target_guid, std::string msg, Unit* speaker)
{
	data->clear();
	data->SetOpcode( 0x02 );
	data->Prepare();
	*data << type;
	*data << session->GetPlayer()->GetAccountId();
//	*data << msg.c_str();
}
