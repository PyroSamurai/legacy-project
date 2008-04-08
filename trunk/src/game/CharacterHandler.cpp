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
#include "GossipDef.h"

#include "BattleSystem.h"

// check used symbols in player name at creating and rename
std::string notAllowedChars = "\t\v\b\f\a\n\r\\\"\'\? <>[](){}_=+-|/!@#$%^&*~`.,0123456789\0";

class LoginQueryHolder : public SqlQueryHolder
{
	private:
		uint32 m_accountId;
		uint64 m_guid;
	public:
		LoginQueryHolder(uint32 accountId, uint64 guid)
			: m_accountId(accountId), m_guid(guid) { }
		uint64 GetGuid() const { return m_guid; }
		uint32 GetAccountId() const { return m_accountId; }
		bool Initialize();
};

bool LoginQueryHolder::Initialize()
{
//	sLog.outString("LoginQueryHolder::Initialize");
	SetSize(MAX_PLAYER_LOGIN_QUERY);

	bool res = true;

	res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADFROM, "SELECT * FROM characters WHERE accountid = '%u'", m_accountId);

	res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADPET, "SELECT id, petslot FROM character_pet WHERE owner = '%u' ORDER BY petslot", GUID_LOPART(m_guid));

	res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADINVENTORY, "SELECT entry, pet, slot, item, item_template, item_instance.* FROM character_inventory JOIN item_instance ON character_inventory.item = item_instance.guid WHERE character_inventory.guid = '%u' ORDER BY slot", GUID_LOPART(m_guid));

	res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADSPELL, "SELECT entry, level FROM character_spell where owner = '%u' ORDER BY entry", GUID_LOPART(m_guid));

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
			sLog.outString("CharacterHandler::HandlePlayerLoginCallback");
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

void WorldSession::HandleCharCreateSelectName(WorldPacket & recv_data)
{
	CHECK_PACKET_SIZE(recv_data, 1);
	WorldPacket data;
	std::string name;

	recv_data >> name;

	CharacterDatabase.escape_string(name);
	QueryResult* result = CharacterDatabase.PQuery("SELECT * FROM characters WHERE name = '%s'", name.c_str());

	SetLogging(false);
	///- Name already taken
	if( result )
	{
		sLog.outDebug("Name '%s' exists", name.c_str());
		delete result;
		data.Initialize( 0x09 );
		data << (uint8 ) 0x03;
		data << (uint8 ) 0x01;
		SendPacket(&data);
		return;
	}

	delete result;
	///- Name ok
	
	CharacterDatabase.PExecute("INSERT INTO characters (accountid, name) values (%u, '%s')", GetAccountId(), name.c_str());

	data.Initialize( 0x09 );
	data << (uint8 ) 0x03;
	data << (uint8 ) 0x00;
	SendPacket(&data);
}

void WorldSession::HandleCharCreateOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 1);

	uint8 subOpcode;

	recv_data >> subOpcode;

	sLog.outString( "WORLD: Recv CSMG_CHAR_CREATE Message" );

	switch( subOpcode )
	{
		case 0x01:
			HandleCharCreate( recv_data );
			break;

		case 0x02:
			HandleCharCreateSelectName( recv_data );
			break;
	}
}

void WorldSession::HandleCharCreate( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 1+1+1+1+2+2+2+2+1+1+1+1+1+1+1+1);
	
	uint8  gender = 0;
	uint8  face = 0;
	uint8  hair = 0;
	uint8  reborn = 0;

	uint8  skin_color_R = 0;
	uint8  skin_color_G = 0;
	uint8  skin_color_B = 0;
	uint8  hair_color_R = 0;
	uint8  hair_color_G = 0;
	uint8  hair_color_B = 0;
	uint8  shirt_color = 0;
	uint8  misc_color = 0;

	uint8  element = 0;
	uint8  stat_int = 0;
	uint8  stat_atk = 0;
	uint8  stat_def = 0;
	uint8  stat_hpx = 0;
	uint8  stat_spx = 0;
	uint8  stat_agi = 0;

	uint8  lenPassword1 = 0;
	std::string password1 = "";
	uint8  lenPassword2 = 0;
	std::string password2 = "";

	uint8 tmp_password1[100];
	uint8 tmp_password2[100];

	recv_data >> gender;
	recv_data >> face;
	recv_data >> hair;
	recv_data >> reborn;

	recv_data >> hair_color_R;
	recv_data >> hair_color_G;
	recv_data >> hair_color_B;
	recv_data >> skin_color_R;
	recv_data >> skin_color_G;
	recv_data >> skin_color_B;
	recv_data >> shirt_color;
	recv_data >> misc_color;

	sLog.outDebug("Coloring: hair: %u, skin %u, shirt %u, misc %u", hair_color_R, skin_color_R, shirt_color, misc_color);
	recv_data >> element;

	recv_data >> stat_int;
	recv_data >> stat_atk;
	recv_data >> stat_def;
	recv_data >> stat_hpx;
	recv_data >> stat_spx;
	recv_data >> stat_agi;

	recv_data >> lenPassword1;

	CHECK_PACKET_SIZE( recv_data, lenPassword1 );

	for(uint8 i = 0; i < lenPassword1; i++)
	{
		recv_data >> tmp_password1[i];
		password1 += tmp_password1[i];
	}
	sLog.outDebug("Password1: '%s'", password1.c_str());

	recv_data >> lenPassword2;

	CHECK_PACKET_SIZE( recv_data, lenPassword2 );
	for(uint8 i = 0; i < lenPassword2; i++)
	{
		recv_data >> tmp_password2[i];
		password2 += tmp_password2[i];
	}
	sLog.outDebug("Password2: '%s'", password2.c_str());

	QueryResult* result = CharacterDatabase.PQuery("SELECT name FROM characters WHERE accountid = %u", GetAccountId());
	if( !result )
		return;

	std::string name = (*result)[0].GetCppString();

	delete result;

	loginDatabase.PExecute("DELETE FROM accounts WHERE accountid = %u", GetAccountId());
	loginDatabase.PExecute("INSERT INTO accounts (accountid, password, online) values (%u, md5('%s'), 1)", GetAccountId(), password1.c_str());

	CharacterDatabase.PExecute("DELETE FROM characters WHERE accountid = %u AND name = '%s'", GetAccountId(), name.c_str());

	std::ostringstream ss;
	ss << "INSERT INTO characters (accountid, psswd, gender, element, face, hair, name, reborn, level, mapid, pos_x, pos_y, stat_int, stat_atk, stat_def, stat_hpx, stat_spx, stat_agi, hair_color_R, hair_color_G, hair_color_B, skin_color_R, skin_color_G, skin_color_B, shirt_color, misc_color ) values ( "
		<< GetAccountId() << ", md5('"
		<< password1 << "'), "
		<< uint32(gender) << ", "      // to prevent save uint8 as char
		<< uint32(element) << ", "
		<< uint32(face) << ", "
		<< uint32(hair) << ", '"
		<< name.c_str() << "', "
		<< uint32(reborn) << ", "
		<< uint32(1) << ", "
		<< uint32(10816) << ", "
		<< uint32(442) << ", "
		<< uint32(758) << ", "
		<< uint32(stat_int) << ", "
		<< uint32(stat_atk) << ", "
		<< uint32(stat_def) << ", "
		<< uint32(stat_hpx) << ", "
		<< uint32(stat_spx) << ", "
		<< uint32(stat_agi) << ", "
		<< uint32(hair_color_R) << ", "
		<< uint32(hair_color_G) << ", "
		<< uint32(hair_color_B) << ", "
		<< uint32(skin_color_R) << ", "
		<< uint32(skin_color_G) << ", "
		<< uint32(skin_color_B) << ", "
		<< uint32(shirt_color) << ", "
		<< uint32(misc_color) << " )";

	CharacterDatabase.Execute( ss.str().c_str() );

	WorldPacket data;
	data << lenPassword1;
	data << GetAccountId();
	data << (uint32) 0x00;
	data << password1;

	HandlePlayerLoginOpcode( data );
}

void WorldSession::HandlePlayerLoginOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 9);
	
	m_playerLoading = true;
	uint64 playerGuid = 0;

	sLog.outDebug( "WORLD: Recvd CMSG_AUTH_RESPONSE Message" );

	uint8       lenPassword;
	uint32      accountId;
	uint32      patchVer;
	std::string password;

	recv_data >> lenPassword;
	CHECK_PACKET_SIZE(recv_data, 8+lenPassword);
	recv_data >> accountId;
	recv_data >> patchVer;
	recv_data >> password;

	QueryResult* result = CharacterDatabase.PQuery("SELECT guid FROM characters WHERE accountid = '%u'", accountId);

	if( !result )
		return;

	playerGuid = (*result)[0].GetUInt32();
	delete result;

	LoginQueryHolder *holder = new LoginQueryHolder(accountId, playerGuid);
	if(!holder->Initialize())
	{
		sLog.outDebug("HOLDER: Deleting holder");
		delete holder;    // delete all unprocessed queries
		m_playerLoading = false;
		return;
	}

	CharacterDatabase.DelayQueryHolder(&chrHandler, &CharacterHandler::HandlePlayerLoginCallback, holder);

}

void WorldSession::HandlePlayerLogin(LoginQueryHolder * holder)
{
	sLog.outDebug( "WORLD: HandlePlayerLogin" );

	uint64 playerGuid = holder->GetGuid();

	Player* pCurrChar = new Player(this);
	//pCurrChar->GetMotionMaster()->Initialize();

	if(!pCurrChar->LoadFromDB(GUID_LOPART(playerGuid), holder))
	{
		delete pCurrChar;  // delete it manually
		delete holder;     // delete all unprocessed queries
		m_playerLoading = false;
		return;
	}
	else
		SetPlayer(pCurrChar);

	sLog.outDebug("** Adding player %s to Map.", pCurrChar->GetName());

	pCurrChar->SendInitialPacketsBeforeAddToMap();

	Map* map = MapManager::Instance().GetMap(pCurrChar->GetMapId(), pCurrChar);
	map->Add(pCurrChar);

	ObjectAccessor::Instance().AddObject(pCurrChar);

	pCurrChar->SendInitialPacketsAfterAddToMap();

	CharacterDatabase.PExecute("UPDATE characters SET online = 1 WHERE guid = '%u'", pCurrChar->GetGUIDLow());

	std::string IP_str = _socket ? _socket->GetRemoteAddress().c_str() : "-";
	sLog.outString("** Account: %d (IP: %s) Login Character:[%s] (guid:%u)", GetAccountId(), IP_str.c_str(), pCurrChar->GetName(), pCurrChar->GetGUID());

//	pCurrChar->SetInGameTime( getMSTime() );

//	sLog.outString("Map '%u' has '%u' Players", pCurrChar->GetMapId(), map->GetPlayersCount());



	m_playerLoading = false;
	delete holder;

	WorldPacket data(0, 1);
	data << (uint8) 1;
//	HandlePlayerClickNpc( data );

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
			HandlePlayerClickNpc( recv_data );
			break;
		}
		case 0x02:
		{
			///- Area/Npc near Triggerred
			// ignore for now
			GetPlayer()->EndOfRequest();
			break;
		}
		case 0x06:
		{
			///- Close Dialog if not talking to anyone
			if(!GetPlayer()->PlayerTalkClass->IsMenuOpened())
			{
				GetPlayer()->EndOfRequest();
			}

			GetPlayer()->PlayerTalkClass->CloseMenu();
			// Send prepared dialog if any
			//GetPlayer()->PlayerTalkClass->SendPreparedDialog();
			//GetPlayer()->PlayerTalkClass->SetTalking(false);
			break;
		}

		case CMSG_PLAYER_ENTER_DOOR:
		case CMSG_PLAYER_AREA_TRIGGER:
		{
			HandlePlayerEnterDoorOpcode( recv_data );
			break;
		}

		case 0x09:
		{
			HandlePlayerSelectDialogOpcodes( recv_data );
			break;
		}

		default:
		{
			///- Unhandled, preventing player from getting stuck for now
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
	MapDestination* mapDest = MapManager::Instance().FindMapMatrix(mapDoor);
	delete mapDoor;

	if( !mapDest ) {
		DEBUG_LOG( "Destination map not found, aborting" );
		player->EndOfRequest();
		return;
	}

	///- Temporary update door position
	if(player->GetName() == "Eleven")
	{
		WorldDatabase.PExecute("UPDATE map_matrix set x = %u, y = %u WHERE mapid_src = %u AND mapid_dest = %u", player->GetLastPositionX(), player->GetLastPositionY(), mapDest->MapId, mapid);
		//WorldDatabase.PExecute("UPDATE map_matrix set x = %u, y = %u WHERE mapid_src = %u AND mapid_dest = %u AND x = 0 AND y = 0", player->GetLastPositionX(), player->GetLastPositionY(), mapDest->MapId, mapid);
	}

	///- Send Enter Door action response
	data.Initialize( CMSG_PLAYER_ACTION, 1 );
	data << (uint8) 0x07;
	player->GetSession()->SendPacket(&data);
	
	data.Initialize( 0x29, 1 );
	data << (uint8) 0x0E;
	player->GetSession()->SendPacket(&data);

	player->TeleportTo(mapDest->MapId, mapDest->DestX, mapDest->DestY);
	GetPlayer()->SendMapChanged();

	if(player->GetName() == "Eleven")
		sWorld.RefreshDoorDatabase();
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
	data.Initialize( 0x20, 1 );
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
