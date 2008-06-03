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

#include "BattleSystem.h"
#include "Common.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectAccessor.h"

void WorldSession::HandlePlayerBattleCommandOpcodes( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE( recv_data, 1+1+4+2 );

	DEBUG_LOG( "WORLD: Recvd CMSG_PLAYER_BATTLE_COMMAND Message" );

	uint8  cmd;
	uint8  type;
	uint32 target;
	uint16 map_npcid;

	recv_data >> cmd;

	switch( cmd )
	{
		case 0x02: // PK
		{
			recv_data >> type;
			recv_data >> target;
			recv_data >> map_npcid;
			switch( type )
			{
				case 0x02: // To Player
				{
					if( !_player->isAcceptPK() )
						return;

					Player* enemy = objmgr.GetPlayerByAccountId(target);
					if( !enemy )
						return;

					if( !enemy->isAcceptPK() )
					{
						///- Tell player if enemy is not accepting PK
						WorldPacket data;
						data.Initialize( 0x21 );
						data << (uint8 ) 1;
						data << (uint8 ) 1;
						SendPacket(&data, true);
						return;
					}

					_player->Engage( enemy );

				} break;
				case 0x03: // To Npc
				{
					uint16 mapid = _player->GetMapId();
					uint64 guid  = objmgr.GetNpcGuidByMapNpcId(mapid, map_npcid);
					Creature* unit = ObjectAccessor::GetNPCIfCanInteractWith(*_player, guid, UNIT_NPC_FLAG_NONE);

					if( !unit )
						return;

					_player->Engage( unit );

				} break;
			}
		} break;
	}

}

void WorldSession::HandlePlayerAttackOpcode( WorldPacket & recv_data )
{
	CHECK_PACKET_SIZE( recv_data, 1+1+1+1+1+2 );

	DEBUG_LOG( "WORLD: Recvd CMSG_PLAYER_ATTACK Message '%s'", _player->GetName());
	uint8  type; // 1 = spell; 2 = use inventory item
	uint8  atk_col;
	uint8  atk_row;
	uint8  tgt_col;
	uint8  tgt_row;
	uint16 spell;

	recv_data >> type;
	recv_data >> atk_col;
	recv_data >> atk_row;
	recv_data >> tgt_col;
	recv_data >> tgt_row;
	recv_data >> spell;

	///- Override incorrect target for self target spell type
	if( SPELL_DEFENSE == spell ||
		SPELL_ESCAPE  == spell )
	{
		tgt_col = atk_col;
		tgt_row = atk_row;
	}

	Player* battleMaster = _player->GetBattleMaster();

	if( !battleMaster )
		return;

	BattleSystem* engine = battleMaster->PlayerBattleClass;

	if( !engine )
		return;

	BattleAction* action = new BattleAction(atk_col, atk_row, spell, tgt_col, tgt_row);

	Unit* attacker = engine->GetAttacker(action);

	ASSERT(attacker);

	if( !attacker->HaveSpell(spell) )
	{
		const SpellInfo* sinfo = objmgr.GetSpellTemplate(spell);
		if( sinfo )
		{
			///- don't have spell or using consumable item
			//   TODO: handle use consumable item
			sLog.outDebug("COMBAT: '%s' don't have spell '%s', cheating ?", attacker->GetName(), sinfo->Name);
			///- Use default basic attack
			action->SetSkill(spell);
		}
		else
		{
			sLog.outDebug("COMBAT: '%s' don't have spell %u, cheating ?", attacker->GetName(), spell);
			action->SetSkill(SPELL_BASIC);
		}

	}

	///- Prevent flood actions
	if( engine->isActionComplete() )
		return;

	engine->AddBattleAction(action);
	engine->IncAction();

	WorldPacket data;

	//if( !GetPlayer()->PlayerBattleClass->isActionComplete() )
	if( !engine->isActionComplete() )
	{
		///- Tell next attacker to move if available
		data.Initialize( 0x35 );
		data << (uint8 ) 0x05;
		data << (uint8 ) atk_col;
		data << (uint8 ) atk_row;
		SendPacket(&data);
		//sLog.outDebug("COMBAT: Waiting next action from player '%s'", _player->GetName());
		return;
	}

	engine->BuildActions();

	///- Do not call UpdateBattleAction here, will be updated via Player::Update

}
