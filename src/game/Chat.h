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

		static void FillMessageData( WorldPacket *data, WorldSession* session, uint8 type, const char *channelName, uint64 target_guid, std::string msg, Unit *speaker);

		void FillMessageData( WorldPacket *data, uint8 type, uint64 target_guid, std::string msg)
		{
			FillMessageData( data, m_session, type, NULL, target_guid, msg, NULL );
		}

		void FillSystemMessageData( WorldPacket *data, std::string msg )
		{
			FillMessageData( data, CHAT_MSG_SYSTEM, 0, msg );
		}

//		Player* getSelectedPlayer();
//		Unit*   getSelectedUnit();

		WorldSession * m_session;

		// common global flag
		static bool load_command_table;
};


#endif
