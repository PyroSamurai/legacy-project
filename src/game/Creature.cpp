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
#include "World.h"
#include "ObjectMgr.h"

#include "Creature.h"
//#include "GossipDef.h"
#include "Player.h"
#include "Opcodes.h"
#include "Log.h"

#include "MapManager.h"
#include "CreatureAI.h"
#include "CreatureAISelector.h"


// apply implementation of the singletons
#include "Policies/SingletonImp.h"

Creature::Creature( WorldObject *instantiator ) :
Unit( instantiator ),
m_NPCTextId(0),
i_AI( NULL ),
m_itemsLoaded( false ),
m_respawnTime( 0 ),
m_respawnDelay( 25 ),
m_isPet( false ),
m_defaultMovementType( IDLE_MOTION_TYPE ),
m_AI_locked( false ),
m_puppet( false )
{
	m_valuesCount = UNIT_END;

	for(int i = 0; i < 2; ++i) respawn_cord[i] = 0;
}

Creature::Creature( WorldObject *instantiator, bool puppet ) :
Unit( instantiator ),
m_NPCTextId(0),
i_AI( NULL ),
m_itemsLoaded( false ),
m_respawnTime( 0 ),
m_respawnDelay( 25 ),
m_isPet( false ),
m_defaultMovementType( IDLE_MOTION_TYPE ),
m_AI_locked( false ),
m_puppet( puppet )
{
	m_valuesCount = UNIT_END;

	for(int i = 0; i < 2; ++i) respawn_cord[i] = 0;
}

Creature::~Creature()
{
	m_vendor_items.clear();

	delete i_AI;
	i_AI = NULL;
}

void Creature::AddToWorld()
{
	///- Register the creature for guid lookup
	if(!IsInWorld())
	{
		sLog.outString("Creature::AddToWorld GUID(%u) MAPID(%u) ------", GetGUIDLow(), GetMapId());
		ObjectAccessor::Instance().AddObject(this);
	}
	Object::AddToWorld();
}

void Creature::RemoveFromWorld()
{
	///- Remove the creature from the accessor
	if(IsInWorld()) ObjectAccessor::Instance().RemoveObject(this);
	Object::RemoveFromWorld();
}

void Creature::Update(uint32 diff)
{
	//sLog.outString("***************** Creature Update *****************");
	switch( m_deathState )
	{
		case DEAD:
		{
			if( m_respawnTime <= time(NULL) )
			{
				DEBUG_LOG("Respawning...");
				clearUnitState(UNIT_STATE_ALL_STATE);
			}
			break;
		case ALIVE:
		{
			Unit::Update( diff );
			
			// creature can be dead after Unit::Update call
			if(!isAlive())
				break;
		}
		default:
			break;
		}
	}
}


bool Creature::AIM_Initialize()
{
	// make sure nothing can change the AI during AI update
	if(m_AI_locked)
	{
		sLog.outDebug("AIM_Initialize: failed to init, locked.");
		return false;
	}

	CreatureAI * oldAI = i_AI;
	i_motionMaster.Initialize();
	i_AI = FactorySelector::selectAI(this);
	if(oldAI)
		delete oldAI;
	return true;
}


bool Creature::Create(uint32 guidlow, uint16 mapid, uint16 x, uint16 y, uint32 Entry, uint32 team, const CreatureData *data)
{
	respawn_cord[0] = x;
	respawn_cord[1] = y;
	SetMapId(mapid);
	Relocate(x, y);

	//sLog.outString("    Creature::Create GUID(%u) MAPID(%u) ENTRY(%u)", guidlow, GetMapId(), Entry);

	return CreateFromProto(guidlow, Entry, team, data);
}

void Creature::prepareGossipMenu(Player *pPlayer, uint32 gossidid)
{

}

void Creature::sendPreparedGossip(Player* player)
{
	if(!player)
		return;

	uint8 mapNpcId = GetMapNpcId();//(GetGUIDLow() / 100) - GetMapId();
	player->PlayerTalkClass->SendGossipMenu( mapNpcId, GetNpcTextId() );
}

void Creature::OnGossipSelect(Player* player, uint32 sequence, uint32 option)
{
	if(option == GOSSIP_OPTION_END)
	{
		player->PlayerTalkClass->CloseMenu();
		player->EndOfRequest();
		return;
	}

	QueryResult *result = WorldDatabase.PQuery("SELECT action, gossip_type, textid FROM npc_gossip WHERE npc_guid = %u AND sequence = %u", GetGUIDLow(), sequence);

	if(!result)
	{
		player->PlayerTalkClass->SendTalking(GetMapNpcId(), DEFAULT_GOSSIP_MESSAGE, GOSSIP_TYPE_PLAIN);
		return;
	}

	Field *f = result->Fetch();
	uint32 action = f[0].GetUInt32();

	sLog.outDebug(" >> Action Required %u", action);
	if(action != option - GOSSIP_OPTION_START)
		return;

	uint8  gossip_type = f[1].GetUInt8();
	uint16 textId      = f[2].GetUInt16();
	sLog.outDebug(" >> Response with gossip type %u, textid %u", gossip_type, textId);
	player->PlayerTalkClass->SendTalking(GetMapNpcId(), textId, gossip_type);

}
/*
uint32 Creature::GetGossipCount( uint32 gossipid )
{
	uint32 count=0;
	for( GossipOptionList::iterator i = m_goptions.begin(); i != m_goptions.end(); i++ )
	{
		if(i->GossipId == gossipid)
			count++;
	}
	return count;
}
*/
uint16 Creature::GetNpcTextId()
{
	sLog.outString("Creature::GetNpcTextId GUID(%u) TextId(%u)", GetGUIDLow(), m_NPCTextId );
	// already loaded and cached
	//if(m_NPCTextId)
	//	return m_NPCTextId;

	QueryResult* result = WorldDatabase.PQuery("SELECT textid FROM npc_gossip WHERE npc_guid = '%u' AND sequence = 0", GetGUIDLow());
	if(!result)
		m_NPCTextId = DEFAULT_GOSSIP_MESSAGE;
	else
	{
		Field *f = result->Fetch();
		m_NPCTextId = f[0].GetUInt16();
		delete result;
	}
	sLog.outString("NpcTextId: %u", m_NPCTextId);
	return m_NPCTextId;
}

GossipItem Creature::GetNpcGossip(uint32 sequence, uint32 action)
{
	///- TODO: Implement lazy loading later on

	///- TODO: Change to WHERE entry = GetEntry() for more generalization
	
	QueryResult *result = WorldDatabase.PQuery("SELECT textid, gossip_type FROM npc_gossip WHERE npc_guid = %u AND sequence = %u AND (action = %u or action = 0)", GetGUIDLow(), sequence, action - GOSSIP_OPTION_START);

	GossipItem go;

	if(!result)
		return go;

	Field *f = result->Fetch();
	go.TextId = f[0].GetUInt16();
	go.Type   = f[1].GetUInt8();

	delete result;

	return go;
}

/*
std::string Creature::GetGossipTitle(uint8 type, uint32 id)
{
	for(GossipOptionList::iterator i = m_goptions.begin(); i != m_goptions.end(); i++)
	{
		if(i->Id==id && i->NpcFlag==(uint32) type)
			return i->Option;
	}
	return "";
}

GossipOption const* Creature::GetGossipOption( uint32 id ) const
{
	for(GossipOptionList::const_iterator i = m_goptions.begin(); i != m_goptions.end(); i++)
	{
		if(i->Action==id)
			return &*i;
	}
	return NULL;
}

void Creature::LoadGossipOptions()
{
	if(m_gossipOptionLoaded)
		return;

	uint32 npcflags=GetUInt32Value(UNIT_NPC_FLAGS);

	QueryResult *result = WorldDatabase.PQuery("SELECT id, gossip_id, npcflag, action, option_text FROM npc_option WHERE (npcflag & %u) <> 0", npcflags);

	if(!result)
		return;

	GossipOptions go;
	do
	{
		Field *f = result->Fetch();
		go.Id       = f[0].GetUInt32();
		go.GossipId = f[1].GetUInt32();
		go.NpcFlag  = f[2].GetUInt32();
		go.Action   = f[3].GetUInt32();
		go.Option   = f[5].GetCppString();
		addGossipOption(go);
	} while (result->NextRow() );
	delete result;

	m_gossipOptionLoaded = true;
	return NULL;
}

void Creature::AI_SendMoveToPacket(uint16 x, uint16 y, uint32 time, uint8 type)
{
	SendMonsterMove(x, y, type, time);
}
*/
void Creature::SaveToDB()
{
	// update in loaded data
	CreatureData& data = objmgr.NewOrExistCreatureData(m_DBTableGuid);

	// data->guid = guid don't must be update at save
	data.id    = GetEntry();
	data.mapid = GetMapId();
	data.posX  = GetPositionX();
	data.posY  = GetPositionY();
	data.spawntimesecs = m_respawnDelay;
	data.spawn_posX = respawn_cord[0];
	data.spawn_posY = respawn_cord[1];
	data.hp    = GetHealth();
//	data.sp    = GetPower(POWER_MANA);
	data.deathState = m_deathState;
	data.movementType = GetDefaultMovementType();

	// update in DB
	WorldDatabase.BeginTransaction();
/*
	WorldDatabase.PExecuteLog("DELETE FROM `creature` WHERE guid ='%u'", m_DBTableGuid);

	std::ostringstream ss;
	ss << "INSERT INTO creature VALUES ("
		<< m_DBTableGuid << ","
		<< GetEntry() << ","
		<< GetMapId() << ","
		<< GetPositionX() << ","
		<< GetPositionY() << ","
		<< m_respawnDelay << ","
		<< respawn_cord[0] << ","
		<< respawn_cord[1] << ","
		<< GetHealth() << ","
		<< GetPower(POWER_MANA) << ","
		<< (uint32)(m_deathState) << ","
		<< GetDefaultMovementType() << ")";

	WorldDatabase.PExecuteLog( ss.str().c_str() );
*/
	WorldDatabase.CommitTransaction();
}

void Creature::SelectLevel(const CreatureInfo *cinfo)
{
}





bool Creature::CreateFromProto(uint32 guidlow, uint32 Entry, uint32 team, const CreatureData *data)
{
	Object::_Create(guidlow, HIGHGUID_UNIT);

	m_DBTableGuid = guidlow;

	SetUInt32Value(OBJECT_FIELD_ENTRY, Entry);

	CreatureInfo const *cinfo = objmgr.GetCreatureTemplate(Entry);
	if(!cinfo)
	{
		sLog.outErrorDb("Error: creature entry %u does not exist.", Entry);
		return false;
	}

	SetUInt32Value(UNIT_FIELD_DISPLAYID, cinfo->modelid);
	SetName(GetCreatureInfo()->Name);
	sLog.outDebug("CREATURE: >> Creature::CreateFromProto '%s' %u '%s'", GetName(), GetMapId(), GetCreatureInfo()->ScriptName);

	SetUInt32Value(UNIT_FIELD_HP, cinfo->hp);
	SetUInt32Value(UNIT_FIELD_SP, cinfo->sp);
	SetUInt32Value(UNIT_FIELD_HP_MAX, cinfo->hp);
	SetUInt32Value(UNIT_FIELD_SP_MAX, cinfo->sp);
	SetUInt32Value(UNIT_FIELD_INT, cinfo->stat_int);
	SetUInt32Value(UNIT_FIELD_ATK, cinfo->stat_atk);
	SetUInt32Value(UNIT_FIELD_DEF, cinfo->stat_def);
	SetUInt32Value(UNIT_FIELD_AGI, cinfo->stat_agi);


	SetUInt32Value(UNIT_FIELD_LEVEL, cinfo->level);
	SetUInt32Value(UNIT_FIELD_ELEMENT, cinfo->element);

	///- Default all creature have maxed level spells
	AddSpell(cinfo->spell1, objmgr.GetSpellTemplate(cinfo->spell1) ? objmgr.GetSpellTemplate(cinfo->spell1)->LevelMax : 0, SPELL_UNCHANGED);

	AddSpell(cinfo->spell2, objmgr.GetSpellTemplate(cinfo->spell2) ? objmgr.GetSpellTemplate(cinfo->spell2)->LevelMax : 0, SPELL_UNCHANGED);

	AddSpell(cinfo->spell3, objmgr.GetSpellTemplate(cinfo->spell3) ? objmgr.GetSpellTemplate(cinfo->spell3)->LevelMax : 0, SPELL_UNCHANGED);

	AddSpell(cinfo->spell4, objmgr.GetSpellTemplate(cinfo->spell4) ? objmgr.GetSpellTemplate(cinfo->spell4)->LevelMax : 0, SPELL_UNCHANGED);

	AddSpell(cinfo->spell5, objmgr.GetSpellTemplate(cinfo->spell5) ? objmgr.GetSpellTemplate(cinfo->spell5)->LevelMax : 0, SPELL_UNCHANGED);


	SetUInt32Value(UNIT_NPC_FLAGS, cinfo->npcflag);
	if(isVendor()) sLog.outString(" >> '%s' is Vendor", GetName()); 
	if(isTrainer()) sLog.outString(" >> '%s' is Trainer", GetName()); 
	if(isQuestGiver()) sLog.outString(" >> '%s' is QuestGiver", GetName());
	if(isGossip()) sLog.outString(" >> '%s' is Gossip", GetName());
	if(isBanker()) sLog.outString(" >> '%s' is Banker", GetName());
	if(isInnKeeper()) sLog.outString(" >> '%s' is InnKeeper", GetName());
	if(isServiceProvider()) sLog.outString(" >> '%s' is ServiceProvider", GetName());

	if( !m_puppet )
	{
		// checked at loading
		m_defaultMovementType = MovementGeneratorType(cinfo->MovementType);
	}

	// Notify the map's instance data.
	// Only works if you create the object in it, not if it is moves to that map
	// Normally non-players do not teleport to others map.
	/*
	Map *map = MapManager::Instance().GetMap(GetMapId(), this);
	if(map && map->GetInstanceData())
	{
		map->GetInstanceData()->OnCreatureCreate(this, Entry);
	}
	*/

	return true;
}

bool Creature::LoadFromDB(uint32 guid, uint32 InstanceId)
{
	CreatureData const* data = objmgr.GetCreatureData(guid);

	if(!data)
	{
		sLog.outErrorDb("CREATURE: Creature (GUID: %u) not found in table `creature`, can't load.\n", guid);
		return false;
	}

	uint32 stored_guid = guid;

//	if (InstanceId != 0) guid = objmgr.GenerateLogGuid(HIGHGUID_UNIT);
//	SetInstanceId(InstanceId);

	uint32 team = 0;
	if(!Create(guid, data->mapid, data->posX, data->posY, data->id, team, data))
		return false;

	m_DBTableGuid = stored_guid;
/*
	uint32 curhealth = data->curhealth;
	if(curhealth)
	{
		curhealth = uint32(curhealth*_GetHealthMod(GetCreatureInfo()->rank));
		if(curhealth < 1)
			curhealth = 1;
	}

	SetHealth(curhealth);
	SetPower(POWER_MANA, data->curmana);

	respawn_cord[0] = data->spawn_posx;
	respawn_cord[1] = data->spawn_posy;

	m_respawnDelay = data->spawntimesecs;
*/
	m_deathState = (DeathState)data->deathState;
	if(m_deathState == JUST_DIED)   // Don't must be set to JUST_DEAD, see Creature::setDeathState JUST_DIED -> CORPSE promoting
	{
		sLog.outErrorDb("Creature (GUIDLow: %u Entry: %u) in wrong state: JUST_DIED (1). State set to ALIVE.", GetGUIDLow(), GetEntry());
		m_deathState = ALIVE;
	}
	else
	if(m_deathState < ALIVE || m_deathState > DEAD)
	{
		sLog.outErrorDb("Creature (GUIDLow: %u Entry: %u) in wrong state: %d. State set to ALIVE.", GetGUIDLow(), GetEntry());
		m_deathState = ALIVE;
	}
/*
	m_respawnTime = objmgr.GetCreatureRespawnTime(m_DBTableGuid, GetInstanceId());
	if(m_respawnTime > time(NULL))     // not ready to respawn
		m_deathState = DEAD;
	else if(m_respawnTime)             // respawn time set but expired
	{
		m_respawnTime = 0;
		objmgr.SaveCreatureRespawnTime(m_DBTableGuid, GetInstanceId(), 0);
	}
*/
	if( !m_puppet )
	{
		// checked at creature_template loading
		m_defaultMovementType = MovementGeneratorType(data->movementType);
		AIM_Initialize();
	}

	return true;
}


void Creature::LoadGoods()
{
	// already loaded;
	if(m_itemsLoaded)
		return;

	m_vendor_items.clear();

	QueryResult *result = WorldDatabase.PQuery("SELECT slot, item FROM npc_vendor WHERE guid = '%u'", GetGUIDLow());

	if(!result) return;

	do
	{
		Field *f = result->Fetch();

		if(GetItemCount() >= MAX_VENDOR_ITEMS)
		{
			sLog.outErrorDb("Vender %u has too many items (%u >= %i). Check the DB!", GetEntry(), GetItemCount(), MAX_VENDOR_ITEMS );
			break;
		}

		uint8  slot  = f[0].GetUInt8(); 
		uint32 entry = f[1].GetUInt32();
		if(!sItemStorage.LookupEntry<ItemPrototype>(entry))
		{
			sLog.outErrorDb("Vendor '%s' have in item list non-existed item %u", GetName(), entry);
			continue;
		}

		AddItem( slot, entry );
	}
	while ( result->NextRow() );

	delete result;

	m_itemsLoaded = true;
}
/*
bool Creature::hasQuest(uint32 quest_id) const
{
	QuestRelations const& qr = objmgr.mCreatureQuestRelations;
	for(QuestRelations::const_iterator itr = qr.lower_bound(GetEntry()); itr != qr.upper_bound(GetEntry()); ++itr)
	{
		if(itr->second==quest_id)
			return true;
	}
	return false;
}

bool Creature::hasInvolvedQuest(uint32 quest_id) const
{
	QuestRelations const& qr = objmgr.mCreatureQuestInvolvedRelations;
	for(QuestRelations::const_iterator itr = qr.lower_bound(GetEntry()); itr != qr.upper_bound(GetEntry()); ++itr)
	{
		if(itr->second==quest_id)
			return true;
	}
	return false;
}
*/
void Creature::DeleteFromDB()
{
//	objmgr.SaveCreatureRespawnTime(m_DBTableGuid, GetInstanceId(), 0);
	objmgr.DeleteCreatureData(m_DBTableGuid);

	WorldDatabase.BeginTransaction();
	WorldDatabase.PExecuteLog("DELETE FROM creature WHERE guid = '%u'", m_DBTableGuid);
//	WorldDatabase.PExecuteLog("DELETE FROM creature_addon WHERE guid = '%u'", m_DBTableGuid);
//	WorldDatabase.PExecuteLog("DELETE FROM creature_movement WHERE id = '%u'", m_DBTableGuid);
	WorldDatabase.CommitTransaction();
}

CreatureInfo const *Creature::GetCreatureInfo() const
{
	const CreatureInfo * cinfo = objmgr.GetCreatureTemplate(GetEntry());
//	sLog.outString("Creature::GetCreatureInfo ENTRY('%s') NAME('%s') SCRIPTNAME('%s')", cinfo->Entry, cinfo->Name, cinfo->ScriptName);
	return cinfo;
}
/*
void Creature::setDeathState(DeathState s)
{
	if(s == JUST_DIED)
	{
		if(!IsStopped()) StopMoving();
	}
	Unit::setDeathState(s);

	if(s == JUST_DIED)
	{
		SetUInt64Value (UNIT_FIELD_TARGET, 0);
		SetUInt32Value (UNIT_NPC_FLAGS, 0);
		Unit::setDeathState(CORPSE);
	}
}

void Creature::Respawn()
{
	if(getDeathState()==CORPSE)
	{
		m_deathTimer = 0;
		Update(0);
	}
	if(getDeathState()==DEAD)
	{
		objmgr.SaveCreatureRespawnTime(m_DBTableGuid, GetInstanceId(), 0);
		m_respawnTime = time(NULL);                  // respawn at next tick
	}
}





bool Creature::IsVisibleInGridForPlayer(Player* pl) const
{
	// gamemaster in GM mode see all
	if(pl->isGameMaster())
		return true;

	// Live player
	if(pl->isAlive())
	{
		return isAlive() || m_deathTimer > 0;
	}
	return true;
}

void Creature::SaveRespawnTime()
{
	if(isPet())
		return;

	if(m_respawnTime > time(NULL))     // dead (no corpse)
		objmgr.SaveCreatureRespawnTime(m_DBTableGuid, GetInstanceId(), m_respawnTime);
	else if(m_deathTimer > 0)          // dead (corpse)
		objmgr.SaveCreatureRespawnTime(m_DBTableGuid, GetInstanceId(), time(NULL)+m_respawnDelay+m_deathTimer/1000);
}

bool Creature::IsOutOfThreatArea(Unit* pVictim) const
{
	if(!pVictim)
		return true;

	if(!pVictim->IsInMap(this))
		return true;

	if(!pVictim->isTargetableForAttack())
		return true;

	if(!pVictim->isInAccessablePlaceFor(this))
		return true;

	// we not need get instance map, base map provide all info
	if(MapManager::Instance().GetBaseMap(GetMapId())->Instanceable())
		return false;

	uint16 rx, ry;
	GetRespawnCoord(rx, ry);

	float length = pVictim->GetDistanceSq(rx, ry);
	return ( length > 100.0f );        // real value unknown
}

CreatureDataAddon const* Creature::GetCreatureAddon() const
{
	if(CreatureDataAddon const* addon = objmgr.GetCreatureAddon(m_DBTAbleGuid))
		return addon;

	return objmgr.GetCreatureTemplateAddon(GetEntry());
}

bool Creature::IsInEvadeMove() const
{
	//return !i_motionMaster.empty() && i_motionMaster.top()->GetMovementGeneratorType() == HOME_MOTION_TYPE;
	return false;
}

bool Creature::HasSpell(uint32 spellID) const
{
	uint8 i;
	for(i = 0; i < CREATURE_MAX_SPELLS; i++)
		if(spellID == m_spells[i])
			break;
	return i < CREATURE_MAX_SPELLS;     // break before end of iteration of known spells
}
*/

uint8 Creature::GetMapNpcId()
{
	uint32 guidlow = GetGUIDLow();
	uint16 mapid   = GetMapId();
	uint8  mapnpcid = guidlow - (mapid * MAP_NPCID_MULTIPLIER);

	sLog.outString(" ********* ");
	sLog.outString(" Creature::GetMapNpcId guidlow %u mapid %u mapnpcid %u", guidlow, mapid, mapnpcid);

	return mapnpcid;

}
