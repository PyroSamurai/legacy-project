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

#include "MapManager.h"
#include "Policies/SingletonImp.h"
#include "Database/DatabaseEnv.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "GridDefines.h"
#include "World.h"

#define CLASS_LOCK LeGACY::ClassLevelLockable<MapManager, ZThread::Mutex>
INSTANTIATE_SINGLETON_2(MapManager, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(MapManager, ZThread::Mutex);

extern GridState* si_GridStates[]; // debugging code

MapManager::MapManager()
{
	i_timer.SetInterval(sWorld.getConfig(CONFIG_INTERVAL_MAPUPDATE));
}

MapManager::~MapManager()
{
	for(MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
		delete iter->second;

	Map::DeleteStateMachine();
	ClearDoorDatabase();
}

void
MapManager::Initialize()
{
	Map::InitStateMachine();
}

Map*
MapManager::_GetBaseMap(uint32 id)
{
	//sLog.outString("_GetBaseMap '%u'", id);
	Map *m = _findMap(id);

	if( m == NULL )
	{
//		sLog.outString("_GetBaseMap Create new Map '%u'", id);
		Guard guard(*this);

		m = new Map(id, i_gridCleanUpDelay);

		i_maps[id] = m;
	}

	assert(m != NULL);
	return m;
}

Map* MapManager::GetMap(uint32 id, const WorldObject* obj)
{
	Map *m = _GetBaseMap(id);

	return m;
}

bool MapManager::CanPlayerEnter(uint32 mapid, Player* player)
{
	return GetBaseMap(mapid)->CanEnter(player);
}

void
MapManager::Update(time_t diff)
{
	i_timer.Update(diff);
	if( !i_timer.Passed() )
		return;

	int i = 0;
	for(MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
	{
		//sLog.outString("MapManager::Update %u", i);
		//checkAndCorrectGridStatesArray(); // debugging code
		iter->second->Update(i_timer.GetCurrent());
	}
	ObjectAccessor::Instance().Update(i_timer.GetCurrent());

	i_timer.SetCurrent(0);
}

void MapManager::MoveAllCreaturesInMoveList()
{
	for(MapMapType::iterator iter=i_maps.begin(); iter != i_maps.end(); ++iter)
		iter->second->MoveAllCreaturesInMoveList();
}

// debugging code
void MapManager::checkAndCorrectGridStatesArray()
{
	bool ok = true;
	for(int i=0;i<MAX_GRID_STATE; i++)
	{
		if(i_GridStates[i] != si_GridStates[i])
		{
			sLog.outError("MapManager::checkGridStates(), GridState: si_GridStates is corrupt !!!");
			ok = false;
			si_GridStates[i] = i_GridStates[i];
		}
	}
	if(!ok)
		i_GridStateErrorCount++;
	if(i_GridStateErrorCount > 2)
		assert(false); // force a crash. Too many errors
}

void MapManager::AddMapMatrix(MapDoor* mapDoor, MapDestination* mapDest)
{
	m_mapMatrix[mapDoor] = mapDest;
}

MapDestination* MapManager::FindMapMatrix(MapDoor* mapDoor)
{
	MapDestination* dest = _findMapMatrix(mapDoor);
	if( !dest )
	{
		DEBUG_LOG("Map %u Door %u not found", mapDoor->MapId, mapDoor->DoorId);
		return NULL;
	}
	DEBUG_LOG("Map %u Door %u destination is %u, x:%u, y:%u", mapDoor->MapId, mapDoor->DoorId, dest->MapId, dest->DestX, dest->DestY);

	return dest;
}
