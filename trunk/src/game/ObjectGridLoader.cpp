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




/*
template<class T>
void LoadHelper(CellGuidSet const& guid_set, CellPair &cell, GridRefManager<T> &m, uint32 &count, Map* map)
{
}
*/

void
ObjectGridLoader::Visit(GameObjectMapType &m)
{
}
/*
void
ObjectGridLoader::Visit(CreatureMapType &m)
{
}
*/

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
	i_cell.data.Part.cell_y = 0;
	for(unsigned int x= 0; x < MAX_NUMBER_OF_CELLS; ++x)
	{
		i_cell.data.Part.cell_x = x;
		for(unsigned int y=0; y < MAX_NUMBER_OF_CELLS; ++y)
		{
			i_cell.data.Part.cell_y = y;
			//GridLoader<Player, AllWorldObjectTypes, AllGridObjectTypes> loader;
			//loader.Load(i_grid(x, y), *this);
		}
	}
	sLog.outDebug("ObjectGridLoader::Load FIX: ... loaded for grid %u on map %u", i_grid.GetGridId(), i_map->GetId());

}

