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

#ifndef __LEGACY_CHAT_H
#define __LEGACY_CHAT_H

#include "SharedDefines.h"

#include <stdarg.h>

class ChatHandler;
class WorldSession;
class Player;
class Unit;

class ChatCommand
{
	public:
		const char *     Name;
		uint32           SecurityLevel;
		bool (ChatHandler::*Handler)(const char* args);
		std::string      Help;
		ChatCommand *    ChildCommands;
};

class ChatHandler
{
	public:
		explicit ChatHandler(WorldSession* session) : m_session(session) {}
		explicit ChatHandler(Player* player) : m_session(player->GetSession()){}
		~ChatHandler() {}

		static void FillMessageData( WorldPacket *data, WorldSession* session, uint8 type, const char *channelName, uint64 target_guid, const char *message, Unit *speaker);

		void FillMessageData( WorldPacket *data, uint8 type, uint64 target_guid, const char *message)
		{
			FillMessageData( data, m_session, type, NULL, target_guid, message, NULL );
		}

		void FillSystemMessageData( WorldPacket *data, const char* message )
		{
			FillMessageData( data, CHAT_MSG_SYSTEM, 0, message );
		}

//		Player* getSelectedPlayer();
//		Unit*   getSelectedUnit();

		WorldSession * m_session;

		void SendSysMessage(const char *str);
		void PSendSysMessage(const char *format, ...) ATTR_PRINTF(2,3);
		void SendGmMessage(const char *str);
		void PSendGmMessage(const char *format, ...) ATTR_PRINTF(2,3);

		int ParseCommands(const char* text);


	protected:
		bool hasStringAbbr(const char* s1, const char* s2);
		bool ExecuteCommandInTable(ChatCommand *table, const char* text);

		ChatCommand* getCommandTable();

		///- LEVEL 0 SEC_PLAYER
		bool HandleUnstuckCommand(const char* args);
		bool HandleSaveCommand(const char* args);
		bool HandlePasswordCommand(const char* args);

		///- LEVEL 1 SEC_MODERATOR
		bool HandleLookupAreaCommand(const char* args);
		bool HandleLookupNpcCommand(const char* args);
		bool HandleLookupItemCommand(const char* args);
		bool HandleModifyGoldCommand(const char* args);

		///- LEVEL 2 SEC_GAMEMASTER
		bool HandleChangeLevelCommand(const char* args);
		bool HandleChangeLevelPetCommand(const char* args);
		bool HandleWarpCommand(const char* args);
		bool HandleSaveAllCommand(const char* args);
		bool HandleNpcTalkCommand(const char* args);
		bool HandleNpcInfoCommand(const char* args);
		bool HandleNpcAddCommand(const char* args);
		bool HandleNpcDeleteCommand(const char* args);
		bool HandleNpcEditCommand(const char* args);
		bool HandlePetAddCommand(const char* args);
		bool HandlePetReleaseCommand(const char* args);
		bool HandleItemAddCommand(const char* args);
		bool HandleVendorAddItemCommand(const char* args);
		bool HandleVendorDelItemCommand(const char* args);

		///- LEVEL 3 SEC_ADMINISTRATOR
		bool HandleGetItemState(const char* args);
		bool HandleBanCommand(const char* args);
		bool HandleUnBanCommand(const char* args);
		bool HandleLevelUpCommand(const char* args);

		// common global flag
		static bool load_command_table;
};


#endif
