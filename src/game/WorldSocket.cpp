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
		sWorldLog.Log("Packet Origin: %s ",
				origin == 0 ? "___SERVER ----->>" : "___CLIENT <<-----");
		sWorldLog.Log("Packet # %u\n",
				origin == 0 ? m_send_cnt : m_recv_cnt);
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
	hdr  = ENCODE(uint16(hdr));
	size = ENCODE(uint16(size));
	//cmd  = ENCODE(uint8(cmd));
	sWorldLog.Log("Header: 0x%.2X ", hdr);
	sWorldLog.Log("Data Size: %u (0x%.4X) ", size - 1, size - 1);
	sWorldLog.Log("Command %u (0x%.2X)\n", cmd, cmd);
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
				///- Ignore for now
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
		LogPacket(*packet, 0);

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
	///- Send a LOGON_CHALLENGE packet
	/* Packet # 1 - the logon challenge */
	WorldPacket data((uint8) 0x01, 5);
	data.Prepare();
	data << uint8(0x09) << uint8(0x02);
	SendPacket(&data);

	data.clear();
	data.SetOpcode((uint8) 0x14);
	data.Prepare();
	data << (uint8) 0x08;
	SendPacket(&data);
}

/// Handle the client authentication packet
void WorldSocket::_HandleAuthSession(WorldPacket& recvPacket)
{
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

	QueryResult *result = loginDatabase.PQuery("SELECT * FROM accounts WHERE accountid = '%u' AND password = '%s' AND online = 0", accountId, password.c_str());

	///- Stop if the account is not found
	if( !result )
	{
		sLog.outString("Invalid user login");
		WorldPacket packet;
		packet.SetOpcode( SMSG_AUTH_LOGON ); packet.Prepare();
		packet << (uint16) 0x0006;
		SendPacket(&packet);

		_HandleLogonChallenge();

		return;
	}

	loginDatabase.PExecute("UPDATE accounts set online = 1 WHERE accountid = '%u'", accountId);

	result = CharacterDatabase.PQuery("SELECT * FROM characters WHERE accountid = '%u' AND psswd = '%s' AND online = 0", accountId, password.c_str());

	///- Create new character challenge if character is not found
	if( !result )
	{
		sLog.outString("Creating new Character for Player");
		WorldPacket packet;
		packet.SetOpcode( 0x01 ); packet.Prepare();
		packet << (uint8) 0x03;
		packet << (uint8) 0x00;
		SendPacket(&packet);

		return;

	}

	_session = new WorldSession(accountId, this);
	
	ASSERT(_session);
	sWorld.AddSession(_session);

	///- do small delay
#ifdef WIN32
	Sleep(10);
#else
	ZThread::Thread::sleep(10);
#endif

	return;

}

void WorldSocket::_SendInitializePacket(uint32 accountId)
{
	WorldPacket data(1);

	/*
	data.clear();
	data.SetOpcode(1);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << uint8(0x09) << uint8(0x02);
	data.Finalize();
	SendPacket(&data);
	*/

	/* Packet # 2 */
	data.clear();
	data.SetOpcode(0x14);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << uint8(0x8);
	data.Finalize();
	SendPacket(&data);

	// Packet # 3 (repeat packet # 2 ???)
	SendPacket(&data);

	// Packet # 4
	// Player data
	QueryResult *result = loginDatabase.PQuery(
		"select * from Player where userid = %u", accountId);
	Field* f = result->Fetch();
	data.clear();
	data.SetOpcode(3);
	data << data.GetHeader();
	data << uint16(0); // length
	data << data.GetOpcode();


	data << uint32(accountId);
	data << f[3].GetUInt8();  // gender
	data << (uint16) 0x00; // fix dr aming
	data << f[10].GetUInt16(); // map id
	data << f[11].GetUInt16(); // pos x
	data << f[12].GetUInt16(); // pos y
	data << BLANK << BLANK << BLANK;
	//data << f[40].GetUInt32(); // color 1
	data << (uint32) 0x1A7B3A34;
	//data << f[41].GetUInt32(); // color 2
	data << (uint32) 0x1A7DAE3E;

	// equipment weared
	uint8 equip_cnt = 0;
	uint16 equip[6];
	for (int i = 0; i < 6; i++) {
		if (f[34 + i].GetUInt16()) {
			equip[i] = f[34 + i].GetUInt16();
			equip_cnt++;
		}
	}

	data << equip_cnt;
	for (int i = 0; i < equip_cnt; i++) {
		data << equip[i];
	}

	data << BLANK << BLANK << BLANK << BLANK << BLANK << BLANK << BLANK;
	std::string nick = f[7].GetCppString();
	for (int i = 0; nick[i] != '\0'; i++) {
		data << uint8(nick[i]);
	}

	data.Finalize();
	SendPacket(&data);

	/* Packet # 5 GM Info */
	data.clear();
	data.SetOpcode(0x0E);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << (uint32) 0x00006505;
	data << BLANK;
	data << uint8(2);
	data << (uint8) 0x47 << (uint8) 0x4D; // G M nick
	data << (uint8) 0x07 << (uint8) 0x01 << BLANK << BLANK;
	data << (uint8) 0x1C << (uint8) 0xAF << (uint8) 0x7D << (uint8) 0x1A;
	data << (uint8) 0x1C << (uint8) 0xAF << (uint8) 0x7D;
	data << (uint8) 0x1A << BLANK;
	data.Finalize();
	SendPacket(&data);

	/* Packet # 6 Unknown */
	data.clear();
	data.SetOpcode(0x21);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << (uint8) 0x02 << BLANK << (uint8) 0x01;
	data.Finalize();
	SendPacket(&data);

	/* Packet # 7 Current Player Data */
	data.clear();
	data.SetOpcode(0x05);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << (uint8) 0x03;        // unknown
	data << f[4].GetUInt8();     // element

	data << f[13].GetUInt16();   // current hp
	data << f[14].GetUInt16();   // current sp

	data << f[18].GetUInt16();   // int
	data << f[19].GetUInt16();   // atk
	data << f[20].GetUInt16();   // def
	data << f[23].GetUInt16();   // agi
	data << f[21].GetUInt16();   // hpx
	data << f[22].GetUInt16();   // spx
	data << f[9].GetUInt8();     // level
	data << f[17].GetUInt32();   // total xp
	data << f[30].GetUInt16();   // sisa skill
	data << f[32].GetUInt16();   // sisa stat
	data << BLANK << uint8(7) << uint8(1) << BLANK;
	data << (uint16) ((f[21].GetUInt16() * 4) + 80 + f[9].GetUInt8()); // max hp
	data << (uint16) ((f[22].GetUInt16() * 2) + 60 + f[9].GetUInt8()); // max sp

	data << f[25].GetUInt32();   // atk bonus
	data << f[26].GetUInt32();   // def bonus
	data << f[24].GetUInt32();   // int bonus
	data << f[29].GetUInt32();   // agi bonus
	data << f[27].GetUInt32();   // hpx bonus
	data << f[28].GetUInt32();   // spx bonus

	/* unknown fields - TODO: Identify */

	data << (uint8) 0xF4 << (uint8) 0x01;
	data << (uint8) 0xF4 << (uint8) 0x01;
	data << (uint8) 0xF4 << (uint8) 0x01;
	data << (uint8) 0xF4 << (uint8) 0x01;
	data << (uint8) 0xF4 << (uint8) 0x01;

	data << (uint16) 0x00;
	data << 0x0101;


	data << (uint32) 0x00 << (uint32) 0x00 << uint32(0x00) << uint32(0x00);
	data << (uint32) 0x00 << (uint32) 0x00 << uint32(0x00) << uint32(0x00);
	data << (uint32) 0x00 << (uint8) 0x00;
	//data << f[7].GetCppString();

	data.Finalize();
	SendPacket(&data);

	/* Packet # 8 */
	data.clear();
	data.SetOpcode(23);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << (uint8) 0x05;
	data << (uint8) 0x01;
	data << (uint8) 0x0C;
	data << (uint8) 0x7D;
	data << (uint8) 0x31;
	data << (uint32) 0x00 << (uint32) 0x00;
	data.Finalize();
	SendPacket(&data);

	/* Packet # 9 */
	data.clear();
	data.SetOpcode(24);
	data << data.GetHeader();
	data << data.GuessSize();
	data << data.GetOpcode();
	data << (uint8) 0x07 << (uint8) 0x03 << (uint8) 0x04;
	data.Finalize();
	SendPacket(&data);

	/* Packet # 10 */
	data.clear();
	data.SetOpcode(26);
	data << data.GetHeader();
	data << data.GuessSize();
	data << data.GetOpcode();
	data << (uint8) 0x04;
	data << (uint32) 0x00 << (uint32) 0x00;
	data.Finalize();
	SendPacket(&data);

	/* Packet # 11 */
	data.clear();
	data.SetOpcode(0x29);
//	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();

	data << (uint32) (0x00E344F4 ^ 0xADADADAD) << (uint32) 0x01010529;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x02000000 << (uint32) 0x00000001;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000103 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00010400;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x01050000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint32) 0x00000000;
	data << (uint32) 0x00000000 << (uint16) 0x0000 << (uint8) 0x00;

	data.Finalize();
	SendPacket(&data);

	/* Packet # 12 */
	data.clear();
	data.SetOpcode(0x16);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();

	data << (uint16) 0x0104 << (uint8) 0x00 << (uint32) 0x024E0000;
	data << (uint32) 0x0000021C;
	data << (uint8)  0x00 << (uint16) 0x0000 << (uint32) 0x00000002;
	data << (uint32) 0x01B80366 << (uint32) 0x00000000 << (uint8) 0x00;
	data << (uint16) 0x0003 << (uint8) 0x00 << (uint32) 0xCC03B600;
	data << (uint32) 0x00000001 << (uint32) 0x00040000 << (uint8) 0x00;
	data << (uint16) 0x9600 << (uint8) 0x05 << (uint32) 0x00000294;
	data << (uint32) 0x05000000 << (uint32) 0x36000000 << (uint8) 0x01;
	data << (uint16) 0x0244 << (uint8) 0x00 << (uint32) 0x00000000;
	data << (uint32) 0x00000006 << (uint32) 0x025800E6 << (uint8) 0x00;
	data << (uint32) 0x00000000;

	data.Finalize();
	SendPacket(&data);

	/* Packet # 13 */
	data.clear();
	data.SetOpcode(0x0B);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << (uint32) 0xF24B0204 << (uint32) 0x00000001 << (uint8) 0x00;
	data.Finalize();
	SendPacket(&data);

	/* Packet # 14 */
	data.clear();
	data.SetOpcode(0x05);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << (uint8) 0x04;
	data.Finalize();
	SendPacket(&data);

	/* Packet # 15 */
	data.clear();
	data.SetOpcode(0x0F);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << (uint8) 0x0A;
	data.Finalize();
	SendPacket(&data);

	/* Packet # 19 */
	/* #end system broadcast message */
	data.clear();
	data.SetOpcode(0x02);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << (uint8) 0x0B << (uint32) 0x00;
	data << (uint32) 0x6E652320;
	data << (uint8) 0x64;
	data.Finalize();
	SendPacket(&data);

	/* Packet # 20 */
	data.clear();
	data.SetOpcode(0x27);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << (uint32) 0x0000091F << (uint32) 0x000CDA00;
	data << (uint32) 0x27160000 << (uint32) 0x11000000;
	data << (uint32) 0x00000032 << (uint32) 0x0001D20F;
	data << (uint32) 0x025A0100 << (uint32) 0xDB050000;
	data << (uint32) 0x12000002 << (uint32) 0x0000037F;
	data << (uint32) 0x00040F03 << (uint32) 0x05310E00;
	data << (uint8)  0x00;
	data << (uint32) 0x05480800 << (uint32) 0x70500000;
	data << (uint32) 0x0C000005;
	data.Finalize();
	SendPacket(&data);

	/* Packet # 21 */
	data.clear();
	data.SetOpcode(0x23);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << (uint32) 0x0003E804 << (uint32) 0x00000000;
	data << (uint32) 0x209C3800 << (uint32) 0x00000000;
	data << (uint8)  0x00;
	data << (uint32) 0x00CB46E3 << (uint32) 0x40F40000;
	data.Finalize();
	SendPacket(&data);

	return;
}

void WorldSocket::_ChangeMap(const uint16 mapid, const uint16 pos_x, const uint16 pos_y)
{
	WorldPacket data(1);
	data.clear();
	data.SetOpcode(0x14);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << (uint8) 0x07;
	data.Finalize();
	SendPacket(&data);

	data.clear();
	data.SetOpcode(0x29);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << (uint8) 0x14;
	data.Finalize();
	SendPacket(&data);

	data.clear();
	data.SetOpcode(0x0C);
	data << data.GetHeader() << data.GuessSize() << data.GetOpcode();
	data << (uint8) 0x6D << (uint8) 0x82 << (uint8) 0x03 << (uint8) 0x00;
	data << (uint16) mapid;
	data << (uint16) pos_x;
	data << (uint16) pos_y;
	data << (uint16) 0x0001;
	data.Finalize();
	SendPacket(&data);

}

void WorldSocket::_EndForNow()
{
	WorldPacket data(1);
	data.clear();
	data.SetOpcode(ENCODE((uint8) 171));
	data.Prepare();
	data << ENCODE((uint8) 175);
	SendPacket(&data);

	data.clear();
	data.SetOpcode(ENCODE((uint8) 185));
	data.Prepare();
	data << ENCODE((uint8) 165);
	SendPacket(&data);
}

void WorldSocket::_SendFixedPacket()
{
}

void WorldSocket::_HandleAddOnPacket()
{
}
	
/// Send Player Info
void WorldSocket::_HandlePlayerInfo(uint32 accountId, std::string password)
{
	WorldPacket data(1);

	/* MOTD */
	// 89 233 len-int16 175 166 173 173 173 173 getStr("Motd")
	std::string motd;
	motd = "Welcome to LeGACY - MMORPG Server Object.";
	data.clear();
	data.SetOpcode(2);
	data << data.GetHeader();
	data << uint16(0); // guess
	data << uint8(2);
	data << uint8(11);
	data << uint32(0);
	data << motd;
	data.Finalize();
	SendPacket(&data);

	motd = "This server is still under heavy development.";
	data.clear();
	data.SetOpcode(2);
	data << data.GetHeader();
	data << uint16(0); // guess
	data << uint8(2);
	data << uint8(11);
	data << uint32(0);
	data << motd;
	data.Finalize();
	SendPacket(&data);
	/* end MOTD */

	/* Voucher Point */
	// 89 233 183 173 142 169 Point2-uint32 173 173 173 173
	// 149 49 141 173 173 173 173 173 78 235 102 173 173 173 89 237
	uint32 point;
	point = 1000;
	data.clear();
	data.SetOpcode(35);
	data << data.GetHeader();
	data << uint16(0); // guess
	data << uint8(37);
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
	data.Finalize();
	SendPacket(&data);

	/* end Voucher Point */

}

/// Handle the authentication waiting queue (to be completed)
void WorldSocket::SendAuthWaitQue(uint32 position)
{
	printf("WorldSocket::SendAuthWaitQue called\n");
}

