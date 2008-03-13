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
#include "Log.h"
#include "Opcodes.h"
#include "ObjectMgr.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"

#include "GossipDef.h"

#include "MapManager.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "ObjectMgr.h"
#include "ObjectAccessor.h"
#include "Util.h"
#include "Database/DatabaseImpl.h"

#include <cmath>

Player::Player (WorldSession *session): Unit( 0 )
{
	m_session = session;

	m_objectType |= TYPE_PLAYER;
	m_objectTypeId = TYPEID_PLAYER;

	m_valuesCount = PLAYER_END;

	m_GMFlags = 0;

	m_nextSave = sWorld.getConfig(CONFIG_INTERVAL_SAVE);

	// randomize first save time in range [CONFIG_INTERVAL_SAVE] around [CONFIG_INTERVAL_SAVE]
	// this must help in case next save after mass player load after server startup
	m_nextSave = rand32(m_nextSave/2,m_nextSave*3/2);

	m_dontMove = false;

	PlayerTalkClass = new PlayerMenu( GetSession() );
}

Player::~Player ()
{
	if( GetSession()->PlayerLogout() )
	{
		WorldPacket data;
		sLog.outDebug( "Player '%s' is logged out", GetName());
		data.clear(); data.SetOpcode( 0x01 ); data.Prepare();
		data << (uint8) 0x01;
		data << GetAccountId();
		SendMessageToSet(&data, false);
		return;
	}
}

bool Player::Create( uint32 accountId, WorldPacket& data)
{
	uint8 element, gender, skin, face, hair, hairColor;

	Object::_Create(accountId, HIGHGUID_PLAYER);

	data >> m_name;
	data >> element;
	data >> gender;
	// later on
	
	return true;
}

void Player::Update( uint32 p_time )
{
	if(!IsInWorld())
		return;

	Unit::Update( p_time );

	if(m_nextSave > 0)
	{
		if(p_time >= m_nextSave)
		{
			// m_nextSave reseted in SaveToDB call
			SaveToDB();
	//		sLog.outDetail("Player '%u' '%s' Saved", GetAccountId(), GetName());
		}
		else
		{
			m_nextSave -= p_time;
		}
	}
}

void Player::SendMessageToSet(WorldPacket *data, bool self)
{
//	sLog.outString("Player::SendMessageToSet");
	MapManager::Instance().GetMap(GetMapId(), this)->MessageBroadcast(this, data, self);
}

bool Player::LoadFromDB( uint32 guid, SqlQueryHolder *holder)
{
	QueryResult *result = holder->GetResult(PLAYER_LOGIN_QUERY_LOADFROM);
	
	if(!result)
	{
		sLog.outError("ERROR: Player (GUID: %u) not found in table `characters`,can't load.", guid);
		return false;
	}

	Field *f = result->Fetch();

	uint32 dbAccountId = f[1].GetUInt32();

	// check if the character's account in the db and the logged in account match.
	// player should be able to load/delete only with correct account!
	if( dbAccountId != GetSession()->GetAccountId() )
	{
		sLog.outError("ERROR: Player (GUID: %u) loading from wrong account (is: %u, should be: %u)", guid, GetSession()->GetAccountId(),dbAccountId);
		delete result;
		return false;
	}

	m_name = f[FD_CHARNAME].GetCppString();

	sLog.outDebug("Load Basic value of player %s is: ", m_name.c_str());

	m_element = f[FD_ELEMENT].GetUInt8();
	m_gender  = f[FD_GENDER].GetUInt8();
	m_reborn  = f[FD_REBORN].GetUInt8();

	Relocate(f[FD_POSX].GetUInt16(), f[FD_POSY].GetUInt16());

	SetMapId(f[FD_MAPID].GetUInt16());

	m_face       = f[FD_FACE].GetUInt8();
	m_hair       = f[FD_HAIR].GetUInt8();
	m_hair_color = f[FD_HAIR_COLOR].GetUInt32();
	m_skin_color = f[FD_SKIN_COLOR].GetUInt32();

	m_eq_head   = f[FD_EQ_HEAD].GetUInt16();
	m_eq_body   = f[FD_EQ_BODY].GetUInt16();
	m_eq_wrist  = f[FD_EQ_WRIST].GetUInt16();
	m_eq_weapon = f[FD_EQ_WEAPON].GetUInt16();
	m_eq_shoe   = f[FD_EQ_SHOE].GetUInt16();
	m_eq_accsr  = f[FD_EQ_ACCSR].GetUInt16();

	m_hp        = f[FD_HP].GetUInt16();
	m_sp        = f[FD_SP].GetUInt16();

	m_stat_int  = f[FD_ST_INT].GetUInt16();
	m_stat_atk  = f[FD_ST_ATK].GetUInt16();
	m_stat_def  = f[FD_ST_DEF].GetUInt16();
	m_stat_agi  = f[FD_ST_AGI].GetUInt16();
	m_stat_hpx  = f[FD_ST_HPX].GetUInt16();
	m_stat_spx  = f[FD_ST_SPX].GetUInt16();

	m_level     = f[FD_LEVEL].GetUInt8();
	m_xp_gain   = f[FD_XP_GAIN].GetUInt32();
	m_skill     = f[FD_SKILL_GAIN].GetUInt16() - f[FD_SKILL_USED].GetUInt16();
//	if (m_skill < 0) m_skill = 0;
	m_stat      = f[FD_STAT_GAIN].GetUInt16() - f[FD_STAT_USED].GetUInt16();
//	if (m_stat < 0) m_stat = 0;

	m_hp_max    = (f[FD_ST_HPX].GetUInt16() * 4) + 80 + m_level;
	m_sp_max    = (f[FD_ST_SPX].GetUInt16() * 2) + 60 + m_level;
	
	m_atk_bonus = f[FD_ST_ATK_BONUS].GetUInt32();
	m_def_bonus = f[FD_ST_DEF_BONUS].GetUInt32();
	m_int_bonus = f[FD_ST_INT_BONUS].GetUInt32();
	m_agi_bonus = f[FD_ST_AGI_BONUS].GetUInt32();
	m_hpx_bonus = f[FD_ST_HPX_BONUS].GetUInt32();
	m_spx_bonus = f[FD_ST_SPX_BONUS].GetUInt32();

	m_gold_hand = f[FD_GOLD_IN_HAND].GetUInt32();
	m_gold_bank = f[FD_GOLD_IN_BANK].GetUInt32();

	m_unk1      = f[FD_UNK1].GetUInt16();
	m_unk2      = f[FD_UNK2].GetUInt16();
	m_unk3      = f[FD_UNK3].GetUInt16();
	m_unk4      = f[FD_UNK4].GetUInt16();
	m_unk5      = f[FD_UNK5].GetUInt16();
	
	Object::_Create( guid, HIGHGUID_PLAYER );
	return true;
}

void Player::SaveToDB()
{
}

void Player::BuildUpdateBlockVisibilityPacket(WorldPacket *data)
{
//	WorldPacket data ( (uint8) 0x03, 5);   // Player Info
	data->clear(); data->SetOpcode( 0x03 ); data->Prepare();

	*data << GetAccountId();
	*data << m_gender;
	*data << (uint16) 0x0000; // unknown fix from aming
	*data << GetMapId();
	*data << GetPositionX();
	*data << GetPositionY();

	*data << (uint8) 0x00;
	*data << m_hair;
	*data << m_face;
	*data << m_skin_color;
	*data << m_hair_color;

	uint8  equip_cnt = 0;
	uint16 equip[6];
	if (m_eq_head)   equip[equip_cnt++] = m_eq_head;
	if (m_eq_body)   equip[equip_cnt++] = m_eq_body;
	if (m_eq_wrist)  equip[equip_cnt++] = m_eq_wrist;
	if (m_eq_weapon) equip[equip_cnt++] = m_eq_weapon;
	if (m_eq_shoe)   equip[equip_cnt++] = m_eq_shoe;
	if (m_eq_accsr)  equip[equip_cnt++] = m_eq_accsr;
	*data << (uint8) equip_cnt;
	for (int i = 0; i < equip_cnt; i++) *data << equip[i];

	// unknown
	*data << (uint16) 0x00 << (uint16) 0x00 << (uint16) 0x00;
	*data << m_reborn;
	*data << m_name.c_str();

//	GetSession()->SendPacket(&data);
}

void Player::BuildUpdateBlockVisibilityForOthersPacket(WorldPacket *data)
{

	data->clear(); data->SetOpcode( 0x03 ); data->Prepare();
	*data << GetAccountId();
	//*data << (uint8) 0x01 << (uint8) 0x01;
	*data << m_gender;
	*data << m_element;
	*data << m_level;
	*data << (uint16) 0x0000;
	*data << GetMapId();
	*data << GetPositionX();
	*data << GetPositionY();
	*data << (uint8) 0x00;// << (uint16) 0x0000;
	*data << m_hair;
	*data << m_face;
	*data << m_skin_color;
	*data << m_hair_color;

	uint8  equip_cnt = 0;
	uint16 equip[6];
	if (0 != m_eq_head)   equip[equip_cnt++] = m_eq_head;
	if (0 != m_eq_body)   equip[equip_cnt++] = m_eq_body;
	if (0 != m_eq_wrist)  equip[equip_cnt++] = m_eq_wrist;
	if (0 != m_eq_weapon) equip[equip_cnt++] = m_eq_weapon;
	if (0 != m_eq_shoe)   equip[equip_cnt++] = m_eq_shoe;
	if (0 != m_eq_accsr)  equip[equip_cnt++] = m_eq_accsr;
	*data << (uint8) equip_cnt;
	for (int i = 0; i < equip_cnt; i++) *data << equip[i];

	*data << (uint32) 0x00000000 << (uint16) 0x0000; // << (uint8) 0x02;
	*data << (uint8 ) m_reborn; //0x00;
	*data << (uint8 ) 0x00;
	*data << m_name.c_str();

}

void Player::SendInitialPacketsBeforeAddToMap()
{
	WorldPacket data;

	BuildUpdateBlockVisibilityPacket(&data);
	GetSession()->SendPacket(&data);

	BuildUpdateBlockStatusPacket(&data);
	GetSession()->SendPacket(&data);

	// TODO: Move to Broadcast handler
	/*
	data.clear(); data.SetOpcode( 0x21 ); data.Prepare();
	data << (uint8) 0x02 << (uint8) 0x00 << (uint8) 0x01;
	GetSession()->SendPacket(&data);
	*/

	AllowPlayerToMove();
	SendUnknownImportant();

}

void Player::SendInitialPacketsAfterAddToMap()
{
	UpdateCurrentStatus();
	UpdateCurrentGold();
}

void Player::BuildUpdateBlockStatusPacket(WorldPacket *data)
{

	data->clear(); data->SetOpcode( 0x05 ); data->Prepare();
	*data << (uint8) 0x03;                   // unknown
	*data << m_element;
	
	*data << m_hp;
	*data << m_sp;

	*data << m_stat_int;
	*data << m_stat_atk;
	*data << m_stat_def;
	*data << m_stat_agi;
	*data << m_stat_hpx;
	*data << m_stat_spx;

	*data << m_level;
	*data << m_xp_gain;
	*data << m_skill;
	*data << m_stat;

	*data << (uint8) 0x00 << (uint8) 0x07 << (uint8) 0x01 << (uint8) 0x00;

	*data << m_hp_max;
	*data << m_sp_max;

	*data << m_atk_bonus;
	*data << m_def_bonus;
	*data << m_int_bonus;
	*data << m_agi_bonus;
	*data << m_hpx_bonus;
	*data << m_spx_bonus;

	/* unknow fields - TODO: Identify */
	/*
	*data << (uint8) 0xF4 << (uint8) 0x01;
	*data << (uint8) 0xF4 << (uint8) 0x01;
	*data << (uint8) 0xF4 << (uint8) 0x01;
	*data << (uint8) 0xF4 << (uint8) 0x01;
	*data << (uint8) 0xF4 << (uint8) 0x01;
	*/
	*data << m_unk1;
	*data << m_unk2;
	*data << m_unk3;
	*data << m_unk4;
	*data << m_unk5;
	


	*data << (uint16) 0x00;
	*data << (uint16) 0x0101;

	*data << (uint32) 0x00 << (uint32) 0x00 << uint32(0x00) << uint32(0x00);
	*data << (uint32) 0x00 << (uint32) 0x00 << uint32(0x00) << uint32(0x00);
	*data << (uint32) 0x00 << (uint8 ) 0x00;
}

void Player::UpdateCurrentStatus()
{
	WorldPacket data;
	data.clear(); data.SetOpcode( 0x08 ); data.Prepare();
	data << (uint8) 0x01;    // Update status for Character
	data << (uint8) 0x19;    // update status for Current Health
	data << (uint8) 0x01;    // positive value
	data << m_hp;            // value of health
	data << (uint16) 0x0000; // unknown value
	GetSession()->SendPacket(&data);

	data.clear(); data.SetOpcode( 0x08 ); data.Prepare();
	data << (uint8) 0x01;    // Update status for Character
	data << (uint8) 0x1A;    // update status for Current Health
	data << (uint8) 0x01;    // positive value
	data << m_sp;            // value of health
	data << (uint16) 0x0000; // unknown value
	GetSession()->SendPacket(&data);
}

void Player::UpdateCurrentEquipt()
{
	WorldPacket data;
	data.clear(); data.SetOpcode( 0x05 ); data.Prepare();
	data << (uint8) 0x00;
	data << GetAccountId();

	uint8  equip_cnt = 0;
	uint16 equip[6];
	if (0 != m_eq_head)   equip[equip_cnt++] = m_eq_head;
	if (0 != m_eq_body)   equip[equip_cnt++] = m_eq_body;
	if (0 != m_eq_wrist)  equip[equip_cnt++] = m_eq_wrist;
	if (0 != m_eq_weapon) equip[equip_cnt++] = m_eq_weapon;
	if (0 != m_eq_shoe)   equip[equip_cnt++] = m_eq_shoe;
	if (0 != m_eq_accsr)  equip[equip_cnt++] = m_eq_accsr;
	//data << (uint8) equip_cnt;
	for (int i = 0; i < equip_cnt; i++) data << equip[i];
	GetSession()->SendPacket(&data);
}

void Player::UpdateCurrentGold()
{
	WorldPacket data;
	data.clear(); data.SetOpcode( 0x1A ); data.Prepare();
	data << (uint8 ) 0x04;
	data << (uint32) m_gold_hand; // gold
	data << (uint32) 0x00000000;
	GetSession()->SendPacket(&data);
}

void Player::Send0504()
{
	///- tell the client to wait for request
	WorldPacket data; data.clear(); data.SetOpcode(0x05);
	data.Prepare(); data << (uint8) 0x04;
	GetSession()->SendPacket(&data);
}

void Player::Send0F0A()
{
	WorldPacket data; data.clear(); data.SetOpcode(0x0F);
	data.Prepare(); data << (uint8) 0x0A;
	GetSession()->SendPacket(&data);
}

void Player::Send0602()
{
	WorldPacket data; data.clear(); data.SetOpcode(0x06);
	data.Prepare(); data << (uint8) 0x02;
	GetSession()->SendPacket(&data);
}

void Player::Send1408()
{
	///- tell the client request is completed
	WorldPacket data; data.clear(); data.SetOpcode(0x14);
	data.Prepare(); data << (uint8) 0x08;
	GetSession()->SendPacket(&data);
}

void Player::AllowPlayerToMove()
{
	Send0504(); Send0F0A(); //EndOfRequest();
}

void Player::EndOfRequest()
{
	Send1408();
}

void Player::BuildUpdateBlockTeleportPacket(WorldPacket* data)
{
	data->clear(); data->SetOpcode( 0x0C ); data->Prepare();
	*data << GetAccountId();
	*data << GetTeleportTo();
	*data << GetPositionX();
	*data << GetPositionY();
	*data << (uint8) 0x01 << (uint8) 0x00;
}


void Player::AddToWorld()
{
	///- Do not add/remove the player from the object storage
	///- It will crash when updating the ObjectAccessor
	///- The player should only be added when logging in
	Object::AddToWorld();

}

void Player::RemoveFromWorld()
{
	///- Do not add/remove the player from the object storage
	///- It will crash when updating the ObjectAccessor
	///- The player should only be removed when logging out
	Object::RemoveFromWorld();
}

void Player::SendDelayResponse(const uint32 ml_seconds)
{
}

void Player::UpdateVisibilityOf(WorldObject* target)
{
	sLog.outString("Player::UpdateVisibilityOf '%s' to '%s'",
		GetName(), target->GetName());
	//target->SendUpdateToPlayer(this);
	SendUpdateToPlayer((Player*) target);
}

void Player::TeleportTo(uint16 mapid, uint16 pos_x, uint16 pos_y)
{
	sLog.outDebug( "PLAYER: Teleport Player '%s' to %u [%u,%u]", GetName(), mapid, pos_x, pos_y );

	WorldPacket data;

	SetDontMove(true);

	Map* map = MapManager::Instance().GetMap(GetMapId(), this);
	map->Remove(this, false);

	SetMapId(mapid);
	Relocate(pos_x, pos_y);

	// update player state for other and vice-versa
	CellPair p = LeGACY::ComputeCellPair(GetPositionX(), GetPositionY());
	Cell cell(p);
	MapManager::Instance().GetMap(GetMapId(), this)->EnsureGridLoadedForPlayer(cell, this, true);
}
	
void Player::SendMapChanged()
{
	WorldPacket data;
	data.clear(); data.SetOpcode( 0x0C ); data.Prepare();
	data << GetAccountId();
	data << GetMapId();
	data << GetPositionX();
	data << GetPositionY();
	data << (uint8) 0x01 << (uint8) 0x00;
	GetSession()->SendPacket(&data);

	///- Unknown data after map changed
	/*
	data.clear(); data.SetOpcode( 0x18 ); data.Prepare();
	data << (uint32) 0x00584808 << (uint32) 0x05000200;
	GetSession()->SendPacket(&data);
	*/

	Send0504();  // This is important, maybe tell the client to Wait Cursor

	/*
	UpdateCurrentEquipt();
	*/

	Map* map = MapManager::Instance().GetMap(GetMapId(), this);
	map->Add(this);

	UpdateMap2Npc();

	SetDontMove(false);
}

void Player::UpdateRelocationToSet()
{
	///- Send Relocation Message to Set
	WorldPacket data;
	data.clear(); data.SetOpcode( 0x06 ); data.Prepare();
	data << (uint8) 0x01;
	data << GetAccountId();
	data << (uint8) 0x05;
	data << GetPositionX() << GetPositionY();
	SendMessageToSet(&data, false);
}

bool Player::HasSpell(uint32 spell) const
{
	return false;
}

void Player::TalkedToCreature( uint32 entry, uint64 guid )
{
}
