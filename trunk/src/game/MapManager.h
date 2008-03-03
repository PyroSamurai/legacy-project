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

		typedef LeGACY::ClassLevelLockable<MapManager, ZThread::Mutex>::Lock Guard;

		uint32 i_gridCleanUpDelay;
		MapMapType i_maps;
		IntervalTimer i_timer;

};
#endif
