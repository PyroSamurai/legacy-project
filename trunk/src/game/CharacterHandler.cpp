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
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSocket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Database/DatabaseImpl.h"
#include "Encoder.h"

// check used symbols in player name at creating and rename
std::string notAllowedChars = "\t\v\b\f\a\n\r\\\"\'\? <>[](){}_=+-|/!@#$%^&*~`.,0123456789\0";

class LoginQueryHolder : public SqlQueryHolder
{
	private:
		uint32 m_accountId;
	public:
		LoginQueryHolder(uint32 accountId)
			: m_accountId(accountId) { }
		uint32 GetAccountId() const { return m_accountId; }
		bool Initialize();
};

bool LoginQueryHolder::Initialize()
{
//	sLog.outString("LoginQueryHolder::Initialize");
	SetSize(MAX_PLAYER_LOGIN_QUERY);

	bool res = true;

	res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADFROM, "SELECT * FROM characters WHERE accountid = '%u'", m_accountId);

	return res;
}



// don't call WorldSession directly
// it may get deleted before the query callbacks get executed
// instead pass an account id to this handler
class CharacterHandler
{
	public:
		void HandlePlayerLoginCallback(QueryResult * /*dummy*/, SqlQueryHolder * holder)
		{
//			sLog.outString("CharacterHandler::HandlePlayerLoginCallback");
			if(!holder) return;
			WorldSession *session = sWorld.FindSession(((LoginQueryHolder*)holder)->GetAccountId());
			if(!session)
			{
				sLog.outString("Session not found for %u",
					((LoginQueryHolder*)holder)->GetAccountId());
				delete holder;
				return;
			}
			session->HandlePlayerLogin((LoginQueryHolder*)holder);
		}
} chrHandler;

void WorldSession::HandleCharCreateOpcode( WorldPacket & recv_data )
{
	sLog.outString( "WORLD: Recv CSMG_CHAR_CREATE Message" );
}

void WorldSession::HandlePlayerLoginOpcode( WorldPacket & recv_data )
{
	//TODO: CHECK_PACKET_SIZE(recv_data, 8);
	
	m_playerLoading = true;

	sLog.outDebug( "WORLD: Recvd CMSG_AUTH_RESPONSE Message" );

	uint8       lenPassword;
	uint32      accountId;
	uint32      patchVer;
	std::string password;

	recv_data >> lenPassword;
	recv_data >> accountId;
	recv_data >> patchVer;
	recv_data >> password;

	LoginQueryHolder *holder = new LoginQueryHolder(accountId);
	if(!holder->Initialize())
	{
		delete holder;    // delete all unprocessed queries
		m_playerLoading = false;
		return;
	}

	CharacterDatabase.DelayQueryHolder(&chrHandler, &CharacterHandler::HandlePlayerLoginCallback, holder);

}

void WorldSession::HandlePlayerLogin(LoginQueryHolder * holder)
{
	sLog.outDetail("Creating Player");
	uint32 accountId = holder->GetAccountId();

	Player* pCurrChar = new Player(this);
	//pCurrChar->GetMotionMaster()->Initialize();

	if(!pCurrChar->LoadFromDB(accountId, holder))
	{
		delete pCurrChar;  // delete it manually
		delete holder;     // delete all unprocessed queries
		m_playerLoading = false;
		return;
	}
	else
		SetPlayer(pCurrChar);

	WorldPacket data;

	data.clear();
	data.SetOpcode(0x14); data.Prepare(); data << (uint8) 0x08;
	SendPacket(&data);

	data.clear();
	data.SetOpcode(0x18); data.Prepare();
	data << (uint8) 0x05 << (uint8) 0x02 << (uint16) 0x0000;
	SendPacket(&data);

	data.clear();
	data.SetOpcode(0x18); data.Prepare();
	data << (uint8) 0x05 << (uint8) 0x31 << (uint16) 0x0001;
	SendPacket(&data);

	data.clear();
	data.SetOpcode(0x18); data.Prepare();
	data << (uint8) 0x05 << (uint8) 0x62 << (uint16) 0x0001;
	SendPacket(&data);

	data.clear();
	data.SetOpcode(0x18); data.Prepare();
	data << (uint8) 0x05 << (uint8) 0x91 << (uint16) 0x0001;
	SendPacket(&data);

	data.clear();
	data.SetOpcode(0x18); data.Prepare();
	data << (uint8) 0x05 << (uint8) 0x92 << (uint16) 0x0001;
	SendPacket(&data);

	sLog.outDebug("Adding player %s to Map.", pCurrChar->GetName());

	pCurrChar->SendInitialPacketsAfterAddToMap();

	Map* map = MapManager::Instance().GetMap(pCurrChar->GetMapId(), pCurrChar);
	map->Add(pCurrChar);

	ObjectAccessor::Instance().AddObject(pCurrChar);

	//pCurrChar->SendInitialPacketsAfterAddToMap();

	CharacterDatabase.PExecute("UPDATE characters SET online = 1 WHERE accountid = '%u'", accountId);

	std::string IP_str = _socket ? _socket->GetRemoteAddress().c_str() : "-";
	sLog.outString("Account: %d (IP: %s) Login Character:[%s] (guid:%u)", GetAccountId(), IP_str.c_str(), pCurrChar->GetName(), pCurrChar->GetGUID());

//	pCurrChar->SetInGameTime( getMSTime() );

//	sLog.outString("Map '%u' has '%u' Players", pCurrChar->GetMapId(), map->GetPlayersCount());

	m_playerLoading = false;
	delete holder;
}

void WorldSession::HandlePlayerActionOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 1);

	sLog.outDebug( "WORLD: Recvd CMSG_PLAYER_ACTION Message" );

	uint8 subOpcode;
	recv_data >> subOpcode;

	switch( subOpcode )
	{

		case 0x01:
		{
			sLog.outDebug( "WORLD: Recvd CMSG_PLAYER_CLICK_NPC Message" );
			GetPlayer()->EndOfRequest();
			break;
		}
		case 0x02:
		{
			sLog.outDebug( "WORLD: Recvd CMSG_UNKNOWN_2 Message" );
			GetPlayer()->Send0602();
			GetPlayer()->EndOfRequest();
			break;
		}
		case 0x06:
		{
			//GetPlayer()->SendMapChanged();
			break;
		}

		case CMSG_PLAYER_ENTER_DOOR:
		case CMSG_PLAYER_ENTER_DOOR2:
		{
			HandlePlayerEnterDoorOpcode( recv_data );
			break;
		}

		default:
		{
			sLog.outDebug( "WORLD: Unhandled CMSG_PLAYER_ACTION Message" );
			GetPlayer()->EndOfRequest();
		}
	}
}

void WorldSession::HandlePlayerEnterDoorOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 1+1);

	sLog.outDebug( "WORLD: Recvd CMSG_PLAYER_ENTER_DOOR Message" );

	WorldPacket data;
	uint16 mapid;
	uint8  doorid;
	Player* player;

	recv_data >> doorid;

	player = GetPlayer();

	mapid = player->GetMapId();
	MapDoor*        mapDoor = new MapDoor(mapid, doorid);
	MapDestination* mapDest = MapManager::Instance().FindMap2Dest(mapDoor);
	delete mapDoor;

	if( !mapDest ) {
		DEBUG_LOG( "Destination map not found, aborting" );
		player->EndOfRequest();
		return;
	}

	///- Temporary update door position
	CharacterDatabase.PExecute("UPDATE map2map set x = %u, y = %u WHERE mapid_src = %u AND mapid_dest = %u AND x = 0 AND y = 0", player->GetLastPositionX(), player->GetLastPositionY(), mapDest->MapId, mapid);

	///- Send Enter Door action response
	data.clear(); data.SetOpcode( CMSG_PLAYER_ACTION ); data.Prepare();
	data << (uint8) 0x07;
	player->GetSession()->SendPacket(&data);
	
	data.clear(); data.SetOpcode( 0x29 ); data.Prepare();
	data << (uint8) 0x0E;
	player->GetSession()->SendPacket(&data);

	player->TeleportTo(mapDest->MapId, mapDest->DestX, mapDest->DestY);
	GetPlayer()->SendMapChanged();

	//sWorld.RefreshDoorDatabase();
}

void WorldSession::HandlePlayerEnterMapCompletedOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 1);

	sLog.outDebug( "WORLD: Recvd CMSG_PLAYER_ENTER_MAP_COMPLETED Message" );
	uint8 subOpcode;
	recv_data >> subOpcode;
	switch (subOpcode)
	{
		case 0x01:
		{
			GetPlayer()->EndOfRequest();
			GetPlayer()->UpdateRelocationToSet();
			break;
		}

		default:
		{
			sLog.outDebug( "WORLD: Unhandled CMSG_PLAYER_ENTER_MAP_COMPLETED Message" );
			GetPlayer()->EndOfRequest();
		}
	}
}

void WorldSession::HandlePlayerExpressionOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 1+1);

	uint8 expressionType;
	uint8 expressionCode;
	recv_data >> expressionType;
	recv_data >> expressionCode;

	WorldPacket data;
	data.clear(); data.SetOpcode( 0x20 ); data.Prepare();
	data << expressionType;
	data << GetPlayer()->GetAccountId();
	data << expressionCode;

	GetPlayer()->SendMessageToSet(&data, false);
}

void WorldSession::HandleUnknownRequest14Opcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 1);

	uint8 subOpcode;
	recv_data >> subOpcode;
	if ( subOpcode != 0x08 )
		return;

	sLog.outDetail( "WORLD: Recv CMSG_UKNOWN_14 Message" );
//	GetPlayer()->EndOfRequest();
}
