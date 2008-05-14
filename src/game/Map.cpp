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

#include "Player.h"
#include "GridNotifiers.h"
#include "WorldSession.h"
#include "Log.h"
#include "GridStates.h"
#include "CellImpl.h"
#include "Map.h"
#include "GridNotifiersImpl.h"
#include "Config/ConfigEnv.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "World.h"

#include "MapManager.h"

#define DEFAULT_GRID_EXPIRY 300
#define MAX_GRID_LOAD_TIME  50

GridState* si_GridStates[MAX_GRID_STATE];

Map::~Map()
{
	UnloadAll(true);
}

void Map::LoadMap(uint16 mapid, int x, int y)
{
}

void Map::InitStateMachine()
{
}

void Map::DeleteStateMachine()
{
}

Map::Map(uint16 id, time_t expiry)
: i_id(id), i_gridExpiry(expiry),
  i_resetTime(0), i_resetDelayTime(0), i_maxPlayers(0)
{
	for(unsigned int idx=0; idx < MAX_NUMBER_OF_GRIDS; ++idx)
	{
		for(unsigned int j=0; j < MAX_NUMBER_OF_GRIDS; ++j)
		{
			setNGrid(NULL, idx, j);
		}
	}

	i_Players.clear();
}

// Template specialization of utility methods
template<class T>
void Map::AddToGrid(T* obj, NGridType *grid, Cell const& cell)
{
	(*grid)(cell.CellX(), cell.CellY()).template AddGridObject<T>(obj, obj->GetGUID());
}

template<>
void Map::AddToGrid(Player *obj, NGridType *grid, Cell const& cell)
{
	(*grid)(cell.CellX(), cell.CellY()).AddWorldObject(obj, obj->GetGUID());
}

template<>
void Map::AddToGrid(Creature* obj, NGridType *grid, Cell const& cell)
{
	// add to world object registry in grid
	if(obj->isPet())
	{
	}
	// add to grid object store
	else
	{
		(*grid)(cell.CellX(), cell.CellY()).AddGridObject<Creature>(obj, obj->GetGUID());
		obj->SetCurrentCell(cell);
	}
}

template<class T>
void Map::RemoveFromGrid(T* obj, NGridType *grid, Cell const& cell)
{
	(*grid)(cell.CellX(), cell.CellY()).template RemoveGridObject<T>(obj, obj->GetGUID());
}

template<class T>
void Map::AddNotifier(T* , Cell const& , CellPair const& )
{
}

template<>
void Map::AddNotifier(Player* obj, Cell const& cell, CellPair const& cellpair)
{
	PlayerRelocationNotify(obj,cell,cellpair);
}

template<>
void Map::AddNotifier(Creature* obj, Cell const& cell, CellPair const& cellpair)
{
	CreatureRelocationNotify(obj,cell,cellpair);
}

void Map::Reset()
{
	UnloadAll(false);
}

bool Map::CanEnter(Player *player) const
{
	return true;
}

void Map::Add(Player *player)
{
	player->AddToWorld();

	SendInitSelf(player);

	// update player state for other and vice-versa
	CellPair p = LeGACY::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
	Cell cell(p);
	EnsureGridLoadedForPlayer(cell, player, true);

	// TODO: FIX HERE
	UpdatePlayerVisibility(player, cell, p);
	//UpdateObjectsVisibilityFor(player, cell, p);
	
	AddNotifier(player, cell, p);

	// reinitialize reset time
	//InitResetTime();

}

template<class T>
void
Map::Add(T *obj)
{
	CellPair p = LeGACY::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());

	assert(obj);

	if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
	{
		sLog.outError("Map::Add: Object " I64FMTD " have invalid coordinates X:%u Y:%u grid cell [%u,%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
		return;
	}

	Cell cell(p);
	EnsureGridCreated(GridPair(cell.GridX(), cell.GridY()));
	NGridType *grid = getNGrid(cell.GridX(), cell.GridY());
	assert( grid != NULL );

	AddToGrid(obj,grid,cell);
	obj->AddToWorld();

	DEBUG_LOG("Object %u enters grid[%u,%u]", GUID_LOPART(obj->GetGUID()), cell.GridX(), cell.GridY());

	UpdateObjectVisibility(obj,cell,p);

	AddNotifier(obj,cell,p);
}


void Map::MessageBroadcast(Player *player, WorldPacket *msg, bool to_self, bool own_team_only)
{
	//sLog.outDetail("Map::MessageBroadcast Player");
	CellPair p = LeGACY::ComputeCellPair(player->GetPositionX(), player->GetPositionY());

	if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
	{
		sLog.outError("Map::MessageBroadcast: Player '%s' (%u) have invalid coordinates X:%f Y:%f grid cell [%u:%u]", player->GetName(), player->GetAccountId(), player->GetPositionX(), player->GetPositionY(), p.x_coord, p.y_coord);
		return;
	}

	Cell cell(p);
	cell.data.Part.reserved = CENTER_DISTRICT;

	if( !loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)) ) {
		sLog.outDetail("Not loaded");
	//	return;
	}

	LeGACY::MessageDeliverer post_man(*player, msg, to_self, own_team_only);
	TypeContainerVisitor<LeGACY::MessageDeliverer, WorldTypeMapContainer > message(post_man);
	CellLock<ReadGuard> cell_lock(cell, p);
	cell_lock->Visit(cell_lock, message, *this);

}

void Map::MessageBroadcast(WorldObject *obj, WorldPacket *msg)
{
//	sLog.outString("Map::MessageBroadcast WorldObject");
	CellPair p = LeGACY::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());

	if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
	{
		sLog.outError("Map::MessageBroadcast: Object " I64FMTD " have invalid coordinates X:%f Y:%f grid cell [%u,%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
		return;
	}

	Cell cell(p);
	cell.data.Part.reserved = CENTER_DISTRICT;
	cell.SetNoCreate();

	if( !loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)) )
		return;

	LeGACY::ObjectMessageDeliverer post_man(*obj, msg);
	TypeContainerVisitor<LeGACY::ObjectMessageDeliverer, WorldTypeMapContainer > message(post_man);
	CellLock<ReadGuard> cell_lock(cell, p);
	cell_lock->Visit(cell_lock, message, *this);

}

bool Map::loaded(const GridPair &p) const
{
	return ( getNGrid(p.x_coord, p.y_coord) && isGridObjectDataLoaded(p.x_coord, p.y_coord) );
}

void Map::Update(const uint32 &t_diff)
{
	return;
	for (GridRefManager<NGridType>::iterator i = GridRefManager<NGridType>::begin(); i != GridRefManager<NGridType>::end(); )
	{
		NGridType *grid = i->getSource();
		GridInfo *info = i->getSource()->getGridInfoRef();
		++i; // The update might delete the map and we need the next map before the iterator gets invalid
		assert(grid->GetGridState() >= 0 && grid->GetGridState() < MAX_GRID_STATE);
		si_GridStates[grid->GetGridState()]->Update(*this, *grid, *info, grid->getX(), grid->getY(), t_diff);
	}
}

void Map::Remove(Player *player, bool remove)
{

	CellPair p = LeGACY::ComputeCellPair(player->GetPositionX(), player->GetPositionY());
	if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
	{
		// invalid coordinates
		player->RemoveFromWorld();
		if( remove )
			DeleteFromWorld(player);

		return;
	}

	Cell cell(p);
	
	if( !getNGrid(cell.data.Part.grid_x, cell.data.Part.grid_y) )
	{
		sLog.outError("MAP: Map::Remove() i_grids was NULL x:%d, y:%d",cell.data.Part.grid_x,cell.data.Part.grid_y);
		return;
	}
	DEBUG_LOG("MAP: Remove player '%s' from map %u grid[%u,%u]", player->GetName(), player->GetMapId(), cell.GridX(), cell.GridY());
	NGridType *grid = getNGrid(cell.GridX(), cell.GridY());
	assert(grid != NULL);

	RemoveFromGrid(player,grid,cell);
	player->RemoveFromWorld();


	UpdateObjectsVisibilityFor(player,cell,p);

	if( remove )
		DeleteFromWorld(player);

	// reinitialize reset time
//	InitResetTime();
}

template<class T>
void
Map::Remove(T *obj, bool remove)
{
	CellPair p = LeGACY::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
	if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
	{
		sLog.outError("Map::Remove: Object " I64FMTD " have invalid coordinates X: %f Y:%f grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), p.x_coord, p.y_coord);
		return;
	}

	Cell cell(p);
	if( !loaded(GridPair(cell.data.Part.grid_x, cell.data.Part.grid_y)) )
		return;

	DEBUG_LOG("Remove object " I64FMTD " from grid[%u,%u]", obj->GetGUID(), cell.data.Part.grid_x, cell.data.Part.grid_y);
	NGridType *grid = getNGrid(cell.GridX(), cell.GridY());
	assert( grid != NULL );

	RemoveFromGrid(obj,grid,cell);
	obj->RemoveFromWorld();

	UpdateObjectVisibility(obj,cell,p);

	if( remove )
	{
		DeleteFromWorld(obj);
	}
}

void
Map::PlayerRelocation(Player *player, uint16 x, uint16 y)
{
	player->Relocate(x, y);
}

void Map::UnloadAll(bool pForce)
{
}

const char* Map::GetMapName() const
{
//	return i_mapEntry ? i_mapEntry->name : "UNNAMEDMAP\x0";
}

void Map::SendInitSelf( Player *player )
{
	sLog.outDetail("Creating player data for himself %u", player->GetAccountId());
}

template<class T>
void Map::DeleteFromWorld(T* obj)
{
	delete obj;
}


void
Map::EnsureGridLoadedForPlayer(const Cell &cell, Player *player, bool add_player)
{
	EnsureGridCreated(GridPair(cell.GridX(), cell.GridY()));
	NGridType *grid = getNGrid(cell.GridX(), cell.GridY());

	assert(grid != NULL);
	if( !isGridObjectDataLoaded(cell.GridX(), cell.GridY()) )
	{
		if( player != NULL )
		{
			player->SendDelayResponse(MAX_GRID_LOAD_TIME);
			DEBUG_LOG("Player %s enter cell[%u,%u] triggers of loading grid[%u,%u] on map %u", player->GetName(), cell.CellX(), cell.CellY(), cell.GridX(), cell.GridY(), i_id);
		}
		else
		{
			DEBUG_LOG("Player nearby triggers of loading grid [%u,%u] on map %u", cell.GridX(), cell.GridY(), i_id);
		}

		ObjectGridLoader loader(*grid, this, cell);
		loader.LoadN();
		setGridObjectDataLoaded(true, cell.GridX(), cell.GridY());

		ResetGridExpiry(*getNGrid(cell.GridX(), cell.GridY()), 0.1f);
		grid->SetGridState(GRID_STATE_ACTIVE);

		if( add_player && player != NULL )
			(*grid)(cell.CellX(), cell.CellY()).AddWorldObject(player, player->GetAccountId());
	}
	else if( player && add_player )
		AddToGrid(player, grid, cell);
}

void
Map::EnsureGridCreated(const GridPair &p)
{
	if(!getNGrid(p.x_coord, p.y_coord))
	{
		Guard guard(*this);
		if(!getNGrid(p.x_coord, p.y_coord))
		{
			setNGrid(new NGridType(p.x_coord*MAX_NUMBER_OF_GRIDS + p.y_coord, p.x_coord, p.y_coord, i_gridExpiry, sWorld.getConfig(CONFIG_GRID_UNLOAD)),
				p.x_coord, p.y_coord);

			// build a linkage between this map and NGridType
			buildNGridLinkage(getNGrid(p.x_coord, p.y_coord));

			getNGrid(p.x_coord, p.y_coord)->SetGridState(GRID_STATE_IDLE);

		}
	}
}

void Map::UpdateObjectVisibility( WorldObject* obj, Cell cell, CellPair cellpair)
{
	/*
	cell.data.Part.reserved = CENTER_DISTRICT;
	cell.SetNoCreate();
	LeGACY::VisibleChangesNotifier notifier(*obj);
	TypeContainerVisitor<LeGACY::VisibleChangesNotifier, WorldTypeMapContainer > player_notifier(notifier);
	CellLock<GridReadGuard> cell_lock(cell, cellpair);
	cell_lock->Visit(cell_lock, player_notifier, *this);
	*/
}

void Map::UpdatePlayerVisibility( Player* player, Cell cell, CellPair cellpair )
{
	
	cell.data.Part.reserved = CENTER_DISTRICT;

	LeGACY::PlayerNotifier pl_notifier(*player);
	TypeContainerVisitor<LeGACY::PlayerNotifier, WorldTypeMapContainer > player_notifier(pl_notifier);

	CellLock<ReadGuard> cell_lock(cell, cellpair);
	cell_lock->Visit(cell_lock, player_notifier, *this);
}

void Map::UpdateObjectsVisibilityFor( Player* player, Cell cell, CellPair cellpair)
{
	/*
	LeGACY::VisibleNotifier notifier(*player);

	cell.data.Part.reserved = CENTER_DISTRICT;
	cell.SetNoCreate();
	TypeContainerVisitor<LeGACY::VisibleNotifier, WorldTypeMapContainer > world_notifier(notifier);
	TypeContainerVisitor<LeGACY::VisibleNotifier, GridTypeMapContainer  > grid_notifier(notifier);
	CellLock<GridReadGuard> cell_lock(cell, cellpair);
	cell_lock->Visit(cell_lock, world_notifier, *this);
	cell_lock->Visit(cell_lock, grid_notifier,  *this);

	// send data
	notifier.Notify();
	*/
}

void Map::PlayerRelocationNotify(Player* player, Cell cell, CellPair cellpair)
{
	CellLock<ReadGuard> cell_lock(cell, cellpair);
	LeGACY::PlayerRelocationNotifier relocationNotifier(*player);
	cell.data.Part.reserved = ALL_DISTRICT;

	TypeContainerVisitor<LeGACY::PlayerRelocationNotifier, GridTypeMapContainer > p2grid_relocation(relocationNotifier);
	TypeContainerVisitor<LeGACY::PlayerRelocationNotifier, WorldTypeMapContainer > p2world_relocation(relocationNotifier);

	cell_lock->Visit(cell_lock, p2grid_relocation, *this);
	cell_lock->Visit(cell_lock, p2world_relocation, *this);
}

void
Map::CreatureRelocation(Creature *creature, uint16 x, uint16 y)
{
	Cell old_cell = creature->GetCurrentCell();

	CellPair new_val = LeGACY::ComputeCellPair(x, y);
	Cell new_cell(new_val);

	// delay creature move for grid/cell to grid/cell moves
	if( old_cell.DiffCell(new_cell) || old_cell.DiffGrid(new_cell) )
	{
		AddCreatureToMoveList(creature, x, y);
	}
	else
	{
		creature->Relocate(x, y);
		CreatureRelocationNotify(creature,new_cell,new_val);
	}
}

void Map::AddCreatureToMoveList(Creature* c, uint16 x, uint16 y)
{
	if(!c) return;

	i_creaturesToMove[c] = CreatureMover(x,y);
}

void Map::MoveAllCreaturesInMoveList()
{
	while(!i_creaturesToMove.empty())
	{
		// get data and remove element
		CreatureMoveList::iterator iter = i_creaturesToMove.begin();
		Creature* c = iter->first;
		CreatureMover cm = iter->second;
		i_creaturesToMove.erase(iter);

		c->Relocate(cm.x, cm.y);
	}
}

void Map::CreatureRelocationNotify(Creature *creature, Cell cell, CellPair cellpair)
{
}

template void Map::Add(Creature *);
template void Map::Add(GameObject *);

template void Map::Remove(Creature *,bool);
template void Map::Remove(GameObject *,bool);
