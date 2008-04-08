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
#include "Config/ConfigEnv.h"
#include "SystemConfig.h"
#include "Log.h"
#include "Opcodes.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "Player.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Chat.h"
#include "MapManager.h"
#include "ScriptCalls.h"
#include "CreatureAIRegistry.h"
#include "Policies/SingletonImp.h"
#include "Database/DatabaseImpl.h"
#include "WorldSocket.h"
#include "RedZoneDistrict.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"

#include "SystemConfig.h"

#include <signal.h>

INSTANTIATE_SINGLETON_1( World );

volatile bool World::m_stopEvent = false;

World::World()
{
	m_resultQueue = NULL;
}

World::~World()
{
	if(m_resultQueue) delete m_resultQueue;
}

void World::SetInitialWorldSettings()
{
	///- Initialize the random number generator
	srand((unsigned int)time(NULL));

	///- Read the version of the configuration file and warn the user in cans of emptiness or mismatch
	uint32 confVersion = sConfig.GetIntDefault("ConfVersion", 0);
	if(!confVersion)
	{
		sLog.outError("******************************************************");
		sLog.outError(" WARNING: legacyd.conf doest not include a ConfVersion");
		sLog.outError("          Your configuration file may be out of date!");
		sLog.outError("******************************************************");
		clock_t pause = 3000 + clock();
		while(pause > clock());
	}
	else
	{
		if( confVersion < _LEGACYDCONFVERSION )
		{
		sLog.outError("******************************************************");
		sLog.outError(" WARNING: legacyd.conf version is out of date!");
		sLog.outError("          Please check for updates");
		sLog.outError("******************************************************");
		clock_t pause = 3000 + clock();
		while(pause > clock());
		}
	}

	///- Read the player limit and the Message of the day from the config file
	SetPlayerLimit(sConfig.GetIntDefault("PlayerLimit", DEFAULT_PLAYER_LIMIT), true);
	SetMotd(sConfig.GetStringDefault("Motd", "Welcome to the Massive Network Game Object Server.").c_str() );

	///- Read all rates from the config file
	rate_values[RATE_DROP_ITEMS] = sConfig.GetFloatDefault("Rate.Drop.Items", 1);
	rate_values[RATE_XP_KILL]    = sConfig.GetFloatDefault("Rate.XP.Kill", 1);
	rate_values[RATE_XP_QUEST]   = sConfig.GetFloatDefault("Rate.XP.Quest", 1);
	rate_values[RATE_HONOR_GAIN] = sConfig.GetFloatDefault("Rate.Honor.Gain", 1);
	rate_values[RATE_ITEM_CREATION_QUALITY] = sConfig.GetFloatDefault("Rate.Item.Creation.Quality", 1);
	rate_values[RATE_CREATURE_NORMAL_DAMAGE] = sConfig.GetFloatDefault("Rate.Creature.Normal.Damage", 1);
	rate_values[RATE_CREATURE_SPELL_DAMAGE] = sConfig.GetFloatDefault("Rate.Creature.Spell.Damage", 1);
	rate_values[RATE_CREATURE_DIFFICULTY] = sConfig.GetFloatDefault("Rate.Creature.Difficulty", 1);

	rate_values[RATE_LINKED_AGILITY] = sConfig.GetIntDefault("Rate.Linked.Agility", 5);


	///- Read other configuration items from the config file
	m_configs[CONFIG_INTERVAL_SAVE] = sConfig.GetIntDefault("PlayerSaveInterval", 900000);
	m_configs[CONFIG_PORT_WORLD] = sConfig.GetIntDefault("WorldServerPort", DEFAULT_WORLDSERVER_PORT);
	m_configs[CONFIG_SOCKET_SELECTTIME] = sConfig.GetIntDefault("SocketSelectTime", DEFAULT_SOCKET_SELECT_TIME);

	m_configs[CONFIG_GRID_UNLOAD] = sConfig.GetBoolDefault("GridUnload", true);




	sLog.outString( "Loading Items..." );
	objmgr.LoadItemPrototypes();

	sLog.outString( "Loading Spells..." );
	objmgr.LoadSpellPrototypes();

	sLog.outString( "Loading Creature templates..." );
	objmgr.LoadCreatureTemplates();




	sLog.outString( "Loading Creature Data...");
	objmgr.LoadCreatures();




	///- Load and initialized scripts
	sLog.outString( "Loading Scripts..." );
	objmgr.LoadQuestStartScripts();   // must be after load Creature/Gameobject(Template/Data) and QuestTemplate
	objmgr.LoadQuestEndScripts();     // must be after load Creature/Gameobject(Template/Data) and QuestTemplate
	objmgr.LoadSpellScripts();
	objmgr.LoadGameObjectScripts();
	objmgr.LoadEventScripts();



	sLog.outString( "Initializing Scripts..." );
	if(!LoadScriptingModule())
		exit(1);

	RefreshDoorDatabase();

	m_timers[WUPDATE_OBJECTS].SetInterval(0);
	m_timers[WUPDATE_SESSIONS].SetInterval(0);

	///- Initialize static helper structures
	AIRegistry::Initialize();

	///- Initialize MapManager
	sLog.outString( "Starting Map System" );
	MapManager::Instance().Initialize();





	sLog.outString( "WORLD: World initialized" );
}

void World::RefreshDoorDatabase()
{
	sLog.outString("Loading Map Matrix...");
	MapManager::Instance().ClearDoorDatabase();
	QueryResult* resultMap2Map = WorldDatabase.PQuery("SELECT * FROM map_matrix");
	//where x != 0 and y != 0");
	do
	{
		Field* f = resultMap2Map->Fetch();
		uint16 srcMap  = f[1].GetUInt16();
		uint16 door    = f[2].GetUInt16();
		uint16 destMap = f[3].GetUInt16();
		uint16 destX   = f[4].GetUInt16();
		uint16 destY   = f[5].GetUInt16();
		MapDoor* mapDoor = new MapDoor(srcMap, door);
		MapDestination* mapDest = new MapDestination(destMap, destX, destY);
		MapManager::Instance().AddMapMatrix(mapDoor, mapDest);
		//MapManager::Instance().AddMap2Door(mapDoor, destMap);
	//	delete mapDoor;
	} while( resultMap2Map->NextRow() );
	sLog.outString(">> Loaded %u Map Matrix definitions", MapManager::Instance().GetMapMatrixCount());
	sLog.outString("");
}

void World::InitResultQueue()
{
	m_resultQueue = new SqlResultQueue;
	CharacterDatabase.SetResultQueue(m_resultQueue);
}

void World::UpdateResultQueue()
{
	m_resultQueue->Update();
}


void World::Update(time_t diff)
{
	for(int i = 0; i < WUPDATE_COUNT; i++)
		if(m_timers[i].GetCurrent()>=0)
			m_timers[i].Update(diff);
		else m_timers[i].SetCurrent(0);

	_UpdateGameTime();

	if(m_timers[WUPDATE_SESSIONS].Passed())
	{
		m_timers[WUPDATE_SESSIONS].Reset();

		SessionMap::iterator itr, next;
		for(itr = m_sessions.begin(); itr != m_sessions.end(); itr = next)
		{
			next = itr;
			next++;
			
			if(!itr->second->Update(diff))
			{
				delete itr->second;
				m_sessions.erase(itr);
			}
		}
	}

	/// <li> Handle all other objects
	if (m_timers[WUPDATE_OBJECTS].Passed())
	{
		m_timers[WUPDATE_OBJECTS].Reset();
		///- Update object when the timer has passed (maps, ...)
		MapManager::Instance().Update(diff);    // As interval = 0
	}

	// execute callbacks from sql queries that were queued recently
	UpdateResultQueue();

	///- Move all creatures with "delayed move" and remove and delete all object with "delayed remove"
	ObjectAccessor::Instance().DoDelayedMovesAndRemoves();
}

WorldSession * World::FindSession(uint32 id) const
{
	SessionMap::const_iterator itr = m_sessions.find(id);

	if(itr != m_sessions.end())
		return itr->second;
	else
		return 0;
}

bool World::RemoveSession(uint32 id)
{
	SessionMap::iterator itr = m_sessions.find(id);

	if(itr != m_sessions.end())
	{
		delete itr->second;
		m_sessions.erase(itr);
	}
}

uint32 World::GetQueuePos(WorldSocket* socket)
{
	uint32 position = 1;

	for(Queue::iterator iter = m_QueuedPlayer.begin(); iter != m_QueuedPlayer.end(); ++iter, ++position)
		if((*iter) == socket)
			return position;

	return 0;
}

/// Add a session to the session list
void World::AddSession(WorldSession* s)
{
	ASSERT(s);
	m_sessions[s->GetAccountId()] = s;
}

void World::AddQueuedPlayer(WorldSocket* socket)
{
	m_QueuedPlayer.push_back(socket);
}

void World::RemoveQueuedPlayer(WorldSocket* socket)
{
	// sessions count including queued to remove (if removed_session set)
	uint32 sessions = GetActiveSessionCount();

	uint32 position = 1;
	Queue::iterator iter = m_QueuedPlayer.begin();

	// if session not queued then we need decrease sessions count (Remove socked callet before session removing from session list)
	bool decrease_session = true;

	// search socket to remove and count skipped positions
	for(;iter != m_QueuedPlayer.end(); ++iter, ++position)
	{
		if(*iter==socket)
		{
			Queue::iterator iter2 = iter;
			++iter;
			m_QueuedPlayer.erase(iter2);
			decrease_session = false;    // removing queued session
			break;
		}
	}

	// iter point to next socked after removed or end()
	// position store position of removed socket and the new position next socket after removed
	
	// decrease for case session queued for removing
	if(decrease_session && sessions)
		--sessions;

	// accept first in queue
	if( (!m_playerLimit || sessions < m_playerLimit) && !m_QueuedPlayer.empty() )
	{
		WorldSocket * socket = m_QueuedPlayer.front();
		socket->SendAuthWaitQue(0);
		m_QueuedPlayer.pop_front();

		// update iter to point first queued socket or end() if queue is empty now
		iter = m_QueuedPlayer.begin();
		position = 1;
	}

	// update position from iter to end()
	// iter point to first not updated socket, position store new position
	for(; iter != m_QueuedPlayer.end(); ++iter, ++position)
		(*iter)->SendAuthWaitQue(position);
}

/// Update the game time
void World::_UpdateGameTime()
{
	///- update the time
	time_t thisTime = time(NULL);
	uint32 elapsed = uint32(thisTime - m_gameTime);
	m_gameTime = thisTime;

	///- Handle shutdown timer here
}

void World::SetPlayerLimit(int32 limit, bool needUpdate)
{
	if(limit < -SEC_ADMINISTRATOR)
		limit = -SEC_ADMINISTRATOR;

	// lock update need
	bool db_update_need = needUpdate || (limit < 0) != (m_playerLimit < 0) || (limit < 0 && m_playerLimit < 0 && limit != m_playerLimit);

	m_playerLimit = limit;

	//if(db_update_need)
	//	loginDatabase.PExecute("UPDATE realmlist SET allowedSecurityLevel = '%u' WHERE id = '%d'", uint8(GetPlayerSecurityLimit()),realmID);

}
