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
#include "BattleSystem.h"
#include "Chat.h"

void WorldSession::HandlePlayerClickNpc( WorldPacket & recv_data )
{
	if( _player->isDead() )
		return;

	CHECK_PACKET_SIZE(recv_data, 1);

	sLog.outDebug( "WORLD: Recvd CMSG_PLAYER_CLICK_NPC Message" );

	uint16 mapid = GetPlayer()->GetMapId();
	uint8  map_npcid;
	uint64 guid;

	recv_data >> map_npcid;

	///- Temporary guid mapping for npc
	//uint32 tmp_guid = mapid * MAP_NPCID_MULTIPLIER;
	//guid = MAKE_GUID(tmp_guid + map_npcid, HIGHGUID_UNIT);
	guid = objmgr.GetNpcGuidByMapNpcId(mapid, map_npcid);

	sLog.outString(" >> Player '%s' try to interact with %u in %u", GetPlayer()->GetName(), GUID_LOPART(guid), mapid);

	Creature* unit = ObjectAccessor::GetNPCIfCanInteractWith(*_player, guid, UNIT_NPC_FLAG_NONE);

	if(!unit)
	{
		sLog.outDebug( "WORLD: HandlePlayerClickNpc - Unit(GUID:%u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
		_player->PlayerTalkClass->SendTalking( map_npcid, DEFAULT_GOSSIP_MESSAGE );
		if(GetSecurity() > SEC_PLAYER)
			ChatHandler(this).PSendSysMessage("WORLD: '<none>' guid %u map %u id %u", GUID_LOPART(guid), mapid, map_npcid);
		return;
	}

	if(GetSecurity() > SEC_PLAYER)
		ChatHandler(this).PSendSysMessage("WORLD: '%s' guid %u map %u id %u", unit->GetName(), GUID_LOPART(guid), mapid, map_npcid);

	if( unit->isDead() )
		return;

	if( unit->isGossip() || unit->isQuestGiver() || unit->isServiceProvider())
	{
		///- Handle Dialog
		if(!Script->GossipHello( _player, unit ))
		{
			sLog.outDebug("GOSSIP: No script defined. Use core handling");
			_player->TalkedToCreature( unit->GetEntry(), unit->GetGUID());
			unit->prepareGossipMenu(_player, 0);
			unit->sendPreparedGossip( _player );
		}
		return;
	}

	///- Creature is a monster, engage the Creature
	_player->Engage( unit );
}

void WorldSession::HandlePlayerSelectDialogOpcodes( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE(recv_data, 1);

	sLog.outDebug( "WORLD: Recvd CMSG_PLAYER_SELECT_DIALOG Message" );

	uint8  option;
	recv_data >> option;

	uint64 guid = _player->GetTalkedCreatureGuid();
	Creature* unit = ObjectAccessor::GetNPCIfCanInteractWith(*_player, guid, UNIT_NPC_FLAG_NONE);

	if(!unit)
	{
		sLog.outDebug( "WORLD: HandleSelectDialogOpcodes - Unit(GUID:%u) not found or you can't interact with him.", uint32(GUID_LOPART(guid)) );
		if(GetSecurity() > SEC_PLAYER)
			ChatHandler(this).PSendSysMessage("WORLD: '<none>' guid %u", GUID_LOPART(guid));
		return;
	}

	sLog.outString(" >> Player '%s' select option %u", GetPlayer()->GetName(), option);


	uint32 sequence = _player->GetTalkedSequence();
	///- Handle Select Dialog
	if(!Script->GossipSelect( _player, unit, sequence, option ))
	{
		unit->OnGossipSelect( _player, sequence, option );
	}
}
