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

void WorldSession::HandlePlayerAttackOpcode( WorldPacket & recv_data )
{
	DEBUG_LOG( "WORLD: Recvd CMSG_PLAYER_ATTACK Message '%s'", _player->GetName());

	///- TODO: Check Packet Size

	uint8 unk1;

	uint8 atk_col;
	uint8 atk_row;
	uint8 tgt_col;
	uint8 tgt_row;

	uint16 unk2;
	uint16 skill;

	recv_data >> unk1;
	recv_data >> atk_col;
	recv_data >> atk_row;
	recv_data >> tgt_col;
	recv_data >> tgt_row;
	recv_data >> skill;

	Player* battleMaster = _player->GetBattleMaster();

	if( !battleMaster )
		return;

	BattleSystem* engine = battleMaster->PlayerBattleClass;

	if( !engine )
		return;

	BattleAction* action = new BattleAction(atk_col, atk_row, skill, tgt_col, tgt_row);

	Unit* attacker = engine->GetAttacker(action);

	ASSERT(attacker);

	if( !attacker->HaveSpell(skill) )
	{
		sLog.outDebug("COMBAT: '%s' don't have spell '%s', cheating ?", attacker->GetName(), objmgr.GetSpellTemplate(skill)->Name);
		///- Use default basic attack
		action->SetSkill(SPELL_BASIC);
	}

	engine->AddBattleAction(action);
	engine->IncAction();

	WorldPacket data;

	//if( !GetPlayer()->PlayerBattleClass->isActionComplete() )
	if( !engine->isActionComplete() )
	{
		///- Tell next attacker to move if available
		data.Initialize( 0x35 );
		data << (uint8 ) 0x05;
		data << atk_col;
		data << atk_row;
		SendPacket(&data);
		//sLog.outDebug("COMBAT: Waiting next action from player '%s'", _player->GetName());
		return;
	}

	engine->BuildActions();

	///- Do not call UpdateBattleAction here, will be updated via Player::Update

}
