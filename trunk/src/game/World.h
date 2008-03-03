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

#ifndef __LEGACY_WORLD_H
#define __LEGACY_WORLD_H

#include "Common.h"
#include "Timer.h"
#include "Policies/Singleton.h"

#include <map>
#include <set>
#include <list>

using namespace std;

class Object;
class WorldPacket;
class WorldSession;
class Player;
//class Weather;
class QueryResult;
class SqlResultQueue;
class QueryResult;
class WorldSocket;

enum WorldTimers
{
	WUPDATE_OBJECTS = 0,
	WUPDATE_SESSIONS = 1,
	WUPDATE_COUNT = 2
};

/// Configuration elements
enum WorldConfigs
{
	CONFIG_COMPRESSION = 0,
	CONFIG_GRID_UNLOAD,
	CONFIG_INTERVAL_SAVE,
	CONFIG_LOG_LEVEL,
	CONFIG_LOG_WORLD,
	CONFIG_PORT_WORLD,
	CONFIG_SOCKET_SELECTTIME,
	CONFIG_VALUE_COUNT,
	CONFIG_INTERVAL_MAPUPDATE
};

enum Rates
{
	RATE_HEALTH = 0
};

enum ServerMessageType
{
	SERVER_MSG_SHUTDOWN_TIME = 1
};

class World
{
	public:
		static volatile bool m_stopEvent;

		World();
		~World();

		WorldSession* FindSession(uint32 id) const;
		void AddSession(WorldSession *s);
		void RemoveSession(uint32 id);
		/// Get the number of current active sessions
		uint32 GetActiveAndQueuedSessionCount() const { return m_sessions.size(); }
		uint32 GetActiveSessionCount() const { return m_sessions.size() - m_QueuedPlayer.size(); }

		void Update(time_t diff);

		void setConfig(uint32 index, uint32 value)
		{
			if(index < CONFIG_VALUE_COUNT)
				m_configs[index] = value;
		}

		uint32 getConfig(uint32 index) const
		{
			if(index<CONFIG_VALUE_COUNT)
				return m_configs[index];
			else
				return 0;
		}

		// Player Queue
		typedef std::list<WorldSocket*> Queue;
		void AddQueuedPlayer(WorldSocket* Socket);
		void RemoveQueuedPlayer(WorldSocket* Socket);
		uint32 GetQueuePos(WorldSocket* Socket);
		uint32 GetQueueSize() const { return m_QueuedPlayer.size(); }

		void SetInitialWorldSettings();

		void UpdateResultQueue();
		void InitResultQueue();

	private:
		IntervalTimer m_timers[WUPDATE_COUNT];

		typedef HM_NAMESPACE::hash_map<uint32, WorldSession*> SessionMap;
		SessionMap m_sessions;

		uint32 m_configs[CONFIG_VALUE_COUNT];
		uint32 m_playerLimit;

		SqlResultQueue *m_resultQueue;

		// Player Queue
		Queue m_QueuedPlayer;
};

extern uint32 realmID;

#define sWorld LeGACY::Singleton<World>::Instance()
#endif
