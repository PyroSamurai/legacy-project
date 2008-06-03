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
#include "Spell.h"
#include "ObjectMgr.h"

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

#define ANI_IMPACT_MISS  0
#define ANI_IMPACT_HIT   1

#define ANI_TARGET_HIT   0
#define ANI_TARGET_DEF   1
#define ANI_TARGET_DODGE 2
#define ANI_TARGET_NONE  4

class BattleAction
{
	public:
		BattleAction() : _atkCol(0), _atkRow(0), _skill(0), _tgtCol(0), _tgtRow(0), _agility(0) { }
		BattleAction(uint8 atkCol, uint8 atkRow, uint16 skill, uint8 tgtCol, uint8 tgtRow) : _atkCol(atkCol), _atkRow(atkRow), _skill(skill), _tgtCol(tgtCol), _tgtRow(tgtRow), _agility(0) { }
		BattleAction(const BattleAction &act) : _atkCol(act._atkCol), _atkRow(act._atkRow), _skill(act._skill), _tgtCol(act._tgtCol), _tgtRow(act._tgtRow), _agility(act._agility) { }

		void SetAttacker(uint8 col, uint8 row) { _atkCol = col; _atkRow = row; }
		void SetAttackerCol(uint8 col) { _atkCol = col; }
		void SetAttackerRow(uint8 row) { _atkRow = row; }
		void SetSkill(uint16 skill) { _skill = skill; }
		void SetTarget(uint8 col, uint8 row) { _tgtCol = col; _tgtRow = row; }
		void SetTargetCol(uint8 col) { _tgtCol = col; }
		void SetTargetRow(uint8 row) { _tgtRow = row; }
		void SetAgility(int32 agi) { _agility = agi; }
		uint8  GetAttackerCol() const { return _atkCol; }
		uint8  GetAttackerRow() const { return _atkRow; }
		uint8  GetTargetCol() const { return _tgtCol; }
		uint8  GetTargetRow() const { return _tgtRow; }
		uint16 GetSkill() const { return _skill; }
		int32  GetAgility() const { return _agility; }

		BattleAction& operator=(const BattleAction &obj)
		{
			_atkCol  = obj._atkCol;
			_atkRow  = obj._atkRow;
			_skill   = obj._skill;
			_tgtCol  = obj._tgtCol;
			_tgtRow  = obj._tgtRow;
			_agility = obj._agility;
		}

		const SpellInfo* GetProto()
		{
			return objmgr.GetSpellTemplate(_skill);
		}

		bool isLinkable();

	protected:
		uint8  _atkCol;
		uint8  _atkRow;
		uint16 _skill;
		uint8  _tgtCol;
		uint8  _tgtRow;
		int32  _agility;
};

typedef std::list<BattleAction*> UnitActionTurn;

class ItemDropped
{
	public:
		ItemDropped() : _rcvCol(0), _rcvRow(0), _item(0), _ctrCol(0), _ctrRow(0) { }
		ItemDropped(uint8 rcvCol, uint8 rcvRow, uint16 item, uint8 ctrCol, uint8 ctrRow) : _rcvCol(rcvCol), _rcvRow(rcvRow), _item(item), _ctrCol(ctrCol), _ctrRow(ctrRow) { }

		uint8  GetReceiverCol() const { return _rcvCol; }
		uint8  GetReceiverRow() const { return _rcvRow; }
		uint16 GetItem() const { return _item; }
		uint8  GetContributorCol() const { return _ctrCol; }
		uint8  GetContributorRow() const { return _ctrRow; }

	protected:
		uint8  _rcvCol;
		uint8  _rcvRow;
		uint16 _item;
		uint8  _ctrCol;
		uint8  _ctrRow;
};

typedef std::list<ItemDropped*> ItemDroppedTurn;

class LEGACY_DLL_SPEC BattleSystem
{
	public:
		BattleSystem(Player* master);
		~BattleSystem();

		bool FindNewMaster();

		void Engage(Player* attacker, Creature* enemy);
		void Engage(Player* attacker, Player* enemy);
		void BattleStart();
		bool BattleConcluded();
		void BattleStop();
		void WaitForAnimation(uint16 skill);

		void BattleScreenTrigger();
		void BattleScreenTrigger2();

		void InitAttackerPosition();
		void InitDefenderPosition();
		void InitDefenderPosition2();

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
		void BuildActions();
		bool SendAction();
		void IncAction()
		{
			m_PlayerActionCount++;
			sLog.outString("COMBAT: >> PlayerActionCount %u", m_PlayerActionCount);
		}
		bool isActionComplete()
		{
			sLog.outString("COMBAT: >> ROUND PLAYER ACTION need %u, count %u", m_PlayerActionNeed, m_PlayerActionCount);
			return m_PlayerActionNeed == m_PlayerActionCount;
		}

		Unit* GetAttacker(const BattleAction *action);
		Unit* GetVictim(const BattleAction *action);

		bool NeedRedirectTarget(const BattleAction *action);
		BattleAction* RedirectTarget(BattleAction *action);


		bool CanJoin() const;
		void JoinBattle(Player* player);
		void LeaveBattle(Player* player);
		void Escaped(BattleAction *action);

		bool ActivatePetFor(Player* player, Pet* pet);
		bool DeactivatePetFor(Player* player);
		bool GetPosFor(Unit* unit, uint8 &col, uint8 &row);
		bool GetPetPosFor(Player* player, uint8 &col, uint8 &row);

		bool CanCatchPet(Unit* attacker, Unit* victim);

	protected:
		void ResetAction();

		int32 GetDamage(Unit* attacker, Unit* victim, const SpellInfo* sinfo, bool linked=false);
		float GetDamageMultiplier(uint8 el1, uint8 el2, const SpellInfo* sinfo);
		bool isDealDamageToAndKill(Unit* victim, int32 damage);

		void AIMove();
		void DumpBattlePosition();

		bool CanLink(BattleAction act1, BattleAction act2);
		void BuildUpdateBlockAction(WorldPacket *data, BattleAction* action, bool linked=false);
		void BuildUpdateBlockSpellMod(WorldPacket *data, BattleAction* action, const SpellInfo* sinfo,Unit* attacker, Unit* victim, int32 damage, uint8 modifier, bool linked, bool catched);
		void BuildUpdateBlockPetPosition(WorldPacket *data, Pet* pet, uint8 col, uint8 row);
		UnitActionTurn ParseSpell(BattleAction* action, uint8 hit, bool linked=false);

		void SendMessageToSet(WorldPacket * packet, bool log=false);
		void SendMessageToPlayer(Player* player, WorldPacket * packet, bool log=false);
		void SendMessageToAttacker(WorldPacket *packet, bool log=false);
		void SendMessageToDefender(WorldPacket *packet, bool log=false);

		/*******************************************************************/
		/***                    POSITIONING HELPER                       ***/
		/*******************************************************************/
		bool SameSide(const BattleAction* prev, const BattleAction* next);
		bool isDefPos(const uint8 col); 
		bool isAtkPos(const uint8 col);
		bool isDefPosBackRow(const uint8 col);
		bool isAtkPosBackRow(const uint8 col);

		Unit* GetBattleUnit(uint8 col, uint8 row);
		bool isUnitAvail(uint8 col, uint8 row, bool linked=false);


		/*******************************************************************/
		/***                     SPELL HELPER                            ***/
		/*******************************************************************/
		const char* GetDamageModText(uint32 dmg_mod);


		/*******************************************************************/
		/***               EXPERIENCE & ITEM DROP HELPER                 ***/
		/*******************************************************************/
		void AddKillExpGained(Unit* killer, Unit* victim, bool linked);
		void AddHitExpGained(Unit* hitter, Unit* victim, bool linked);
		void AddKillItemDropped(BattleAction* action);
		void GiveExpGainedFor(Player* player);
		void GiveItemDropped();
		void SendItemDropped(ItemDropped* item);
		void BuildUpdateBlockItemDropped(WorldPacket *data, ItemDropped* item);
	private:
		Player* i_master;

		uint8  m_PlayerActionNeed;
		uint8  m_PlayerActionCount;

		uint16 m_BattleGroundId;

		bool   m_waitForAction;
		uint32 m_actionTime;
		uint32 m_animationTime;

		uint32 m_animationTimeAction;
		uint32 m_animationTimeItemDropped;

		bool   m_creatureKilled;

		Unit*  m_BattleUnit[BATTLE_COL_MAX][BATTLE_ROW_MAX];

		UnitActionTurn m_unitTurn;

		ItemDroppedTurn m_itemDropped;

		UnitActionTurn m_Actions[20];

		uint32 DefenderTeam[BATTLE_COL_MAX][BATTLE_ROW_MAX];
		uint32 AttackerTeam[BATTLE_COL_MAX][BATTLE_ROW_MAX];

		double m_AtkRateLvl;
		double m_DefRateLvl;

		typedef std::set<Player*> PlayerListMap;
		PlayerListMap m_PlayerList;

		///- for experience calculation helper
		bool   m_teamBattle;
		uint8  m_teamLowestLevel;
		uint8  m_teamHighestLevel;
		uint8  m_teamLowerLevelCount;

};


#endif
