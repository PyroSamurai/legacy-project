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

#ifndef __LEGACY_MAPMANAGER_H
#define __LEGACY_MAPMANAGER_H

#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "zthread/Mutex.h"
#include "Common.h"
#include "Map.h"
#include "GridStates.h"

class MapDoor
{
	public:
		MapDoor(uint16 mapid, uint16 doorid) : MapId(mapid), DoorId(doorid) {}
		uint16 MapId;
		uint8  DoorId;
};

class MapDestination
{
	public:
		MapDestination(uint16 id, uint16 x, uint16 y) :
			MapId(id), DestX(x), DestY(y) {}

		uint16 MapId;
		uint16 DestX;
		uint16 DestY;
};



class LEGACY_DLL_DECL MapManager : public LeGACY::Singleton<MapManager, LeGACY::ClassLevelLockable<MapManager, ZThread::Mutex> >
{
	friend class LeGACY::OperatorNew<MapManager>;
	typedef HM_NAMESPACE::hash_map<uint32, Map*> MapMapType;
	typedef std::pair<HM_NAMESPACE::hash_map<uint32, Map*>::iterator, bool> MapMapPair;

	public:


		Map* GetMap(uint32, const WorldObject* obj);

		// only const version for outer users
		Map const* GetBaseMap(uint32 id) const { return const_cast<MapManager*>(this)->_GetBaseMap(id); }
		void Initialize(void);
		void Update(time_t);

		bool CanPlayerEnter(uint32 mapid, Player* player);

		void AddMap2Dest(MapDoor* mapDoor, MapDestination* mapDest);
		MapDestination* FindMap2Dest(MapDoor* mapDoor);
		void ClearDoorDatabase()
		{
			HM_NAMESPACE::hash_map<MapDoor*, MapDestination*>::iterator itr;
			for(itr = m_map2Dest.begin(); itr != m_map2Dest.end(); ++itr)
			{
				delete itr->second;
				delete itr->first;
			}
		}

	private:
		void checkAndCorrectGridStatesArray();
		GridState* i_GridStates[MAX_GRID_STATE];
		int i_GridStateErrorCount;

	private:
		MapManager();
		~MapManager();

		MapManager(const MapManager &);
		MapManager& operator=(const MapManager &);

		Map* _GetBaseMap(uint32 id);
		Map* _findMap(uint32 id) const
		{
			MapMapType::const_iterator iter = i_maps.find(id);
			return (iter == i_maps.end() ? NULL : iter->second);
		}

		typedef HM_NAMESPACE::hash_map<MapDoor*, MapDestination*> Map2Dest;
		Map2Dest m_map2Dest;


		MapDestination* _findMap2Dest(MapDoor* mapDoor)
		{
			HM_NAMESPACE::hash_map<MapDoor*, MapDestination*>::iterator itr;
			for(itr = m_map2Dest.begin(); itr != m_map2Dest.end(); ++itr)
			{
				MapDoor *map = itr->first;
				if ((map->MapId == mapDoor->MapId) &&
					(map->DoorId == mapDoor->DoorId))
				{
					return itr->second;
				}
			}

			return NULL;
		}


		typedef LeGACY::ClassLevelLockable<MapManager, ZThread::Mutex>::Lock Guard;

		uint32 i_gridCleanUpDelay;
		MapMapType i_maps;
		IntervalTimer i_timer;

};
#endif
