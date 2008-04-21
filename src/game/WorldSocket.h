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

#ifndef __LEGACY_WORLDSOCKET_H
#define __LEGACY_WORLDSOCKET_H

#include "sockets/TcpSocket.h"

enum ResponseCodes
{
	RESPONSE_SUCCESS                = 0x00,
	RESPONSE_FAILURE                = 0x01
};

class WorldPacket;
class SocketHandler;
class WorldSession;

/// Handle connection with the client software
class WorldSocket : public TcpSocket
{
	public:
		WorldSocket(ISocketHandler&);
		~WorldSocket();

		void LogHeader(uint16 hdr, uint16 size, uint8 cmd);
		void LogPacket(WorldPacket packet, uint8 origin);

		//void SendImmediatePacket(WorldPacket* packet);
		void SendPacket(WorldPacket* packet, bool log=false);
		void CloseSocket();

		void OnAccept();
		void OnRead();
		void OnDelete();

		void Update(time_t diff);
		// Player Queue
		void SendAuthWaitQue(uint32 position);
		void SendMotd();

		void _ChangeMap(const uint16, const uint16, const uint16);
		void _EndForNow();




		void SetLogging(bool newvalue)
		{
			m_logPacket = newvalue;
		}

	protected:
		void _HandleLogonChallenge();
		bool _HandleAuthSession(WorldPacket& recvPacket);
		void _HandlePlayerInfo(uint32 uid, std::string password);
		void _HandlePing(WorldPacket& recvPacket);

		void _HandleAddOnPacket();

		void _SendInitializePacket(uint32 uid);
		void _SendFixedPacket();

	private:
		uint32 _seed;
		uint8 _cmd;
		uint16 _remaining;
		WorldSession* _session;

		ZThread::LockedQueue<WorldPacket*,ZThread::FastMutex> _sendQueue;

		void SizeError(WorldPacket const& packet, uint32 size) const;

		uint32 m_pkt_cnt;
		uint32 m_send_cnt;
		uint32 m_recv_cnt;

		bool   m_logPacket;
};

#endif
