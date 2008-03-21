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

#ifndef __LEGACY_BATTLESYSTEM_H
#define __LEGACY_BATTLESYSTEM_H

#include "Common.h"
#include "Log.h"
#include "Object.h"
#include "Unit.h"

class WorldSession;

#define BATTLE_COL_MAX 4
#define BATTLE_ROW_MAX 5
#define BATTLESLOT_ROW_MAX BATTLE_ROW_MAX

#define DEFENDER_COL_MAX 2

#define ATTACKER_COL_MIN DEFENDER_COL_MAX

/*
 * DEFENDER               ATTACKER
 * 0,0  1,0               2,0  3,0
 * 0,1  1,1               2,1  3,1
 * 0,2  1,2               2,2  3,2
 * 0,3  1,3               2,3  3,3
 * 0,4  1,4               2,4  3,4
 */

class BattleAction
{
	public:
		BattleAction() : _atkCol(0), _atkRow(0), _skill(0), _tgtCol(0), _tgtRow(0), _agility(0) { }
		BattleAction(uint8 atkCol, uint8 atkRow, uint16 skill, uint8 tgtCol, uint8 tgtRow) : _atkCol(atkCol), _atkRow(atkRow), _skill(skill), _tgtCol(tgtCol), _tgtRow(tgtRow), _agility(0) { }
		BattleAction(const BattleAction &act) : _atkCol(act._atkCol), _atkRow(act._atkRow), _skill(act._skill), _tgtCol(act._tgtCol), _tgtRow(act._tgtRow), _agility(act._agility) { }

		void SetAttackerCol(uint8 col) { _atkCol = col; }
		void SetAttackerRow(uint8 row) { _atkRow = row; }
		void SetSkill(uint16 skill) { _skill = skill; }
		void SetTargetCol(uint8 col) { _tgtCol = col; }
		void SetTargetRow(uint8 row) { _tgtRow = row; }
		void SetAgility(uint16 agi) { _agility = agi; }
		uint8  GetAttackerCol() const { return _atkCol; }
		uint8  GetAttackerRow() const { return _atkRow; }
		uint8  GetTargetCol() const { return _tgtCol; }
		uint8  GetTargetRow() const { return _tgtRow; }
		uint16 GetSkill() const { return _skill; }
		uint16 GetAgility() const { return _agility; }

		BattleAction& operator=(const BattleAction &obj)
		{
			_atkCol  = obj._atkCol;
			_atkRow  = obj._atkRow;
			_skill   = obj._skill;
			_tgtCol  = obj._tgtCol;
			_tgtRow  = obj._tgtRow;
			_agility = obj._agility;
		}

	protected:
		uint8  _atkCol;
		uint8  _atkRow;
		uint16 _skill;
		uint8  _tgtCol;
		uint8  _tgtRow;
		uint16 _agility;
};

class LEGACY_DLL_SPEC BattleSystem
{
	private:
		WorldSession* pSession;

	public:
		BattleSystem( WorldSession *Session );
		~BattleSystem();


		void Engage(Creature* enemy);
		void Engage(Player* enemy);
		void BattleStart();
		bool BattleConcluded();
		void BattleStop();
		void WaitForAnimation(uint16 skill);

		void BattleScreenTrigger();

		void InitAttackerPosition();
		void InitDefenderPosition();

		void SetPosition(Unit *unit, uint8 col, uint8 row);

		void SendTurnComplete();
		void SendAttackerPosition();
		void SendPetPosition();
		void SendDefenderPosition();
		void SendAttack(BattleAction* action, int32 damage);
		void SendComboAttack(BattleAction* action, int32 damage);

		bool isActionTimedOut();

		void AddBattleAction(BattleAction* action);
		void UpdateBattleAction();
		void IncAction()
		{
			m_PlayerActionCount++;
			sLog.outString(" >> PlayerActionCount %u", m_PlayerActionCount);
		}
		bool IsActionComplete()
		{
			sLog.outString(" >> IsActionComplete need %u count %u", m_PlayerActionNeed, m_PlayerActionCount);
			return m_PlayerActionNeed == m_PlayerActionCount;
		}

		Unit* GetAttacker(const BattleAction *action) const;
		Unit* GetVictim(const BattleAction *action) const;
		BattleAction* RedirectTarget(BattleAction *action);




	protected:
		void ResetAction();

		int32 GetDamage(Unit*, Unit*, uint16);
		void DealDamageTo(Unit* unit, int32 dmg);

		void AIMove();
		void DumpBattlePosition();

	private:
		uint8 m_PlayerActionNeed;
		uint8 m_PlayerActionCount;

		uint16 m_BattleGroundId;

		bool   m_waitForAction;
		uint32 m_actionTime;

		Unit* m_BattleUnit[BATTLE_COL_MAX][BATTLE_ROW_MAX];

		typedef std::list<BattleAction*> UnitActionTurn;
		UnitActionTurn m_unitTurn;

		uint32 DefenderTeam[BATTLE_COL_MAX][BATTLE_ROW_MAX];
		uint32 AttackerTeam[BATTLE_COL_MAX][BATTLE_ROW_MAX];

};


#endif
