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

#include "ObjectGridLoader.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "Creature.h"
#include "GameObject.h"
#include "DynamicObject.h"
#include "Corpse.h"
#include "World.h"
#include "CellImpl.h"

// for loading world object at grid loading (Corpses)
class ObjectWorldLoader
{
	public:
		explicit ObjectWorldLoader(ObjectGridLoader& gloader)
			: i_cell(gloader.i_cell), i_grid(gloader.i_grid), i_map(gloader.i_map)
		{}

		template<class T> void Visit(GridRefManager<T>&) { }

	private:
		Cell i_cell;
		NGridType &i_grid;
		Map* i_map;
	public:
		uint32 i_corpses;
};

template<class T> void addUnitState(T* /*obj*/, CellPair const& /*cell_pair*/)
{
}

template<> void addUnitState(Creature *obj, CellPair const& cell_pair)
{
	Cell cell(cell_pair);

	obj->SetCurrentCell(cell);
}

template <class T>
void LoadHelper(CellGuidSet const& guid_set, CellPair &cell, GridRefManager<T> &m, uint32 &count, Map* map)
{
	for(CellGuidSet::const_iterator i_guid = guid_set.begin(); i_guid != guid_set.end(); ++i_guid)
	{
		T* obj = new T(NULL);
		uint32 guid = *i_guid;
		sLog.outString(" >> LoadHelper::Object Creating GUID %u", guid);
		
		if(!obj->LoadFromDB(guid, 0)) //map->GetInstanceId()))
		{
			delete obj;
			continue;
		}
		//obj->SetInstanceId(map->GetInstanceId());
		obj->GetGridRef().link(&m, obj);

		//sLog.outString("    >> LoadHelper::Adding Object to World GUID(%u) MAPID(%u)", obj->GetGUIDLow(), ((Creature*) obj)->GetMapId()); 
		addUnitState(obj,cell);
		obj->AddToWorld();
		++count;
	}
}

void
ObjectGridLoader::Visit(GameObjectMapType &m)
{
//	sLog.outString("ObjectGridLoader::Visit GameObjectMapType");
	uint32 x = (i_cell.GridX()*MAX_NUMBER_OF_CELLS) + i_cell.CellX();
	uint32 y = (i_cell.GridY()*MAX_NUMBER_OF_CELLS) + i_cell.CellY();
	CellPair cell_pair(x,y);
	uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

	CellObjectGuids const& cell_guids = objmgr.GetCellObjectGuids(i_map->GetId(), cell_id);

	LoadHelper(cell_guids.gameobjects, cell_pair, m, i_gameObjects, i_map);
}

void
ObjectGridLoader::Visit(CreatureMapType &m)
{
//	sLog.outString("ObjectGridLoader::Visit CreatureMapType");
	uint32 x = (i_cell.GridX()*MAX_NUMBER_OF_CELLS) + i_cell.CellX();
	uint32 y = (i_cell.GridY()*MAX_NUMBER_OF_CELLS) + i_cell.CellY();
	CellPair cell_pair(x,y);
	uint32 cell_id = (cell_pair.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

	sLog.outString( "ObjectGridLoader::Visit Map %u Cell %u", i_map->GetId(), cell_id);
	CellObjectGuids const& cell_guids = objmgr.GetCellObjectGuids(i_map->GetId(), cell_id);

	LoadHelper(cell_guids.creatures, cell_pair, m, i_creatures, i_map);
}


void
ObjectGridLoader::Load(GridType &grid)
{
	{
		TypeContainerVisitor<ObjectGridLoader, GridTypeMapContainer > loader(*this);
		grid.Visit(loader);
	}

	{
		ObjectWorldLoader wloader(*this);
		TypeContainerVisitor<ObjectWorldLoader, WorldTypeMapContainer > loader(wloader);
		grid.Visit(loader);
//		i_corpses = wloader.i_corpses;
	}
}

void ObjectGridLoader::LoadN(void)
{
	sLog.outString("");
	sLog.outString("ObjectGridLoader::LoadN Start");
	i_gameObjects = 0; i_creatures = 0; i_corpses = 0;
	i_cell.data.Part.cell_y = 0;
	for(unsigned int x= 0; x < MAX_NUMBER_OF_CELLS; ++x)
	{
		i_cell.data.Part.cell_x = x;
		for(unsigned int y=0; y < MAX_NUMBER_OF_CELLS; ++y)
		{
			sLog.outString(" - Cell of [%u,%u]", x, y);
			i_cell.data.Part.cell_y = y;
	 		GridLoader<Player, AllWorldObjectTypes, AllGridObjectTypes> loader;
			loader.Load(i_grid(x, y), *this);
		}
	}
	sLog.outDebug("%u GameObjects, %u Creatures, and %u Corpses/Bones loaded for grid %u on map %u", i_gameObjects, i_creatures, i_corpses, i_grid.GetGridId(), i_map->GetId());
	sLog.outString("");

}

