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
#include "Chat.h"
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
	sLog.outString("LoginQueryHolder::Initialize");
	SetSize(MAX_PLAYER_LOGIN_QUERY);

	bool res = true;

	res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADFROM, "SELECT * FROM characters WHERE accountid = '%u'", m_accountId);

	res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADPET, "SELECT guid, petslot FROM character_pet WHERE owner_guid = '%u' ORDER BY petslot", GUID_LOPART(m_guid));

	res &= SetPQuery(PLAYER_LOGIN_QUERY_LOADINVENTORY, "SELECT data, pet, slot, item, entry FROM character_inventory ci JOIN item_instance ii ON ci.item = ii.guid WHERE ci.guid = '%u' ORDER BY ci.slot", GUID_LOPART(m_guid));

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

	recv_data >> new_name;

	if(objmgr.GetPlayerGUIDByName(new_name))
	{
		data.Initialize( 0x09 );
		data << (uint8 ) 0x03;
		data << (uint8 ) 0x01;
		SendPacket(&data);
		return;
	}

	///- Name ok
	data.Initialize( 0x09 );
	data << (uint8 ) 0x03;
	data << (uint8 ) 0x00;
	SendPacket(&data);





	///- TODO: delete below this after fix
	return;
/*
	///- Delete possible re-choosing name after marking previous one
	CharacterDatabase.PExecute("DELETE FROM characters WHERE accountid = %u", GetAccountId());

	CharacterDatabase.escape_string(name);
	QueryResult* result = CharacterDatabase.PQuery("SELECT name FROM characters WHERE name = '%s' and accountid != %u", name.c_str(), GetAccountId());

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
*/
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

	Player * pNewChar = new Player(this);

	// prevent character creating with invalid name
	if(new_name.empty())
	{
		sLog.outError("Account:[%d] but tried to Create character with empty [name]", GetAccountId());
		return;
	}

	std::string pass1;
	std::string pass2;

	if(pNewChar->Create( objmgr.GenerateLowGuid(HIGHGUID_PLAYER), new_name, recv_data, pass1, pass2 ))
	{
		Item* shirt = pNewChar->CreateItem( 864, 1 );
		if( shirt )
		{
			uint8 slot = shirt->GetProto()->EquipmentSlot;
			uint8 dest = slot;
			if( pNewChar->CanEquipItem( slot, dest, shirt, false ) == EQUIP_ERR_OK )
				pNewChar->EquipItem( dest, shirt, false );
			else
				delete shirt;
		}

		///- Adding default pet (Guo Jia evo)
		Pet* pet = pNewChar->CreatePet( 3402 );
		if( pet )
		{
			uint8 slot = 0;
			uint8 dest = slot;
			if( pNewChar->CanSummonPet( slot, dest, pet, false ) == PET_ERR_OK )
				pNewChar->SummonPet( 0, pet );
			else
				delete pet;
		}

		pNewChar->SaveToDB();

		delete pNewChar;
	}
	else
	{
		// Player not create (problem?)
		delete pNewChar;

		///- TODO: Send char create error to client
		return;
	}

	std::string md5pass1 = md5(pass1);
	std::string md5pass2 = md5(pass2);

	///- Commit pending player creation transaction, for login preparation
	loginDatabase.PExecute("UPDATE accounts SET md5pass1 = '%s', md5pass2 = '%s' WHERE accountid = %u", md5pass1.c_str(), md5pass2.c_str(), GetAccountId());

	CharacterDatabase.CommitTransaction();

	///- do a small delay to make sure player save to database completed
	ZThread::Thread::sleep(10);

	std::string IP_str = _socket ? _socket->GetRemoteAddress().c_str() : "-";
	sLog.outString("Account: %d (IP: %s) Create Character:[%s]", GetAccountId(), IP_str.c_str(), new_name.c_str());
	sLog.outChar("Account: %d (IP: %s) Create Character:[%s]", GetAccountId(), IP_str.c_str(), new_name.c_str());


	///- Clear new name after new player creation
	new_name = "";

	WorldPacket data;
	data << (uint8 ) pass1.size();
	data << (uint32) GetAccountId();
	data << (uint32) 0x00;
	data << pass1;

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
	{
		sLog.outError("ERROR: Player guid not found for account %u", accountId);
		return;
	}

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

	// Send MOTD
	{
		uint32 linecount = 0;
		std::string str_motd = sWorld.GetMotd();
		std::string::size_type pos, nextpos;
		pos = 0;
		while( (nextpos = str_motd.find('$',pos)) != std::string::npos )
		{
			if( nextpos != pos )
			{
				ChatHandler(this).SendSysMessage(str_motd.substr(pos,nextpos-pos).c_str());
				linecount++;
			}
			pos = nextpos+1;
		}
		if( pos < str_motd.length() )
		{
			ChatHandler(this).SendSysMessage(str_motd.substr(pos).c_str());
			linecount++;
		}
		DEBUG_LOG( "WORLD: Sent motd(SMSG_MOTD)" );
	}

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
		{
			sLog.outDebug("WORLD: Recv CMSG_PLAYER_ENTER_DOOR repeat, ignored");
			GetPlayer()->EndOfRequest();
			break;
		}
		case CMSG_PLAYER_AREA_TRIGGER:
		{
			///- TODO: Check for area trigger event, else warp
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

	///- TODO: Add enable/disable command
	///- Temporary update door position, please disable after matrix is complete
	if(player->GetSession()->GetSecurity() > SEC_GAMEMASTER &&
		(::strcmp(player->GetName(), "Eleven") == 0 ||
		 ::strcmp(player->GetName(), "Valinor") == 0 ))
	{
		WorldDatabase.PExecute("UPDATE map_matrix set x = %u, y = %u WHERE mapid_src = %u AND mapid_dest = %u", player->GetLastPositionX(), player->GetLastPositionY(), mapDest->MapId, mapid);
		//WorldDatabase.PExecute("UPDATE map_matrix set x = %u, y = %u WHERE mapid_src = %u AND mapid_dest = %u AND x = 0 AND y = 0", player->GetLastPositionX(), player->GetLastPositionY(), mapDest->MapId, mapid);

		///- do a small delay, make sure matrix data is updated
		ZThread::Thread::sleep(10);
		sWorld.RefreshDoorDatabase();
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
