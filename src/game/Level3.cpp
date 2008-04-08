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
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"
#include "Log.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "GossipDef.h"

bool ChatHandler::HandleNpcTalkCommand(char const* args)
{
	if(!args)
		return false;

	char* map_npcid_text = strtok((char*)args, " ");
	char* dialog_type_text = strtok(NULL, " ");
	char* textId_text = strtok(NULL, " ");
	if(!map_npcid_text || !dialog_type_text || !textId_text)
		return false;

	uint8 map_npcid = atoi(map_npcid_text);
	std::string type = dialog_type_text;
	uint8 dialog_type = 0;
	if(type == "plain")
		dialog_type = 0x01;
	else if(type == "select")
		dialog_type = 0x06;
	else
		dialog_type = atoi(dialog_type_text);
	uint16 textId = atoi(textId_text);

	sLog.outDebug("");
	sLog.outDebug("COMMAND: HandleNpcTalkCommand");
	sLog.outDebug("COMMAND: args map_npcid `%u`, dialog type `%s`, textId `%u`", map_npcid, dialog_type_text, textId);

	m_session->GetPlayer()->PlayerTalkClass->SendTalking(map_npcid, textId, dialog_type);

	return true;
}

bool ChatHandler::HandleNpcInfoCommand(char const* args)
{
	sLog.outDebug("COMMAND: HandleNpcInfoCommand");
	return true;
}
