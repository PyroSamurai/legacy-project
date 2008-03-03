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

#ifndef __LEGACY_WORLDSESSION_H
#define __LEGACY_WORLDSESSION_H

#include "Common.h"

class Object;
class Player;
class Unit;
class WorldPacket;
class WorldSocket;
class WorldSession;
class QueryResult;
class LoginQueryHolder;
class CharacterHandler;

/// Player session in the World
class LEGACY_DLL_SPEC WorldSession
{
	friend class CharacterHandler;
	public:
		WorldSession(uint32 id, WorldSocket *sock);
		~WorldSession();

		void SendPacket(WorldPacket* packet);

		void QueuePacket(WorldPacket& packet);
		bool Update(uint32 diff);
		void SetSocket(WorldSocket *sock);
		WorldSocket* GetSocket() { return _socket; }

		uint32 GetAccountId() const { return _accountId; }
		Player* GetPlayer() const { return _player; }
		void SetPlayer(Player *plr) { _player = plr; }
//		void HandlePlayerLoginOpcode(WorldPacket& recvPacket);

	protected:

		void HandleMovementOpcodes(WorldPacket& recvPacket);
		void HandlePlayerLoginOpcode(WorldPacket& recvPacket);
		void HandlePlayerLogin(LoginQueryHolder * holder);

	private:

		Player *_player;
		WorldSocket *_socket;

		void HandleCharCreateOpcode(WorldPacket& recvPacket);

		bool m_playerLoading;  // code processed in LoginPlayer
		bool m_playerLogout;   // code processed in LogoutPlayer
		bool m_playerRecentlyLogout;

		uint32 _accountId;

		void FillOpcodeHandlerHashTable();
		ZThread::LockedQueue<WorldPacket*,ZThread::FastMutex> _recvQueue;
};

#endif
