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
#include "GossipDef.h"
#include "Player.h"
#include "Opcodes.h"
#include "Log.h"

#include "MapManager.h"
#include "CreatureAI.h"
//#include "CreatureAISelector.h"


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
m_defaultMovementType( IDLE_MOTION_TYPE )
{
	m_valuesCount = UNIT_END;

	for(int i = 0; i < 2; ++i) respawn_cord[i] = 0;
	//for(int i = 0; i < 4; ++i) m_spells[i] = 0;
	sLog.outString("Creature Constructor");
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
	Unit::Update( diff );
	/*
	switch( m_deathState )
	{
		case JUST_DIED:
			// Dont must be called, see Creature::setDeathState JUST_DIED -> CORPSE promoting.
			sLog.outError("Creature (GUIDLow: %u Entry: %u ) in wrong state: JUST_DEAD (1)", GetGUIDLow(), GetEntry());
			break;

		case DEAD:
		{
			if( m_respawnTime <= time(NULL) )
			{
				DEBUG_LOG("Respawning...");
				m_respawnTime = 0;

				CreatureInfo const *cinfo = objmgr.GetCreatureTemplate(this->GetEntry());

				SetHealth(GetMaxHealth());
				setDeathState( ALIVE );
				clearUnitState(UNIT_STAT_ALL_STATE);
				i_motionMaster.Clear();
				MapManager::Instance().GetMap(GetMapId(), this)->Add(this);
			}
		}
		case ALIVE:
		{
			Unit::Update( diff );

			// creature can be dead after Unit::Update call
			if(!isAlive())
				break;

			// do not allow the AI to be changed during update
			m_AI_locked = true;
			i_AI->UpdateAI(diff);
			m_AI_locked = false;
		}
		default:
			break;
	}
	*/
}

/*
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
*/

bool Creature::Create(uint32 guidlow, uint16 mapid, uint16 x, uint16 y, uint32 Entry, uint32 team, const CreatureData *data)
{
	respawn_cord[0] = x;
	respawn_cord[1] = y;
	SetMapId(mapid);
	Relocate(x, y);

	sLog.outString("Creature::Create GUID(%u) MAPID(%u) ENTRY(%u)", guidlow, GetMapId(), Entry);

	return CreateFromProto(guidlow, Entry, team, data);
}

void Creature::prepareGossipMenu(Player *pPlayer, uint32 gossidid)
{

}

void Creature::sendPreparedGossip(Player* player)
{
	if(!player)
		return;

	uint8 mapNpcId = (GetGUIDLow() / 100) - GetMapId();
	player->PlayerTalkClass->SendGossipMenu( GetNpcTextId(), mapNpcId );
}
/*
void Creature::OnGossipSelect(Player* player, uint32 option)
{
	GossipMenu* gossipmenu = player->PlayerTalkClass->GetGossipMenu();
	uint32 action=gossipmenu->GetItem(option).m_gAction;
	uint32 zoneid=GetZoneId();
	uint64 guid=GetGUID();
	GossipOption const *gossip=GetGossipOption( action );
	uint32 textid;
	if(!gossip)
	{
		zoneid=0;
		gossip=GetGossipOption( action );
		if(!gossip)
			return;
	}
	textid=GetGossipTextId( action, zoneid );
	if(textid==0)
		textid=GetNpcTextId();

	switch (gossip->Action)
	{
		case GOSSIP_OPTION_GOSSIP:
			player->PlayerTalkClass->CloseGossip();
			player->PlayerTalkClass->SendTalking( textid );
			break;

		case GOSSIP_OPTION_QUESTGIVER:
			player->PrepareQuestMenu( guid );
			player->SendPreparedQuest( guid );
			break;

		case GOSSIP_OPTION_VENDOR:
		case GOSSIP_OPTION_ARMORER:
			player->GetSession()->SendListInventory(guid);
			break;

		case GOSSIP_OPTION_INNKEEPER:
			player->PlayerTalkClass->CloseGossip();
			player->SetBindPoint( guid );
			break;
		
		case GOSSIP_OPTION_BANKER:
			player->GetSession()->SendShowBank( guid );
			break;

		default:
			OnPoiSelect( player, gossip );
			break;
	}

}

void Creature::OnPoiSelect(Player* player, GossipOption const *gossip)
{
}
*/
uint32 Creature::GetGossipTextId(uint32 action, uint32 zoneid)
{
	QueryResult *result = WorldDatabase.PQuery("SELECT textid FROM npc_gossip_textid WHERE action = '%u' AND zoneid = '%u'", action, zoneid );

	if(!result)
		return 0;

	Field *f = result->Fetch();
	uint32 id = f[0].GetUInt32();

	delete result;

	return id;
}

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

uint32 Creature::GetNpcTextId()
{
	sLog.outString("Creature::GetNpcTextId GUID(%u) TextId(%u)", GetGUIDLow(), m_NPCTextId );
	// already loaded and cached
	if(m_NPCTextId)
		return m_NPCTextId;

	QueryResult* result = WorldDatabase.PQuery("SELECT textid FROM npc_gossip WHERE npc_guid = '%u'", GetGUIDLow());
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
	data.sp    = GetPower(POWER_MANA);
	data.deathState = m_deathState;
	data.movementType = GetDefaultMovementType();

	// update in DB
	WorldDatabase.BeginTransaction();

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

	WorldDatabase.CommitTransaction();
}
*/
void Creature::SelectLevel(const CreatureInfo *cinfo)
{
}





bool Creature::CreateFromProto(uint32 guidlow, uint32 Entry, uint32 team, const CreatureData *data)
{
	Object::_Create(guidlow, HIGHGUID_UNIT);

	m_DBTableGuid = guidlow;

	SetUInt32Value(OBJECT_FIELD_ENTRY, Entry);

	sLog.outString("Creature FIELD_ENTRY %u", GetEntry());

	sLog.outString("Creature::CreateFromProto ENTRY(%u) MAPID(%u)", Entry, GetMapId());
	CreatureInfo const *cinfo = objmgr.GetCreatureTemplate(Entry);
	if(!cinfo)
	{
		sLog.outErrorDb("Error: creature entry %u does not exist.", Entry);
		return false;
	}

	SetName(GetCreatureInfo()->Name);
	sLog.outString("Creature::CreateFromProto NAME('%s') MAPID(%u) SCRIPTNAME('%s')", GetName(), GetMapId(), GetCreatureInfo()->ScriptName);

	//SelectLevel(cinfo);

	//SetUInt32Value(UNIT_NPC_FLAGS, cinfo->npcflag);

	//SetUInt32Value(UNIT_FIELD_FLAGS, cinfo->Flags);
	//SetUInt32Value(UNIT_DYNAMIC_FLAGS, cinfo->dynamicflags);

	//SetCanModifyStats(true);
	//UpdateAllStats();
/*
	m_spells[0] = cinfo->spell1;
	m_spells[1] = cinfo->spell2;
	m_spells[2] = cinfo->spell3;
	m_spells[3] = cinfo->spell4;
*/
	// checked at loading
	//m_defaultMovementType = MovementGeneratorType(cinfo->MovementType);

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
		sLog.outErrorDb("Creature (GUID: %u) not found in table `creature`, can't load.", guid);
		return false;
	}

	uint32 stored_guid = guid;

//	if (InstanceId != 0) guid = objmgr.GenerateLogGuid(HIGHGUID_UNIT);
//	SetInstanceId(InstanceId);

	sLog.outString("Creature::LoadFromDB CreatureData MAPID(%u) ENTRY(%u)", data->mapid, data->id);
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

	m_respawnTime = objmgr.GetCreatureRespawnTime(m_DBTableGuid, GetInstanceId());
	if(m_respawnTime > time(NULL))     // not ready to respawn
		m_deathState = DEAD;
	else if(m_respawnTime)             // respawn time set but expired
	{
		m_respawnTime = 0;
		objmgr.SaveCreatureRespawnTime(m_DBTableGuid, GetInstanceId(), 0);
	}

	// checked at creature_template loading
	m_defeaultMovementType = MovementGeneratorType(data->movementType);

	AIM_Initialize();
	*/
	return true;
}


void Creature::LoadGoods()
{
	/*
	// already loaded;
	if(m_itemsLoaded)
		return;

	m_vendor_items.clear();

	QueryResult *result = WorldDatabase.PQuery("SELECT item, maxcount, incrtime FROM npc_vendor WHERE entry = '%u'", GetEntry());

	if(!result) return;

	do
	{
		Field *f = result->Fetch();

		if(GetItemCount() >= MAX_VENDOR_ITEMS)
		{
			sLog.outErrorDb("Vender %u has too many items (%u >= %i). Check the DB!", GetEntry(), GetItemCount(), MAX_VENDOR_ITEMS );
			break;
		}

		uint32 item_id = f[0].GetUInt32();
		if(!sItemStorage.LookupEntry<ItemPrototype>(item_id))
		{
			sLog.outErrorDb("Vendor %u have in item list non-existed item %u", GetEntry(), item_id);
			continue;
		}

		AddItem( item_id, f[1].GetUInt32(), f[2].GetUInt32() );
	}
	while ( result->NextRow() );

	delete result;

	m_itemLoaded = true;
	*/
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

void Creature::DeleteFromDB()
{
	objmgr.SaveCreatureRespawnTime(m_DBTableGuid, GetInstanceId(), 0);
	objmgr.DeleteCreatureData(m_DBTableGuid);

	WorldDatabase.BeginTransaction();
	WorldDatabase.PExecuteLog("DELETE FROM creature WHERE guid = '%u'", m_DBTableGuid);
	WorldDatabase.PExecuteLog("DELETE FROM creature_addon WHERE guid = '%u'", m_DBTableGuid);
	WorldDatabase.PExecuteLog("DELETE FROM creature_movement WHERE id = '%u'", m_DBTableGuid);
	WorldDatabase.CommitTransaction();
}
*/

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
