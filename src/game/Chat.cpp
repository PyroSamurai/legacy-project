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


ChatCommand * ChatHandler::getCommandTable()
{
	static ChatCommand debugCommandTable[] =
	{
		{ "getitemstate", SEC_ADMINISTRATOR, &ChatHandler::HandleGetItemState, "", NULL },

	};

	static ChatCommand lookupCommandTable[] =
	{
		{ "area", SEC_MODERATOR, &ChatHandler::HandleLookupAreaCommand, "", NULL },
		{ "npc", SEC_MODERATOR, &ChatHandler::HandleLookupNpcCommand, "", NULL },
		{ "item", SEC_MODERATOR, &ChatHandler::HandleLookupItemCommand, "", NULL},
	};

	static ChatCommand npcCommandTable[] =
	{
		{ "talk", SEC_GAMEMASTER, &ChatHandler::HandleNpcTalkCommand, "", NULL },
		{ "info", SEC_GAMEMASTER, &ChatHandler::HandleNpcInfoCommand, "", NULL },
		{ "add", SEC_GAMEMASTER, &ChatHandler::HandleNpcAddCommand, "", NULL },
		{ "del", SEC_GAMEMASTER, &ChatHandler::HandleNpcDeleteCommand, "", NULL },
		{ "edit", SEC_GAMEMASTER, &ChatHandler::HandleNpcEditCommand, "", NULL },
	};

	static ChatCommand petCommandTable[] =
	{
		{ "add", SEC_GAMEMASTER, &ChatHandler::HandlePetAddCommand, "", NULL },
		{ "release", SEC_GAMEMASTER, &ChatHandler::HandlePetReleaseCommand, "", NULL },
	};

	static ChatCommand itemCommandTable[] =
	{
		{ "add", SEC_GAMEMASTER, &ChatHandler::HandleItemAddCommand, "", NULL },
	};

	static ChatCommand vendorCommandTable[] =
	{
		{ "add", SEC_GAMEMASTER, &ChatHandler::HandleVendorAddItemCommand, "", NULL },
		{ "del", SEC_GAMEMASTER, &ChatHandler::HandleVendorDelItemCommand, "", NULL },
	};

	static ChatCommand commandTable[] =
	{
		{ "debug", SEC_MODERATOR, NULL, "", debugCommandTable },
		{ "lookup", SEC_ADMINISTRATOR, NULL, "", lookupCommandTable },
		{ "npc", SEC_GAMEMASTER, NULL, "", npcCommandTable },
		{ "item", SEC_ADMINISTRATOR, NULL, "", itemCommandTable },
		{ "pet", SEC_GAMEMASTER, NULL, "", petCommandTable },
		{ "vendor", SEC_GAMEMASTER, NULL, "", vendorCommandTable },
		{ "warp", SEC_GAMEMASTER, &ChatHandler::HandleWarpCommand, "", NULL },
		{ "unstuck", SEC_PLAYER, &ChatHandler::HandleUnstuckCommand, "", NULL },
		{ "changelevel", SEC_ADMINISTRATOR, &ChatHandler::HandleChangeLevelCommand, "", NULL },
		{ "save", SEC_PLAYER, &ChatHandler::HandleSaveCommand, "", NULL },
		{ "saveall", SEC_GAMEMASTER, &ChatHandler::HandleSaveAllCommand, "", NULL },
		{ "ban", SEC_ADMINISTRATOR, &ChatHandler::HandleBanCommand, "", NULL },
		{ "unban", SEC_ADMINISTRATOR, &ChatHandler::HandleUnBanCommand, "", NULL },
		{ "levelup", SEC_ADMINISTRATOR, &ChatHandler::HandleLevelUpCommand, "", NULL },
		{ "gold", SEC_MODERATOR, &ChatHandler::HandleModifyGoldCommand, "", NULL },
		{ "password", SEC_PLAYER, &ChatHandler::HandlePasswordCommand, "", NULL },

	};

	if(load_command_table)
	{
		load_command_table = false;
		/*
		for(uint32 i = 0; commandTable[i].Name != NULL; i++)
		{
			if(commandTable[i].ChildCommands != NULL)
			{
				ChatCommand *ptable = commandTable[i].ChildCommands;
				for(uint32 j = 0; ptable[j].Name != NULL; j++)
				{
					if(
		*/
	}

	return commandTable;
}

bool ChatHandler::ExecuteCommandInTable(ChatCommand *table, const char* text)
{
	//sLog.outDebug("Executing '%s'", text);
	char const* oldtext = text;
	std::string fullcmd = text;
	std::string cmd = "";

	while(*text != ' ' && *text != '\0')
	{
		cmd += *text;
		text++;
	}

	while(*text == ' ') text++;
	
	if(!cmd.length())
		return false;

	for(uint32 i = 0; table[i].Name != NULL; i++)
	{
		if(strlen(table[i].Name) && !hasStringAbbr(table[i].Name, cmd.c_str()))
			continue;

		if(table[i].ChildCommands != NULL)
		{
			if(!ExecuteCommandInTable(table[i].ChildCommands, text))
			{

			}
			return true;
		}

		// check security level only simple command (without child commands)
		if(m_session->GetSecurity() < table[i].SecurityLevel)
			continue;

		if((this->*(table[i].Handler))(strlen(table[i].Name)!=0 ? text : oldtext))
		{

		}

		return true;
	}

	return false;
}

bool ChatHandler::hasStringAbbr(const char* s1, const char* s2)
{
	for(;;)
	{
		if( !*s2 )
			return true;
		else if( !*s1 )
			return false;
		else if( tolower( *s1 ) != tolower( *s2 ) )
			return false;
		s1++; s2++;
	}
}

// Note: target_guid used only in CHAT_MSG_WHISPER_INFORM mode (in this case channelName ignored)
void ChatHandler::FillMessageData( WorldPacket *data, WorldSession* session, uint8 type, const char *channelName, uint64 target_guid, const char *message, Unit* speaker)
{
	//sLog.outDebug("FillMessageData '%s'", message);
	data->Initialize( 0x02 );
	*data << (uint8 ) type;

	if( type == CHAT_MSG_SYSTEM )
		*data << (uint32) 0x0000;
	else
		*data << (uint32) session->GetPlayer()->GetAccountId();

	for(int i = 0; message[i] != '\0'; i++)
		*data << (uint8 ) message[i];
	//*data << message;
}

void ChatHandler::SendSysMessage(const char *str)
{
	WorldPacket data;
	FillSystemMessageData(&data, str);
	m_session->SendPacket(&data);
}

void ChatHandler::PSendSysMessage(const char *format, ...)
{
	va_list ap;
	char str [1024];
	va_start(ap, format);
	vsnprintf(str,1024,format,ap);
	va_end(ap);
	SendSysMessage(str);
}

void ChatHandler::SendGmMessage(const char *str)
{
	WorldPacket data;
	FillMessageData(&data, CHAT_MSG_GM, 0, str);
	m_session->SendPacket(&data);
}

void ChatHandler::PSendGmMessage(const char *format, ...)
{
	va_list ap;
	char str [1024];
	va_start(ap, format);
	vsnprintf(str,1024,format,ap);
	va_end(ap);
	SendGmMessage(str);
}

int ChatHandler::ParseCommands(const char* text)
{
	ASSERT(text);
	ASSERT(*text);

	if(text[0] != '!' && text[0] != '.')
		return 0;

	// ignore single . and ! in line
	if(strlen(text) < 2)
		return 0;

	// ignore messages starting from many dots.
	if(text[0] == '.' && text[1] == '.' || text[0] == '!' && text[1] == '!')
		return 0;

	text++;

	//sLog.outDebug("Parsing command '%s'", text);
	if(!ExecuteCommandInTable(getCommandTable(), text))
		SendSysMessage("Command not found.");

	return 1;
}

