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
#include "BattleSystem.h"

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

	for(uint8 petslot = 0; petslot < MAX_PET_SLOT; petslot++)
	{
		m_petSlot[petslot] = NULL;
	}

	m_nextSave = sWorld.getConfig(CONFIG_INTERVAL_SAVE);

	// randomize first save time in range [CONFIG_INTERVAL_SAVE] around [CONFIG_INTERVAL_SAVE]
	// this must help in case next save after mass player load after server startup
	m_nextSave = rand32(m_nextSave/2,m_nextSave*3/2);

	m_dontMove = false;

	PlayerTalkClass = new PlayerMenu( GetSession() );
	PlayerBattleClass = new BattleSystem( GetSession() );
}

Player::~Player ()
{
	if( GetSession()->PlayerLogout() )
	{
		sLog.outDebug( "Player '%s' is logged out", GetName());

		WorldPacket data;

		data.Initialize( 0x01, 1 );
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

	if(PlayerBattleClass->isActionTimedOut())
	{
		PlayerBattleClass->UpdateBattleAction();
	}

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

	Object::_Create( guid, HIGHGUID_PLAYER );

	// overwrite possible wrong/corrupted guid
	SetUInt64Value(OBJECT_FIELD_GUID,MAKE_GUID(guid,HIGHGUID_PLAYER));

	// TODO: cleaup invetory here see mangos Player.cpp












	m_name = f[FD_CHARNAME].GetCppString();

	sLog.outDebug(">> Load Basic value of player %s is: ", m_name.c_str());

	m_element = f[FD_ELEMENT].GetUInt8();
	SetUInt8Value(UNIT_FIELD_ELEMENT, f[FD_ELEMENT].GetUInt8());
	m_gender  = f[FD_GENDER].GetUInt8();
	m_reborn  = f[FD_REBORN].GetUInt8();

	Relocate(f[FD_POSX].GetUInt16(), f[FD_POSY].GetUInt16());

	SetMapId(f[FD_MAPID].GetUInt16());

	m_face       = f[FD_FACE].GetUInt8();
	m_hair       = f[FD_HAIR].GetUInt8();
	m_hair_color_R = f[FD_HAIR_COLOR_R].GetUInt8();
	m_hair_color_G = f[FD_HAIR_COLOR_G].GetUInt8();
	m_hair_color_B = f[FD_HAIR_COLOR_B].GetUInt8();
	m_skin_color_R = f[FD_SKIN_COLOR_R].GetUInt8();
	m_skin_color_G = f[FD_SKIN_COLOR_G].GetUInt8();
	m_skin_color_B = f[FD_SKIN_COLOR_B].GetUInt8();
	m_shirt_color = f[FD_SHIRT_COLOR].GetUInt8();
	m_misc_color = f[FD_MISC_COLOR].GetUInt8();

	m_eq_head   = f[FD_EQ_HEAD].GetUInt16();
	m_eq_body   = f[FD_EQ_BODY].GetUInt16();
	m_eq_wrist  = f[FD_EQ_WRIST].GetUInt16();
	m_eq_weapon = f[FD_EQ_WEAPON].GetUInt16();
	m_eq_shoe   = f[FD_EQ_SHOE].GetUInt16();
	m_eq_accsr  = f[FD_EQ_SPECIAL].GetUInt16();

	m_hp        = f[FD_HP].GetUInt16();
	SetUInt16Value(UNIT_FIELD_HP, f[FD_HP].GetUInt16());
	m_sp        = f[FD_SP].GetUInt16();
	SetUInt16Value(UNIT_FIELD_SP, f[FD_SP].GetUInt16());

	m_stat_int  = f[FD_ST_INT].GetUInt16();
	SetUInt16Value(UNIT_FIELD_INT, f[FD_ST_INT].GetUInt16());

	m_stat_atk  = f[FD_ST_ATK].GetUInt16();
	SetUInt16Value(UNIT_FIELD_ATK, f[FD_ST_ATK].GetUInt16());

	m_stat_def  = f[FD_ST_DEF].GetUInt16();
	SetUInt16Value(UNIT_FIELD_DEF, f[FD_ST_DEF].GetUInt16());

	m_stat_agi  = f[FD_ST_AGI].GetUInt16();
	SetUInt16Value(UNIT_FIELD_AGI, f[FD_ST_AGI].GetUInt16());

	m_stat_hpx  = f[FD_ST_HPX].GetUInt16();
	SetUInt16Value(UNIT_FIELD_HPX, f[FD_ST_HPX].GetUInt16());

	m_stat_spx  = f[FD_ST_SPX].GetUInt16();
	SetUInt16Value(UNIT_FIELD_SPX, f[FD_ST_SPX].GetUInt16());

	m_level     = f[FD_LEVEL].GetUInt8();
	SetUInt8Value(UNIT_FIELD_LEVEL, f[FD_LEVEL].GetUInt8());

	m_rank      = f[FD_RANK].GetUInt8();

	m_xp_gain   = f[FD_XP_GAIN].GetUInt32();
	m_skill     = f[FD_SKILL_GAIN].GetUInt16();
	m_stat      = f[FD_STAT_GAIN].GetUInt16();

	///- TODO: Calculate from equipment weared
	m_atk_mod   = 0;
	m_def_mod   = 0;
	m_int_mod   = 0;
	m_agi_mod   = 0;
	m_hpx_mod   = 0;
	m_spx_mod   = 0;

	///- Calculation must be after hpx & spx modifier applied
	m_hp_max    = ((f[FD_ST_HPX].GetUInt16() + m_hpx_mod) * 4) + 80 + m_level;
	SetUInt16Value(UNIT_FIELD_HP_MAX, ((f[FD_ST_HPX].GetUInt16() + m_hpx_mod) * 4) + 80 + GetUInt8Value(UNIT_FIELD_LEVEL));
	m_sp_max    = ((f[FD_ST_SPX].GetUInt16() + m_spx_mod) * 2) + 60 + m_level;
	SetUInt16Value(UNIT_FIELD_SP_MAX, ((f[FD_ST_SPX].GetUInt16() + m_spx_mod) * 4) + 80 + GetUInt8Value(UNIT_FIELD_LEVEL));

	m_gold_hand = f[FD_GOLD_IN_HAND].GetUInt32();
	m_gold_bank = f[FD_GOLD_IN_BANK].GetUInt32();

	m_unk1      = f[FD_UNK1].GetUInt16();
	m_unk2      = f[FD_UNK2].GetUInt16();
	m_unk3      = f[FD_UNK3].GetUInt16();
	m_unk4      = f[FD_UNK4].GetUInt16();
	m_unk5      = f[FD_UNK5].GetUInt16();

	sLog.outString(" - Name    : %s", GetName());
	sLog.outString(" - Level   : %u", m_level);
	sLog.outString(" - Element : %u", m_element);
	sLog.outString(" - HP & SP : %u/%u %u/%u", m_hp, m_hp_max, m_sp, m_sp_max);
	
	Object::_Create( guid, HIGHGUID_PLAYER );
	return true;
}

void Player::SaveToDB()
{
	CharacterDatabase.BeginTransaction();
	CharacterDatabase.PExecute("UPDATE characters SET mapid = '%u', pos_x = '%u', pos_y = '%u' WHERE accountid = '%u'", GetMapId(), GetPositionX(), GetPositionY(), GetAccountId());

	//std::ostringstream ss;
	//ss << "INSERT INTO characters (guid, accountid, charname, mapid, pos_x, pos_y, online
	CharacterDatabase.CommitTransaction();
}

bool Player::LoadPet()
{
	QueryResult *result = CharacterDatabase.PQuery("select * from character_pet where owner = '%u'", GetGUIDLow());

	if(!result)
		return false;

	uint8 petslot = 0;
	do
	{
		Field *f = result->Fetch();
		uint32 petguid = f[0].GetUInt32();
		Pet *pet = new Pet(this);
		if(!pet->LoadPetFromDB(this, petguid))
		{
			delete pet;
			continue;
		}

		m_petSlot[petslot++] = pet;
		sLog.outString(" - Slot %u pet Entry %u Model %u '%s'", petslot, pet->GetEntry(), pet->GetModelId(), pet->GetName()); 
		if(petslot > MAX_PET_SLOT)
			break;

	} while( result->NextRow() );

	delete result;

	return true;
}

void Player::BuildUpdateBlockVisibilityPacket(WorldPacket *data)
{
	data->Initialize( 0x03, 1 );

	*data << GetAccountId();
	*data << m_gender;
	*data << (uint16) 0x0000; // unknown fix from aming
	*data << GetMapId();
	*data << GetPositionX();
	*data << GetPositionY();

	*data << (uint8) 0x00;
	*data << m_hair;
	*data << m_face;
	*data << m_hair_color_R;
	*data << m_hair_color_G;
	*data << m_hair_color_B;
	*data << m_skin_color_R;
	*data << m_skin_color_G;
	*data << m_skin_color_B;
	*data << m_shirt_color;
	*data << m_misc_color;

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

}

void Player::BuildUpdateBlockVisibilityForOthersPacket(WorldPacket *data)
{
	data->Initialize( 0x03, 1 );
	*data << GetAccountId();
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
	*data << m_hair_color_R;
	*data << m_hair_color_G;
	*data << m_hair_color_B;
	*data << m_skin_color_R;
	*data << m_skin_color_G;
	*data << m_skin_color_B;
	*data << m_shirt_color;
	*data << m_misc_color;

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

	UpdatePetCarried();
	UpdatePet();

	BuildUpdateBlockStatusPacket(&data);
	GetSession()->SendPacket(&data);

	//SendMotd();

	AllowPlayerToMove();
	SendUnknownImportant();
}

void Player::SendInitialPacketsAfterAddToMap()
{
	UpdatePlayer();
	UpdateCurrentGold();
}

void Player::UpdatePetCarried()
{
	WorldPacket data;

	data.Initialize( 0x0F );
	data << (uint8 ) 0x08;
	uint8 petcount = 0;
	for(uint8 slot = 0; slot < MAX_PET_SLOT; slot++)
	{
		Pet* pet = m_petSlot[slot];
		if( !pet )
			continue;

		petcount++;
		data << (uint8 ) petcount;
		data << (uint16) pet->GetModelId(); // pet npc id ;
		data << (uint32) 0; // pet total xp 0x010D9A19

		data << (uint8 ) pet->GetUInt8Value(UNIT_FIELD_LEVEL); // pet level

		data << pet->GetUInt16Value(UNIT_FIELD_HP);  // pet current hp
		data << pet->GetUInt16Value(UNIT_FIELD_SP);  // pet current sp
		data << pet->GetUInt16Value(UNIT_FIELD_INT); // pet base stat int
		data << pet->GetUInt16Value(UNIT_FIELD_ATK); // pet base stat atk
		data << pet->GetUInt16Value(UNIT_FIELD_DEF); // pet base stat def
		data << pet->GetUInt16Value(UNIT_FIELD_AGI); // pet base stat agi
		data << pet->GetUInt16Value(UNIT_FIELD_HPX); // pet base stat hpx
		data << pet->GetUInt16Value(UNIT_FIELD_SPX); // pet base stat spx

		data << (uint8 ) 5;    // stat point left ?
		data << (uint8 ) pet->GetLoyalty(); // pet loyalty
		data << (uint8 ) 0x01; // unknown
		data << (uint16) pet->GetSkillPoint(); //skill point

		std::string tmpname = pet->GetName();
		data << (uint8 ) tmpname.size(); //name length
		data << tmpname.c_str();

		data << (uint8 ) pet->GetSkillLevel(0); //level skill #1
		data << (uint8 ) pet->GetSkillLevel(1); //level skill #2
		data << (uint8 ) pet->GetSkillLevel(2); //level skill #3

		uint8 durability = 0xFF;
		data << (uint16) pet->GetUInt16Value(UNIT_FIELD_EQ_HEAD);
		data << (uint8 ) durability;
		data << (uint32) 0 << (uint16) 0 << (uint8 ) 0; // unknown 7 byte

		data << (uint16) pet->GetUInt16Value(UNIT_FIELD_EQ_BODY);
		data << (uint8 ) durability;
		data << (uint32) 0 << (uint16) 0 << (uint8 ) 0; // unknown 7 byte

		data << (uint16) pet->GetUInt16Value(UNIT_FIELD_EQ_WRIST);
		data << (uint8 ) durability;
		data << (uint32) 0 << (uint16) 0 << (uint8 ) 0; // unknown 7 byte

		data << (uint16) pet->GetUInt16Value(UNIT_FIELD_EQ_WEAPON);
		data << (uint8 ) durability;
		data << (uint32) 0 << (uint16) 0 << (uint8 ) 0; // unknown 7 byte

		data << (uint16) pet->GetUInt16Value(UNIT_FIELD_EQ_SHOE);
		data << (uint8 ) durability;
		data << (uint32) 0 << (uint16) 0 << (uint8 ) 0; // unknown 7 byte

		data << (uint16) pet->GetUInt16Value(UNIT_FIELD_EQ_SPECIAL);
		data << (uint8 ) durability;
		data << (uint32) 0 << (uint16) 0 << (uint8 ) 0; // unknown 7 byte

		data << (uint16) 0x00;
	}

	if( !petcount )
		return;

	GetSession()->SendPacket(&data);
}

void Player::BuildUpdateBlockStatusPacket(WorldPacket *data)
{

	data->Initialize( 0x05, 1 );
	*data << (uint8) 0x03;                   // unknown
	*data << m_element; // uint8
	
	*data << m_hp; // uint16
	*data << m_sp; // uint16

	*data << (uint16) m_stat_int; //uint16
	*data << (uint16) m_stat_atk; //uint16
	*data << (uint16) m_stat_def; //uint16
	*data << (uint16) m_stat_agi; //uint16
	*data << (uint16) m_stat_hpx; //uint16
	*data << (uint16) m_stat_spx; //uint16

	*data << (uint8 ) m_level; // uint8
	*data << (uint32) m_xp_gain; //uint32
	*data << (uint16) m_skill; //uint16
	*data << (uint16) m_stat; // uint16

	*data << (uint32) 0; //m_rank;

	*data << (uint16) m_hp_max; // alogin use 32bit, while eXtreme use 16bit
	*data << (uint16) m_sp_max;
	*data << (uint16) 0;

	*data << (uint32) m_atk_mod;
	*data << (uint32) m_def_mod;
	*data << (uint32) m_int_mod;
	*data << (uint32) m_agi_mod;
	*data << (uint32) m_hpx_mod;
	*data << (uint32) m_spx_mod;

	*data << m_unk1; // this is Dong Wu moral 
	*data << m_unk2;
	*data << m_unk3;
	*data << m_unk4;
	*data << m_unk5;

	*data << (uint16) 0x00;
	*data << (uint16) 0x0000; //0x0101; // auto attack data ?

	*data << (uint32) 0x00 << (uint32) 0x00 << uint32(0x00) << uint32(0x00);
	*data << (uint32) 0x00 << (uint32) 0x00 << uint32(0x00) << uint32(0x00);
	*data << (uint32) 0x00 << (uint8 ) 0x00;

	///- Try to identify skill info packet
	*data << (uint16) 0x0000 << (uint8 ) 0x00;
	*data << (uint16) 11001; // Submerge
	*data << (uint8 ) 10;
	*data << (uint16) 14001; // Investigation
	*data << (uint8 ) 1;
}

void Player::UpdatePlayer()
{

	///- TODO: Update only when changes
	//_updatePlayer( 0x1B, 1, m_stat_int );
	//_updatePlayer( 0x1C, 1, m_stat_atk );
	//_updatePlayer( 0x1D, 1, m_stat_def );
	//_updatePlayer( 0x1E, 1, m_stat_agi );
	//_updatePlayer( 0x1F, 1, m_stat_hpx );
	//_updatePlayer( 0x20, 1, m_stat_spx );

	//_updatePlayer( 0x23, 1, m_level );
	//_updatePlayer( 0x24, 1, m_xp_gain );
	//_updatePlayer( 0x25, 1, m_skill );
	//_updatePlayer( 0x26, 1, m_stat );

	//_updatePlayer( 0x40, 1, 100 ); // loyalty


	///- This always updated before engaging
	_updatePlayer( 0x19, 1, m_hp );
	_updatePlayer( 0x1A, 1, m_sp );

	_updatePlayer( 0xCF, 1, m_hpx_mod );
	_updatePlayer( 0xD0, 1, m_spx_mod );
	_updatePlayer( 0xD2, 1, m_atk_mod );
	_updatePlayer( 0xD3, 1, m_def_mod );
	_updatePlayer( 0xD4, 1, m_int_mod );
	_updatePlayer( 0xD6, 1, m_agi_mod );
}

void Player::_updatePlayer(uint8 flagStatus, uint8 modifier, uint16 value)
{
	WorldPacket data;
	data.Initialize( 0x08 );
	data << (uint8 ) 0x01;            // flag status for main character
	data << flagStatus;               // flag status fields
	data << modifier;                 // +/- modifier
	data << value;                    // value modifier
	data << (uint32) 0 << (uint16) 0;
	GetSession()->SendPacket(&data);
}

void Player::UpdatePet()
{
	for(uint8 slot = 0; slot < MAX_PET_SLOT; slot++)
	{
		Pet* pet = m_petSlot[slot];
		if( !pet )
			continue;
		_updatePet(slot+1, 0xCF, 1, 0);
		_updatePet(slot+1, 0x19, 1, pet->GetUInt16Value(UNIT_FIELD_HP));
		_updatePet(slot+1, 0xD0, 1, 0);
		_updatePet(slot+1, 0x1A, 1, pet->GetUInt16Value(UNIT_FIELD_SP));
		_updatePet(slot+1, 0xD2, 1, 0);
		_updatePet(slot+1, 0xD3, 1, 0);
		_updatePet(slot+1, 0xD4, 1, 0);
		_updatePet(slot+1, 0xD6, 1, 0);
	}
}

void Player::UpdatePet(uint8 slot)
{
	Pet* pet = m_petSlot[slot];
	if( !pet )
		return;
	_updatePet(slot, 0xCF, 1, 0);
	_updatePet(slot, 0x19, 1, 0);//pet->GetUInt16Value(UNIT_FIELD_HP));
	_updatePet(slot, 0xD0, 1, 0);
	_updatePet(slot, 0x1A, 1, 0);//pet->GetUInt16Value(UNIT_FIELD_SP));
	_updatePet(slot, 0xD2, 1, 0);
	_updatePet(slot, 0xD3, 1, 0);
	_updatePet(slot, 0xD4, 1, 0);
	_updatePet(slot, 0xD6, 1, 0);
}

void Player::_updatePet(uint8 slot, uint8 flagStatus, uint8 modifier, uint32 value)
{
	WorldPacket data;
	data.Initialize( 0x08 );
	data << (uint8 ) 0x02 << (uint8 ) 0x04;
	data << slot; // pet slot number
	data << (uint8 ) 0x00;
	data << flagStatus;
	data << modifier;
	data << value;
	data << (uint32) 0;
	GetSession()->SendPacket(&data);
}

void Player::UpdateCurrentEquipt()
{
	WorldPacket data;
	data.Initialize( 0x05, 1 );
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
	data.Initialize( 0x1A, 1 );
	data << (uint8 ) 0x04;
	data << (uint32) m_gold_hand; // gold
	data << (uint32) 0x00000000;
	GetSession()->SendPacket(&data);
}

void Player::Send0504()
{
	///- tell the client to wait for request
	WorldPacket data;
	data.Initialize( 0x05, 1 );
	data << (uint8) 0x04;
	GetSession()->SendPacket(&data);
}

void Player::Send0F0A()
{
	WorldPacket data;
	data.Initialize( 0x0F, 1 );
	data << (uint8) 0x0A;
	GetSession()->SendPacket(&data);
}

void Player::Send0602()
{
	WorldPacket data;
	data.Initialize( 0x06, 1 );
	data << (uint8) 0x02;
	GetSession()->SendPacket(&data);
}

void Player::Send1408()
{
	///- tell the client request is completed
	WorldPacket data;
	data.Initialize( 0x14, 1 );
	data << (uint8) 0x08;
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
	data->Initialize( 0x0C, 1 );
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
	/* this is working one */
	/*
	sLog.outString("Player::UpdateVisibilityOf '%s' to '%s'",
		GetName(), target->GetName());
	target->SendUpdateToPlayer(this);
	SendUpdateToPlayer((Player*) target);
	*/
	if(HaveAtClient(target))
	{
		//if(!target->isVisibleForInState(this, true))
		{
			//target->DestroyForPlayer(this);
			m_clientGUIDs.erase(target->GetGUID());

		}
	}
	else
	{
		//if(target->isVisibleForInState(this,false))
		{
			if(!target->isType(TYPE_PLAYER))
				return;

			target->SendUpdateToPlayer(this);
		}
	}
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
	data.Initialize( 0x0C, 1 );
	data << GetAccountId();
	data << GetMapId();
	data << GetPositionX();
	data << GetPositionY();
	data << (uint8) 0x01 << (uint8) 0x00;
	GetSession()->SendPacket(&data);


	Send0504();  // This is important, maybe tell the client to Wait Cursor

	Map* map = MapManager::Instance().GetMap(GetMapId(), this);
	map->Add(this);

	UpdateMap2Npc();

	SetDontMove(false);
}

void Player::UpdateRelocationToSet()
{
	///- Send Relocation Message to Set
	WorldPacket data;

	data.Initialize( 0x06, 1 );
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
