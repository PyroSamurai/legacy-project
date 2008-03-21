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
#include "BattleSystem.h"
#include "Log.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "Player.h"
#include "Pet.h"
#include "Creature.h"
#include "ObjectMgr.h"

#define MAX_ACTION_TIME 220 /* 20 sec countdown, 2 sec tolerance */

bool compare_agility(BattleAction *first, BattleAction* second)
{
	//sLog.outString(" >> Comparing %u with %u", first->GetAgility(), second->GetAgility());
	if(first->GetAgility() > second->GetAgility())
		return true;
	else
		return false;
}

void BattleSystem::DumpBattlePosition()
{
	printf("\tDEFENDER\t\t\t\t\tATTACKER\n");
	for(int row = 0; row < BATTLE_ROW_MAX; row++)
	{
		for(int col = 0; col < BATTLE_COL_MAX; col++)
		{
			if( col == 2) printf("| ");
			Creature* unit = (Creature*) m_BattleUnit[col][row];
			if( !unit )
				printf("%9s %4s %3s ", "", "", "");
			else
				printf("[%0.7s %4u/%3u] ",
						unit->GetName(),
						unit->GetUInt16Value(UNIT_FIELD_HP),
						unit->GetUInt16Value(UNIT_FIELD_SP));
		}
		printf("\n");
	}
}
										        
BattleSystem::BattleSystem(WorldSession *Session)
{
	pSession = Session;
	m_BattleGroundId = 0x017D;  // battle background stage id

	m_PlayerActionNeed = 0;
	m_PlayerActionCount = 0;
	m_actionTime = 0;
	m_waitForAction = false;
}

BattleSystem::~BattleSystem()
{
	UnitActionTurn::iterator itr;
	for(itr = m_unitTurn.begin(); itr != m_unitTurn.end(); ++itr)
	{
		BattleAction* act = *itr;
		if( act )
			delete act;
	}
	for(uint8 row = 0; row < BATTLE_ROW_MAX; row++)
	{
		for(uint8 col = 0; col < BATTLE_COL_MAX; col++)
		{
			Unit* unit = m_BattleUnit[col][row];
			if( !unit )
				continue;
			if( unit->isType(TYPE_PLAYER) || unit->isType(TYPE_PET ) )
				continue;

			delete unit;
		}
	}
	m_unitTurn.clear();
}

void BattleSystem::Engage( Creature* enemy )
{
	m_PlayerActionNeed = 0;
	m_PlayerActionCount = 0;
	m_actionTime = 0;
	m_waitForAction = false;
	///- Reset all Unit
	for(uint8 row = 0; row < BATTLE_ROW_MAX; row++)
	{
		for (uint8 col = 0; col < BATTLE_COL_MAX; col++)
		{
			m_BattleUnit[col][row] = NULL;
		}
	}

	const CreatureData* edata = objmgr.GetCreatureData( enemy->GetGUIDLow() );

	///- Define all creature team
	DefenderTeam[0][0] = edata->team00;
	DefenderTeam[0][1] = edata->team01;
	DefenderTeam[0][2] = enemy->GetGUIDLow();  // Defender Leader Position
	DefenderTeam[0][3] = edata->team03;
	DefenderTeam[0][4] = edata->team04;

	DefenderTeam[1][0] = edata->team10;
	DefenderTeam[1][1] = edata->team11;
	DefenderTeam[1][2] = edata->team12;
	DefenderTeam[1][3] = edata->team13;
	DefenderTeam[1][4] = edata->team14;




	///- Define only player row
	// pet will be define later. See InitAttackerPosition
	Player* aLeader = pSession->GetPlayer();
	AttackerTeam[3][0] = aLeader->GetTeamGuid(3);
	AttackerTeam[3][1] = aLeader->GetTeamGuid(1);
	AttackerTeam[3][2] = aLeader->GetGUIDLow();
	AttackerTeam[3][3] = aLeader->GetTeamGuid(2);
	AttackerTeam[3][4] = aLeader->GetTeamGuid(4);

	InitDefenderPosition();
	InitAttackerPosition();
	BattleStart();
}

void BattleSystem::Engage( Player* enemy )
{
}

void BattleSystem::BattleStart()
{
	WorldPacket data;

	data.Initialize( 0x16, 1 );
	data << (uint8 ) 0x02;
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x00;
	data << (uint16) 0x0303;
	data << (uint16) 0x062B;
	pSession->SendPacket(&data); // npc movement info

	data.Initialize( 0x16, 1 );
	data << (uint8 ) 0x02;
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x00;
	data << (uint16) 0x02D8;
	data << (uint16) 0x0673;
	pSession->SendPacket(&data); // npc movement info

	sLog.outString("");
	sLog.outString( "BattleSystem::BattleStart" );
	sLog.outString("");

	data.Initialize( 0x14 );
	data << (uint8 ) 0x0C;
	pSession->SendPacket(&data);

	data.Initialize( 0x06 );
	data << (uint8 ) 0x02;
	pSession->SendPacket(&data);

	pSession->SetLogging(false);

	///- Battle Initiator, Send order is CRITICAL !!!!
	// Battle of Suku Mu
	pSession->GetPlayer()->UpdatePlayer();
	BattleScreenTrigger();

	pSession->GetPlayer()->UpdatePet(0);
	SendPetPosition();

	SendDefenderPosition();

	SendTurnComplete();
}

///- Reset Action
void BattleSystem::ResetAction()
{
	m_actionTime = 0;
	m_PlayerActionCount = 0;
	m_PlayerActionNeed = 0;
	for(uint8 row = 0; row < BATTLE_ROW_MAX; row++)
		for(uint8 col = 0; col < BATTLE_COL_MAX; col++)
		{
			Unit* unit = m_BattleUnit[col][row];
			if( !unit )
				continue;

			if( unit->isType(TYPE_PLAYER) || unit->isType(TYPE_PET))
				m_PlayerActionNeed++;
		}

}

///- Set Turn Ready
void BattleSystem::SendTurnComplete()
{
	ResetAction();
	m_waitForAction = true;

	WorldPacket data;
	data.Initialize( 0x14 );
	data << (uint8 ) 0x09;
	pSession->SendPacket(&data);

	data.Initialize( 0x34 );
	data << (uint8 ) 0x01;
	pSession->SendPacket(&data);

	DumpBattlePosition();
}

///- Initialize Battle Screen Mode
void BattleSystem::BattleScreenTrigger()
{
	WorldPacket data;

	Player *p = pSession->GetPlayer();
	data.Initialize( 0x0B );
	data << (uint8 ) 0xFA;    // initiate battle on screen, i think
	data << m_BattleGroundId; // battle background stage id
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x02;
	data << pSession->GetAccountId();
	data << (uint16) 0x0000;
	data << (uint16) 0x0000;
	data << (uint16) 0x0000;
	data << (uint8 ) 3;       // player row position
	data << (uint8 ) 2;       // player column position
	data << p->GetUInt16Value(UNIT_FIELD_HP_MAX); // player hp max
	data << p->GetUInt16Value(UNIT_FIELD_SP_MAX); // player sp max
	data << p->GetUInt16Value(UNIT_FIELD_HP);     // player current hp
	data << p->GetUInt16Value(UNIT_FIELD_SP);     // player current sp
	data << p->GetUInt8Value(UNIT_FIELD_LEVEL);   // player level
	data << p->GetUInt8Value(UNIT_FIELD_ELEMENT); // player element
	pSession->SendPacket(&data);

	///- Start the battle IMPORTANT!!
	data.Initialize( 0x0B );
	data << (uint8 ) 0x0A;
	data << (uint8 ) 0x01;
	pSession->SendPacket(&data);
}

///- Send Only TYPE_PET Unit position
void BattleSystem::SendPetPosition()
{
	WorldPacket data;
/*
	data.Initialize( 0x0B );
	data << (uint8 ) 0x05;
	data << (uint8 ) 0x05;
	data << (uint8 ) 0x04;
	data << (uint16) 12027; //41027; //47023;   // Pet Npc Id
	data << (uint16) 0x0000;
	data << (uint16) 0x0004;
	data << pSession->GetAccountId(); // pet owner id (0 for AI)
	data << (uint8 ) 2;       // pet row position
	data << (uint8 ) 2;       // pet column position
	data << (uint16) 4000;    // pet hp max
	data << (uint16) 500;     // pet sp max
	data << (uint16) 3900;    // pet current hp
	data << (uint16) 400;     // pet current sp
	data << (uint8 ) 120;     // pet level
	data << (uint8 ) 4;       // pet element
	pSession->SendPacket(&data);
	return;
*/

	for(uint8 row = 0; row < BATTLE_ROW_MAX; row++)
	{
		Pet* pet = (Pet*) m_BattleUnit[2][row];
		if( !pet ) continue;

		data.Initialize( 0x0B );
		data << (uint8 ) 0x05;
		data << (uint8 ) 0x05;
		data << (uint8 ) 0x04;
		data << (uint16) pet->GetModelId(); // Pet ModelId
		data << (uint16) 0x0000;
		data << (uint16) 0x0004;
		data << pSession->GetAccountId(); // pet owner id (0 for AI)
		data << (uint8 ) 2;              // pet col position
		data << row;              // pet row position
		data << pet->GetUInt16Value(UNIT_FIELD_HP_MAX); // pet hp max
		data << pet->GetUInt16Value(UNIT_FIELD_SP_MAX); // pet sp max
		data << pet->GetUInt16Value(UNIT_FIELD_HP);     // pet current hp
		data << pet->GetUInt16Value(UNIT_FIELD_SP);     // pet current sp
		data << pet->GetUInt8Value(UNIT_FIELD_LEVEL);   // pet level
		data << pet->GetUInt8Value(UNIT_FIELD_ELEMENT); // pet element

		pSession->SendPacket(&data);
	}
}

///- Init Attacker position
void BattleSystem::InitAttackerPosition()
{
	uint8 col = 3; // set col for player row only
	for(uint8 row = 0; row < BATTLE_ROW_MAX; row++)
	{
		Player* unit = objmgr.GetPlayer(AttackerTeam[col][row]);
		if( !unit )
			continue;

		SetPosition(unit, col, row);

		Pet* pet = unit->GetPet(1);
		if( !pet )
			continue;

		SetPosition(pet, col - 1, row);
	}

	/*
	SetPosition(pSession->GetPlayer(), 3, 2);

	Pet *pet = pSession->GetPlayer()->GetPet(0);

	if( !pet )
		return;

	SetPosition(pet, 2, 2);
	*/
}

///- Init Enemy position
void BattleSystem::InitDefenderPosition()
{
	for(uint8 row = 0; row < BATTLE_ROW_MAX; row++)
	{
		for(uint8 col = 0; col < DEFENDER_COL_MAX; col++)
		{
			uint32 guid = DefenderTeam[col][row];
			if( !guid )
				continue;

			Creature* unit = new Creature( NULL );
			if( !unit->LoadFromDB( guid ) )
			{
				delete unit;
				continue;
			}
			SetPosition( unit, col, row );
		}
	}
	/*
	for(int i = 0; i < 3; i++)
	{
		Creature *creature = new Creature( NULL );
		if( !creature->LoadFromDB( 1286101 ) )
		{
			delete creature;
			return;
		}
		SetPosition( creature, 0, i+1 );
	}
	Creature *creature = new Creature( NULL );
	if( !creature->LoadFromDB( 1202700 ) )
	{
		delete creature;
		return;
	}
	SetPosition( creature, 1, 2 );
	*/
}

void BattleSystem::SetPosition(Unit *unit, uint8 col, uint8 row)
{
	m_BattleUnit[col][row] = unit;
}

void BattleSystem::SendAttackerPosition()
{
	///- Only send for TYPE_PLAYER
}

void BattleSystem::SendDefenderPosition()
{
	WorldPacket data;
/*
	data.Initialize( 0x0B );
	data << (uint8 ) 0x05;
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x07;
	data << guidlow;              // npc id
	data << count;              // unknown, still guessing
	data << (uint8 ) 0x00;
	data << (uint16) 0x0000;
	data << (uint16) 0x0000;
	data << col;
	data << row;
	data << (uint16) 1200;      // max hp
	data << (uint16) 100;       // max sp
	data << (uint16) 1200;      // current hp
	data << (uint16) 100;       // current sp
	data << (uint8 ) 13;        // level
	data << (uint8 ) 3;         // element
	pSession->SendPacket(&data);
*/
	uint8 count = 0;
	for(uint8 row = 0; row < BATTLE_ROW_MAX; row++)
	{
		for(uint8 col = 0; col < DEFENDER_COL_MAX; col++)
		{
			Creature *unit = (Creature*) m_BattleUnit[col][row];
			if( !unit )
				continue;

			data.Initialize( 0x0B );
			data << (uint8 ) 0x05;
			data << (uint8 ) 0x01;
			data << (uint8 ) 0x07;
			data << (uint16) unit->GetModelId();// npc ModelId
			data << (uint16) 0x0000;
			data << count++;              // unknown, still guessing
			data << (uint8 ) 0x00;
			data << (uint16) 0x0000;
			data << (uint16) 0x0000;
			data << col;
			data << row;
			data << (uint16) unit->GetUInt16Value(UNIT_FIELD_HP_MAX); // max hp
			data << (uint16) unit->GetUInt16Value(UNIT_FIELD_SP_MAX); // max sp
			data << (uint16) unit->GetUInt16Value(UNIT_FIELD_HP); // current hp
			data << (uint16) unit->GetUInt16Value(UNIT_FIELD_SP); // current sp
			data << (uint8 ) unit->GetUInt8Value(UNIT_FIELD_LEVEL); // level
			data << (uint8 ) unit->GetUInt8Value(UNIT_FIELD_ELEMENT);// element
			pSession->SendPacket(&data);
		}
	}
}

void BattleSystem::AIMove()
{
	for( uint8 row = 0; row < BATTLE_ROW_MAX; row++)
	{
		for( uint8 col = 0; col < DEFENDER_COL_MAX; col++)
		{
			Unit *unit = m_BattleUnit[col][row];
			if( !unit ) continue;
			BattleAction* action = new BattleAction(col, row, 10000, 2, 2);
			AddBattleAction(action);
		}
	}
}

Unit* BattleSystem::GetAttacker(const BattleAction *action) const
{
	return m_BattleUnit[action->GetAttackerCol()][action->GetAttackerRow()];
}

Unit* BattleSystem::GetVictim(const BattleAction *action) const
{
	return m_BattleUnit[action->GetTargetCol()][action->GetTargetRow()];
}

BattleAction* RedirectActionTarget(BattleAction *action)
{
	uint8 col = action->GetAttackerCol();
	///- Detect direction
	if( col < DEFENDER_COL_MAX )
	{
	}
	else if( col > DEFENDER_COL_MAX )
	{
	}
	return action;
}

void BattleSystem::AddBattleAction(BattleAction* action)
{
	Unit* pAttacker = GetAttacker(action);

	if( !pAttacker )
		return;

	Unit* pTarget = GetVictim(action);
	
	if( !pTarget || pTarget->isDead() )
		action = RedirectActionTarget(action);

	uint16 agility = pAttacker->GetUInt16Value(UNIT_FIELD_AGI);

	///- TODO:
	// Check Golem status buf
	// Check for Bros Agility modifier here for PLAYER_TYPE


	action->SetAgility(agility);

	sLog.outString(" >> Push Back Unit Agility: %u", action->GetAgility());
	//BattleAction *act = new BattleAction(action);
	m_unitTurn.push_back(action);
}

void BattleSystem::UpdateBattleAction()
{
	m_waitForAction = false;
	m_actionTime = 0;

	BattleAction *action;
	WorldPacket data;
	UnitActionTurn::iterator itr;

	AIMove();

	m_unitTurn.sort(compare_agility);

	for(itr = m_unitTurn.begin(); itr != m_unitTurn.end(); ++itr)
	{
		BattleAction* act = *itr;
		sLog.outString(" == Action Turn Agility: %u", act->GetAgility());
	}

	for(itr = m_unitTurn.begin(); itr != m_unitTurn.end(); ++itr)
	{
		if( !pSession )
			return;

		action = *itr;

		Unit *pAttacker = GetAttacker(action);
		Unit *pVictim   = GetVictim(action);
		uint16 skill    = action->GetSkill();

		///- TODO: Handle Confused state here later on

		if(pAttacker->isDead() || pAttacker->isDisabled())
			continue;

		if(pVictim->isDead())
		{

			if(BattleConcluded())
			{
				BattleStop();
				return;
			}

			RedirectActionTarget(action);
		}

		///- TODO: Calculate Combo chance !!
		
		int32 damage = GetDamage(pAttacker, pVictim, skill);

		DealDamageTo(pVictim, damage);
	
		SendAttack(action, damage);

		delete action;

	}

	m_unitTurn.clear();

	if(BattleConcluded())
	{
		BattleStop();
		return;
	}

	SendTurnComplete();
}

///- Calculate point inflicted to victim
//   negative value for damage
//   positive value for heal
int32 BattleSystem::GetDamage(Unit* attacker, Unit* target, uint16 skill)
{
	///- TODO Check Aura spell buf

	///- TODO Calculate Miss chance

	return -230;
}

void BattleSystem::DealDamageTo(Unit* pVictim, int32 dmg)
{
	uint32 currentHP = pVictim->GetUInt16Value(UNIT_FIELD_HP);

	///- Damage Type
	if(dmg < 0)
	{
		if( currentHP < abs(dmg) )
		{
			pVictim->SetUInt16Value(UNIT_FIELD_HP, 0);
			pVictim->setDeathState(DEAD);
			sLog.outString(" ** %s killed", pVictim->GetName());
			return;
		}
		
		currentHP += dmg;
		pVictim->SetUInt16Value(UNIT_FIELD_HP, currentHP);
		return;
	}
	///- Heal Type
	else
	{
	}
}

void BattleSystem::SendAttack(BattleAction* action, int32 damage)
{
	Unit* pAttacker = m_BattleUnit[action->GetAttackerCol()][action->GetAttackerRow()];
	Unit* pVictim = m_BattleUnit[action->GetTargetCol()][action->GetTargetRow()];
	sLog.outString(" ++ Unit action %s [%u,%u] use skill %u to %s [%u,%u] deals %i point", pAttacker->GetName(), action->GetAttackerCol(), action->GetAttackerRow(), action->GetSkill(), pVictim->GetName(), action->GetTargetCol(), action->GetTargetRow(), damage);

	WorldPacket data;
	data.Initialize( 0x32 );
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x0F;
	data << (uint8 ) 0x00;
	data << action->GetAttackerCol(); // attacker col
	data << action->GetAttackerRow(); // attacker row
	data << action->GetSkill(); // 0x2710 = basic punch
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x01;
	data << action->GetTargetCol(); // target col
	data << action->GetTargetRow(); // target row
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x00;
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x19;

	uint16 point_inflicted = abs(damage);
	uint8  point_modifier  = damage < 0 ? 1 : 0;
	data << point_inflicted; //(uint16) damage;    // actual hit value
	data << point_modifier; //(uint8 ) 0x01;      // flag 0 = heal, 1 = damage
	pSession->SendPacket(&data);
	WaitForAnimation(action->GetSkill());
}

void BattleSystem::SendComboAttack(BattleAction* action, int32 damage)
{
	sLog.outString(" ++ Unit action [%u,%u] use skill %u to [%u,%u] deals %i point", action->GetAttackerCol(), action->GetAttackerRow(), action->GetSkill(), action->GetTargetCol(), action->GetTargetRow(), damage);

	WorldPacket data;
	data.Initialize( 0x32 );
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x0F;
	data << (uint8 ) 0x00;
	data << (uint8 ) action->GetAttackerCol();
	data << (uint8 ) action->GetAttackerRow();
	data << (uint16) action->GetSkill();
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x01;
	data << (uint8 ) action->GetTargetCol();
	data << (uint8 ) action->GetTargetRow();
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x00;
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x19;
	uint16 point_inflicted = abs(damage);
	uint8  point_modifier  = damage < 0 ? 1 : 0;
	data << point_inflicted; //(uint16) damage;    // actual hit value
	data << point_modifier; //(uint8 ) 0x01;      // flag 0 = heal, 1 = damage

	///- Add this for combo
/*
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x0F;
	data << (uint8 ) 0x00;
	data << (uint8 ) 0x02;
	data << (uint8 ) 0x02;
	data << (uint16) 0x2710;
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x01;
	data << (uint8 ) action->GetTargetCol(); //(uint8 ) 0x00;
	data << (uint8 ) action->GetTargetRow(); //(uint8 ) 0x01;
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x00;
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x19;
	data << (uint8 ) 0x68;
	data << (uint8 ) 0x01;
	data << (uint8 ) 0x01;
*/
	pSession->SendPacket(&data);
	WaitForAnimation(action->GetSkill());
}

void BattleSystem::WaitForAnimation(uint16 skill)
{
	uint32 msec;
	if( skill == 1000 )
		msec = 1200;
	else
		msec = 2500;
	ZThread::Thread::sleep(msec);
}

bool BattleSystem::isActionTimedOut()
{
	if( !m_waitForAction )
		return false;

	m_actionTime++;
//	sLog.outString("Action Time %u", m_actionTime);
	if( m_actionTime < MAX_ACTION_TIME )
		return false;

	return true;
}
/*
void BattleSystem::Overkilled(uint8 col, uint8 row)
{
	WorldPacket data;
	data.Initialize( 0x35 );
	data << (uint8 ) 0x03; // overkilled
	data << (uint8 ) 0x00; // col
	data << (uint8 ) 0x03; // row
	pSession->SendPacket(&data);
}
*/

bool BattleSystem::BattleConcluded()
{
	bool lastManStanding = false;
	for(uint8 row = 0; row < BATTLE_ROW_MAX; row++)
	{
		for(uint8 col = 0; col < DEFENDER_COL_MAX; col++)
		{
			Unit* unit = m_BattleUnit[col][row];
			if( !unit )
				continue;

			if( unit->isAlive() )
			{
				lastManStanding = true;
				break;
			}
		}
		if( lastManStanding )
			break;
	}

	if( !lastManStanding )
		return true;

	for(uint8 row = 0; row < BATTLE_ROW_MAX; row++)
	{
		for(uint8 col = ATTACKER_COL_MIN; col < BATTLE_COL_MAX; col++)
		{
			Unit* unit = m_BattleUnit[col][row];
			if( !unit )
				continue;

			if( unit->isAlive() )
				return false;
		}
	}

	return true;
}

void BattleSystem::BattleStop()
{
	///- wait for 14 06 first

	WorldPacket data;

	data.Initialize( 0x0B );
	data << (uint8 ) 0x01 << (uint8 ) 0x02 << (uint8 ) 0x02;
	pSession->SendPacket(&data);

	data.Initialize( 0x0B );
	data << (uint8 ) 0x01 << (uint8 ) 0x03 << (uint8 ) 0x02 << (uint8 ) 0x00;
	pSession->SendPacket(&data);


	//data.Initialize( 0x14 );
	//data << (uint8 ) 0x08;
	pSession->SendPacket(&data);
}
