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
#include "GossipDef.h"
#include "ScriptCalls.h"
#include "ObjectAccessor.h"
#include "Creature.h"
#include "MapManager.h"














void WorldSession::HandleGossipHelloOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 1);

	sLog.outDebug( "WORLD: Received CMSG_GOSSIP_HELLO" );

	uint8  id;
	uint32 guid;
	recv_data >> id;

	sLog.outString("Player '%s' trying to interact with %u in %u", GetPlayer()->GetName(), id, GetPlayer()->GetMapId());

	uint32 tmp_guid = GetPlayer()->GetMapId() * 100;
	guid = tmp_guid + id;

	Creature *unit = ObjectAccessor::GetNPCIfCanInteractWith(*_player, guid, UNIT_NPC_FLAG_NONE);

	if(!unit)
	{
		sLog.outDebug( "WORLD: HandleGossipHelloOpcode - Unit (GUID: %u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );

		_player->PlayerTalkClass->SendTalking( DEFAULT_GOSSIP_MESSAGE, id );
		return;
	}

//	sLog.outString("Unit interactor is GUID(%u) MAPID(%u) NAME(%s)", unit->GetGUIDLow(), unit->GetMapId(), unit->GetName());
	if(!Script->GossipHello( _player, unit ))
	{
		_player->TalkedToCreature(unit->GetEntry(), unit->GetGUID());
		unit->prepareGossipMenu(_player, 0);
		unit->sendPreparedGossip( _player );
	}
	/*
	WorldPacket data;
	data.clear(); data.SetOpcode( 0x06 ); data.Prepare();
	data << (uint8) 0x02;
	_player->GetSession()->SendPacket(&data);

	data.clear(); data.SetOpcode( 0x14 ); data.Prepare();
	data << (uint8 ) 0x01 << (uint32) 0x01000000;
	data << (uint16) 0x0301;
	data << (uint8 ) id; // 0x02; npc id
	data << (uint8 ) 0x00;
	data << (uint16) 0x0000 << (uint16) 0x0000;
	data << (uint8 ) 0x00;
	data << (uint16) 0x3061; // dialog id
	_player->GetSession()->SendPacket(&data);

	data.clear(); data.SetOpcode( 0x05 ); data.Prepare();
	data << (uint32) 0x025FFE02 << (uint16) 0x3900 << (uint8) 0x4A;
	_player->GetSession()->SendPacket(&data);


	data.clear(); data.SetOpcode( 0x0B ); data.Prepare();
	data << (uint32) 0x809D0204 << (uint32) 0x00000002 << (uint8) 0x01;
	_player->GetSession()->SendPacket(&data);

	data.clear(); data.SetOpcode( 0x0B ); data.Prepare();
	data << (uint32) 0xAAE80204 << (uint32) 0x00000000 << (uint8) 0x05;
	_player->GetSession()->SendPacket(&data);

	data.clear(); data.SetOpcode( 0x0B ); data.Prepare();
	data << (uint32) 0x9B0C0204 << (uint32) 0x00000002 << (uint8) 0x05;
	_player->GetSession()->SendPacket(&data);

	data.clear(); data.SetOpcode( 0x06 ); data.Prepare();
	data << (uint32) 0x02A02901 << (uint32) 0x01E20600 << (uint16) 0x0177;
	_player->GetSession()->SendPacket(&data);
	*/
}

