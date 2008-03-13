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

#ifndef __LEGACY_MAP_H
#define __LEGACY_MAP_H

#include "Platform/Define.h"
#include "Policies/ThreadingModel.h"
#include "zthread/Lockable.h"
#include "zthread/Mutex.h"
#include "zthread/FairReadWriteLock.h"
#include "GridDefines.h"
#include "Cell.h"
#include "Object.h"
#include "Timer.h"
#include "SharedDefines.h"
#include "GameSystem/GridRefManager.h"

#include <bitset>
#include <list>

class Unit;
class WorldPacket;

namespace ZThread
{
	class Lockable;
	class ReadWriteLock;
}

typedef ZThread::FairReadWriteLock GridRWLock;

template<class MUTEX, class LOCK_TYPE>
struct RGuard
{
	RGuard(MUTEX &l) : i_lock(l.getReadLock()) {}
	LeGACY::GeneralLock<LOCK_TYPE> i_lock;
};

template<class MUTEX, class LOCK_TYPE>
struct WGuard
{
	WGuard(MUTEX &l) : i_lock(l.getWriteLock()) {}
	LeGACY::GeneralLock<LOCK_TYPE> i_lock;
};

typedef RGuard<GridRWLock, ZThread::Lockable> GridReadGuard;
typedef WGuard<GridRWLock, ZThread::Lockable> GridWriteGuard;
typedef LeGACY::SingleThreaded<GridRWLock>::Lock NullGuard;


class LEGACY_DLL_DECL Map : public GridRefManager<NGridType>, public LeGACY::ObjectLevelLockable<Map, ZThread::Mutex>
{
	public:
		typedef std::list<Player*> PlayerList;

		Map(uint16 id, time_t);
		virtual ~Map();

		void Add(Player *);
		void Remove(Player *, bool);
		template<class T> void Add(T *);
		template<class T> void Remove(T *, bool);

		virtual void Update(const uint32&);

		void MessageBroadcast(Player *, WorldPacket *, bool to_self, bool own_team_only = false);

		void MessageBroadcast(WorldObject *, WorldPacket *);

		template<class LOCK_TYPE, class T, class CONTAINER> void Visit(const CellLock<LOCK_TYPE> &cell, TypeContainerVisitor<T, CONTAINER> &visitor);

		void UnloadAll(bool pForce);

		void PlayerRelocation(Player *, uint16 x, uint16 y);

		uint16 GetId(void) const { return i_id; }

		static void InitStateMachine();
		static void DeleteStateMachine();


		uint32 GetPlayersCount() const { return i_Players.size(); }
		void Reset();
		bool CanEnter(Player* player) const;
		const char* GetMapName() const;

		void LoadMap(uint16, int x, int y);

		void SendInitSelf( Player * player );

		bool loaded(const GridPair &) const;
		void EnsureGridLoadedForPlayer(const Cell&, Player*, bool add_player);
		void EnsureGridCreated(const GridPair &);

		void buildNGridLinkage(NGridType* pNGridType) { pNGridType->link(this); }

		template<class T> void AddType(T *obj);
		template<class T> void RemoveType(T *obj, bool);

		NGridType* getNGrid(uint32 x, uint32 y) const
		{
			return i_grids[x][y];
		}

		bool isGridObjectDataLoaded(uint32 x, uint32 y) const { return getNGrid(x,y)->isGridObjectDataLoaded(); }
		void setGridObjectDataLoaded(bool pLoaded, uint32 x, uint32 y) { getNGrid(x,y)->setGridObjectDataLoaded(pLoaded); }

		void ResetGridExpiry(NGridType &grid, float factor = 1) const
		{
			grid.ResetTimeTracker((time_t)((float)i_gridExpiry*factor));
		}

		inline void setNGrid(NGridType* grid, uint32 x, uint32 y)
		{
			if(x >= MAX_NUMBER_OF_GRIDS || y >= MAX_NUMBER_OF_GRIDS)
			{
				sLog.outError("map::setNGrid() Invalid grid coordinates found: %d, %d!",x,y);
				assert(false);
			}
			i_grids[x][y] = grid;
		}

		void InitResetTime();

		void UpdateObjectVisibility(WorldObject* obj, Cell cell, CellPair cellpair);
		void UpdatePlayerVisibility(Player* player, Cell cell, CellPair cellpair);
		void UpdateObjectsVisibilityFor(Player* player, Cell cell, CellPair cellpair);


		void resetMarkedCells() { marked_cells.reset(); }
		bool isCellMarked(uint32 pCellId) { return marked_cells.test(pCellId); }
		void markCell(uint32 pCellId) { marked_cells.set(pCellId); }

	protected:
		typedef LeGACY::ObjectLevelLockable<Map, ZThread::Mutex>::Lock Guard;

	private:
		typedef GridReadGuard ReadGuard;
		typedef GridWriteGuard WriteGuard;

		uint16 i_id;
		NGridType* i_grids[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
		//GridMap *GridMaps[MAX_NUMBER_OF_GRIDS][MAX_NUMBER_OF_GRIDS];
		std::bitset<TOTAL_NUMBER_OF_CELLS_PER_MAP*TOTAL_NUMBER_OF_CELLS_PER_MAP> marked_cells;

		time_t i_gridExpiry;

//		MapEntry const* i_mapEntry;
		time_t i_resetTime;
		uint32 i_resetDelayTime;
		uint32 i_maxPlayers;


		PlayerList i_Players;

		// Type specific code for add/remove to/from grid
		template<class T>
			void AddToGrid(T*, NGridType *, Cell const&);

		template<class T>
			void AddNotifier(T*, Cell const&, CellPair const&);

		template<class T>
			void RemoveFromGrid(T*, NGridType *, Cell const&);

		template<class T>
			void DeleteFromWorld(T*);
};

template<class LOCK_TYPE, class T, class CONTAINER>
inline void
Map::Visit(const CellLock<LOCK_TYPE> &cell, TypeContainerVisitor<T, CONTAINER> &visitor)
{
	const uint32 x = cell->GridX();
	const uint32 y = cell->GridY();
	const uint32 cell_x = cell->CellX();
	const uint32 cell_y = cell->CellY();

	if( !cell->NoCreate() || loaded(GridPair(x,y)) )
	{
		EnsureGridLoadedForPlayer(cell, NULL, false);
		getNGrid(x, y)->Visit(cell_x, cell_y, visitor);
	}
}

#endif
