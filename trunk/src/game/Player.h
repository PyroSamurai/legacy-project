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

#ifndef __LEGACY_PLAYER_H
#define __LEGACY_PLAYER_H

#include "Common.h"
#include "Unit.h"

#include "Database/DatabaseEnv.h"

#include "WorldSession.h"
#include "Util.h"

#include <string>
#include <vector>

// used at player loading query list preparing, and later result selection
enum PlayerLoginQueryIndex
{
	PLAYER_LOGIN_QUERY_LOADFROM                        = 0,
//	PLAYER_LOGIN_QUERY_LOADGROUP                       = 1,
//	PLAYER_LOGIN_QUERY_LOADINVENTORY                   = 2,
};

enum CharacterFields
{
	FD_GUID = 0,
	FD_ACCOUNTID, 
	FD_PSSWD,
	FD_GENDER,
	FD_ELEMENT,
	FD_FACE,
	FD_HAIR,
	FD_CHARNAME,
	FD_REBORN,
	FD_LEVEL,
	FD_MAPID,
	FD_POSX,
	FD_POSY,
	FD_HP,
	FD_SP,
	FD_HP_MAX,
	FD_SP_MAX,
	FD_XP_GAIN,

	FD_ST_INT,
	FD_ST_ATK,
	FD_ST_DEF,
	FD_ST_HPX,
	FD_ST_SPX,
	FD_ST_AGI,

	FD_ST_INT_BONUS,
	FD_ST_ATK_BONUS,
	FD_ST_DEF_BONUS,
	FD_ST_HPX_BONUS,
	FD_ST_SPX_BONUS,
	FD_ST_AGI_BONUS,

	FD_SKILL_GAIN,
	FD_SKILL_USED,

	FD_STAT_GAIN,
	FD_STAT_USED,

	FD_EQ_HEAD,
	FD_EQ_BODY,
	FD_EQ_WRIST,
	FD_EQ_WEAPON,
	FD_EQ_SHOE,
	FD_EQ_ACCSR,

	FD_HAIR_COLOR,
	FD_SKIN_COLOR,

	FD_ONLINE_STATUS
};

#define MAX_PLAYER_LOGIN_QUERY                           1

class WorldSession;

class LEGACY_DLL_SPEC Player : public Unit
{
	friend class WorldSession;

	public:
		explicit Player (WorldSession *session);
		~Player();

		void AddToWorld();
		void RemoveFromWorld();

		typedef std::set<uint64> ClientGUIDs;
		ClientGUIDs m_clientGUIDs;

		bool HaveAtClient(WorldObject const* u) { return u==this || m_clientGUIDs.find(u->GetGUID())!=m_clientGUIDs.end(); }

		uint32 GetAccountId() { return m_session->GetAccountId(); };

		bool Create ( uint32 guidlow, WorldPacket &data );
	
		void Update( uint32 time );

		WorldSession* GetSession() const { return m_session; }

		void BuildUpdateBlockStatusPacket(WorldPacket *data);

		void BuildUpdateBlockVisibilityPacket(WorldPacket *data);
		void BuildUpdateBlockVisibilityForOthersPacket(WorldPacket *data);

		void BuildUpdateBlockTeleportPacket(WorldPacket *data);

		void TeleportTo(uint16 mapid, uint16 pos_x, uint16 pos_y);
		void SendInitialPacketsBeforeAddToMap();
		void SendInitialPacketsAfterAddToMap();
		void SendMapChanged();

		void AllowPlayerToMove();
		void EndOfRequest();

		/*********************************************************/
		/***                  LOAD SYSTEM                      ***/
		/*********************************************************/
		bool LoadFromDB(uint32 accountId, SqlQueryHolder *holder);


		/*********************************************************/
		/***                  SAVE SYSTEM                      ***/
		/*********************************************************/
		void SaveToDB();

		void SendMessageToSet(WorldPacket *data, bool self); // overwrite Object::SendMessageToSet

		void SendDelayResponse(const uint32);

		/*********************************************************/
		/***                  GROUP SYSTEM                     ***/
		/*********************************************************/


		void SetDontMove(bool dontMove) { m_dontMove = dontMove; }
		bool GetDontMove() const { return m_dontMove; }

		void SetTeleportTo(uint16 target) { m_teleportMapId = target; }
		uint16 GetTeleportTo() { return m_teleportMapId; }

		uint32 GetTeam() const { return m_team; }


		/*********************************************************/
		/***                VARIOUS SYSTEM                     ***/
		/*********************************************************/
		void UpdateVisibilityOf(WorldObject* target);

		template<class T>
			void UpdateVisibilityOf(T* target, UpdateData& data, UpdateDataMapType& data_updates, std::set<WorldObject*>& visibleNow);


		GridReference<Player> &GetGridRef() { return m_gridRef; }
	protected:


		WorldSession *m_session;

		bool   m_dontMove;
		uint16 m_teleportMapId;

		uint32 m_team;
		uint32 m_nextSave;

		uint32 m_GMFlags;

		uint8  m_reborn;
		uint8  m_element;
		uint8  m_gender;
		uint8  m_face;
		uint8  m_hair;
		uint32 m_hair_color;
		uint32 m_skin_color;

		uint16 m_eq_head, m_eq_body, m_eq_wrist, m_eq_weapon, m_eq_shoe, m_eq_accsr;

		uint16 m_hp, m_sp;

		uint16 m_stat_int, m_stat_atk, m_stat_def, m_stat_agi, m_stat_hpx, m_stat_spx;
		uint8  m_level;
		uint32 m_xp_gain;
		uint16 m_skill;
		uint16 m_stat;

		uint16 m_hp_max;
		uint16 m_sp_max;

		uint32 m_atk_bonus, m_def_bonus, m_int_bonus, m_agi_bonus, m_hpx_bonus, m_spx_bonus;

	private:
		GridReference<Player> m_gridRef;
};

#endif
