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
#include "Encoding.h"
#include "Log.h"
#include "Opcodes.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSocket.h"
#include "WorldSession.h"
#include "World.h"
#include "WorldSocketMgr.h"
#include "NameTables.h"
#include "Policies/SingletonImp.h"
#include "WorldLog.h"

#include "sockets/Utility.h"


/// Client Packet Header
struct ClientPktHeader
{
	uint16 header;
	uint16 size;
	uint8 cmd;
};

struct ServerPktHeader
{
	uint16 header;
	uint16 size;
	uint8 cmd;
};

#define SOCKET_CHECK_PACKET_SIZE(P,S) if((P).size() < (S)) return SizeError((P),(S));

// WorldSocket construction and initialization.
WorldSocket::WorldSocket(ISocketHandler &sh): TcpSocket(sh), _cmd(0), _remaining(0), _session(NULL)
{
	_seed = 0xaca4af;

	m_logPacket = false;
}

// WorldSocket destructor
WorldSocket::~WorldSocket()
{
	if(_session)
		_session->SetSocket(0);

	WorldPacket *packet;

	///- Go through the to-be-sent queue and delete remaining packets
	while(!_sendQueue.empty())
	{
		packet = _sendQueue.next();
		delete packet;
	}
}

void WorldSocket::LogPacket(WorldPacket packet, uint8 origin)
{
	///- If log of world packets is enable, log the incoming packet
	if( sWorldLog.LogWorld() )
	{
		uint8 opcode = (packet.GetOpcode());
		sWorldLog.Log("Packet Origin: %s \n", origin == 0 ? "___SERVER ----->>" : "___CLIENT <<-----");
//		sWorldLog.Log("Packet # %u\n", origin == 0 ? m_send_cnt : m_recv_cnt);
		sWorldLog.Log("SOCKET: %u LENGTH: %u OPCODE: %s (0x%.2X)(%u) \nData:\n",
				(uint32)GetSocket(),
				packet.size(),
				origin == 0 ?
				LookupNameServer(packet.GetOpcode(), g_svrOpcodeNames) :
				LookupNameClient(packet.GetOpcode(), g_clntOpcodeNames),
				opcode, opcode);

		uint32 p = 0;
		while (p < packet.size())
		{
			for (uint32 j = 0; j < 16 && p < packet.size(); j++)
				sWorldLog.Log("%.2X ", ENCODE(packet[p++]));
			sWorldLog.Log("\n");
		}
		sWorldLog.Log("\n\n");
		}
}

/// Copy the packet to the to-be-sent queue
void WorldSocket::SendPacket(WorldPacket *packet)
{
	WorldPacket *pck = new WorldPacket(*packet);
	ASSERT(pck);
	pck->Finalize();
	if (m_logPacket)
		LogPacket(*pck, 0);
	_sendQueue.add(pck);
}

/// Send immediate to client
/*
void WorldSocket::SendImmediatePacket(WorldPacket *packet)
{
	TcpSocket::SendBuf((char*)packet->contents(), packet->size());
	LogPacket(*packet, 0);
}
*/

/// On client connection
void WorldSocket::OnAccept()
{
	///- Add the current socket to the list of sockets to be managed (WorldSocketMgr)
	sWorldSocketMgr.AddSocket(this);
//	Utility::ResolveLocal();
	sLog.outString("New client connected.");
//	_HandleLogonChallenge();
}

/// Log Client Header
void WorldSocket::LogHeader(uint16 hdr, uint16 size, uint8 cmd)
{
//	hdr  = ENCODE(uint16(hdr));
//	size = ENCODE(uint16(size));
	//cmd  = ENCODE(uint8(cmd));
//	sWorldLog.Log("Header: 0x%.2X ", hdr);
//	sWorldLog.Log("Data Size: %u (0x%.4X) ", size - 1, size - 1);
//	sWorldLog.Log("Command %u (0x%.2X)\n", cmd, cmd);
}

/// Read the client transmitted data
void WorldSocket::OnRead()
{
	TcpSocket::OnRead();
	ClientPktHeader hdr;
	while(1)
	{
		///- Read the packet header and decipher it (if needed)
		if (!_remaining)
		{
			if (ibuf.GetLength() < 5)
				break;



			ibuf.Read((char *)&hdr, 5);
			//_remaining = ntohs(hdr.size) - 4;
			_remaining = (ENCODE(hdr.size)) - 1;

			_cmd = ENCODE(uint8(hdr.cmd));
		}

		if (ibuf.GetLength() < _remaining)
			break;

		LogHeader(hdr.header, hdr.size, hdr.cmd);

		///- Read the remaining of the packet
		WorldPacket packet((uint8)_cmd, _remaining);
		packet.SetOpcode(_cmd);

		packet.resize(_remaining);
		if(_remaining)
		{
			printf("Remaining unread data: %u\n", _remaining);
			ibuf.Read((char*)packet.contents(), _remaining);
		}
		_remaining = 0;

		///- If log of world packets is enable, log the incoming packet
		m_recv_cnt++;
		LogPacket(packet, 1);

		///- If packet is LOGON_CHALLENGE or AUTH_SESSION, handle immediately
		switch (_cmd)
		{
			case CMSG_LOGON_CHALLENGE: // Logon Challenge
			{
				_HandleLogonChallenge();
				break;
			}
		
			case CMSG_AUTH_RESPONSE:
			{
				_HandleAuthSession(packet);
//				break;
			}

			default:
			{
				///- Else, put it in the world session queue for this user
				//   (need to be already authenticated)
				if (_session)
					_session->QueuePacket(packet);
				else
					sLog.outDetail("Received out of place packet with cmdid 0x%.2X", _cmd);
				break;
			}
		}
	}
}

/// On socket closing
void WorldSocket::CloseSocket()
{
	///- Set CloseAndDelete flag for TcpSocket class
	SetCloseAndDelete(true);

	///- Set _session to NULL. Prevent crashes
	_session = NULL;
}

/// On socket deleting
void WorldSocket::OnDelete()
{
	///- Stop sending remaining data through this socket
	if (_session)
	{
		_session->SetSocket(NULL);
		// Session deleted from World session list at socket==0, This is only back reference from socket to session.
		_session = NULL;
	}

	///- Remove the socket from the WorldSocketMgr list
	sWorldSocketMgr.RemoveSocket(this);

	///- Removes socket from player queue
	sWorld.RemoveQueuedPlayer(this);
}

void WorldSocket::_HandlePing(WorldPacket& recvPacket)
{
}

void WorldSocket::Update(time_t diff)
{
	WorldPacket *packet;

	while(!_sendQueue.empty())
	{
		packet = _sendQueue.next();

		m_send_cnt++;
		//LogPacket(*packet, 0);

		if(packet->size())
			TcpSocket::SendBuf((char*)packet->contents(), packet->size());

		delete packet;
	}
}

void WorldSocket::SizeError(WorldPacket const& packet, uint32 size) const
{
	sLog.outError("Client send packet %s (%u) with size %u but expected %u (attempt crash server?), skipped",
			LookupNameServer(packet.GetOpcode(), g_svrOpcodeNames),packet.GetOpcode(),packet.size(),size);
}

/// Logon Challenge command handler
void WorldSocket::_HandleLogonChallenge()
{
	sLog.outString("Entering _HandleLogonChallenge");
	WorldPacket data;

	///- Send a LOGON_CHALLENGE packet
	data.Initialize( 0x01, 1);
	data << uint8(0x09) << uint8(0x02);
	SendPacket(&data);

	data.Initialize( 0x14, 1 );
	data << (uint8) 0x08;
	SendPacket(&data);
}

/// Handle the client authentication packet
void WorldSocket::_HandleAuthSession(WorldPacket& recvPacket)
{
	CHECK_PACKET_SIZE(recvPacket, 1+4+4);
	sLog.outString("Entering _HandleAuthSession");

	// Saved packet for use in Login Opcode
	WorldPacket savedPacket( recvPacket );

	uint8       lenPassword;
	uint32      accountId;
	uint32      patchVer;
	std::string password;

	savedPacket >> lenPassword;
	savedPacket >> accountId;
	savedPacket >> patchVer;
	savedPacket >> password;

	QueryResult *result = loginDatabase.PQuery("SELECT * FROM accounts WHERE accountid = '%u' AND password = md5('%s') AND online = 0", accountId, password.c_str());

	///- Stop if the account is not found
	if( !result )
	{
		sLog.outString("Invalid user login");
		WorldPacket packet;
		packet.Initialize( SMSG_AUTH_LOGON, 1 );
		//packet.SetOpcode( SMSG_AUTH_LOGON ); packet.Prepare();
		packet << (uint16) 0x0006;
		SendPacket(&packet);

		_HandleLogonChallenge();

		return;
	}

	///- kick already loaded player with same account (if any) and remove session
	if(!sWorld.RemoveSession(accountId))
	{
		return;
	}

	loginDatabase.PExecute("UPDATE accounts set online = 1 WHERE accountid = '%u'", accountId);

	Field* f = result->Fetch();
	uint32 security = f[2].GetUInt32();

	_session = new WorldSession(accountId, this, security);
	
	ASSERT(_session);
	sWorld.AddSession(_session);

	result = CharacterDatabase.PQuery("SELECT * FROM characters WHERE accountid = '%u' AND psswd = md5('%s') AND online = 0", accountId, password.c_str());

	///- Create new character challenge if character is not found
	if( !result )
	{
		sLog.outString("Creating new Character for Player");
		WorldPacket packet;
		//packet.SetOpcode( 0x01 ); packet.Prepare();
		packet.Initialize( 0x01 );
		packet << (uint8) 0x03;
		packet << (uint8) 0x00;
		SendPacket(&packet);

		return;

	}

	///- do small delay
#ifdef WIN32
	Sleep(10);
#else
	ZThread::Thread::sleep(10);
#endif

	return;

}

/// Send Player Info
void WorldSocket::SendMotd()
{
	WorldPacket data(1);

	/* MOTD */
	// 89 233 len-int16 175 166 173 173 173 173 getStr("Motd")
	std::string motd;
	motd = "Welcome to LeGACY - MMORPG Server Object.";
	data.Initialize( 0x02, 1 );
	data << data.GetHeader();
	data << uint8 (11);
	data << uint32(0);
	data << motd;
	SendPacket(&data);

	motd = "This server is still under heavy development.";
	data.Initialize( 0x02, 1 );
	data << data.GetHeader();
	data << uint8(11);
	data << uint32(0);
	data << motd;
	SendPacket(&data);
	/* end MOTD */

	/* Voucher Point */
	// 89 233 183 173 142 169 Point2-uint32 173 173 173 173
	// 149 49 141 173 173 173 173 173 78 235 102 173 173 173 89 237
	uint32 point;
	point = 1000;
	data.Initialize( 37, 1 );
	data << data.GetHeader();
	data << uint8(4);
	data << uint32(point);
	data << uint32(0);
	data << ENCODE(uint8(149)) << ENCODE(uint8(49)) << ENCODE(uint8(141));
	data << uint32(0);
	data << uint8(0);
	data << ENCODE(uint8(78)) << ENCODE(uint8(235)) << ENCODE(uint8(102));
	data << uint16(0);
	data << uint8(0);
	data << ENCODE(uint8(89)) << ENCODE(uint8(237));
	SendPacket(&data);

	/* end Voucher Point */

}

/// Handle the authentication waiting queue (to be completed)
void WorldSocket::SendAuthWaitQue(uint32 position)
{
	printf("WorldSocket::SendAuthWaitQue called\n");
}

