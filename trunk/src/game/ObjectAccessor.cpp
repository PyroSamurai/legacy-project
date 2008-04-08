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

#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Policies/SingletonImp.h"
#include "Player.h"
#include "Creature.h"
#include "GameObject.h"
#include "Corpse.h"
#include "DynamicObject.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "Item.h"
#include "GridNotifiers.h"
#include "MapManager.h"
#include "Map.h"
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "Opcodes.h"
#include "ObjectDefines.h"

#include <cmath>

#define CLASS_LOCK LeGACY::ClassLevelLockable<ObjectAccessor, ZThread::FastMutex>
INSTANTIATE_SINGLETON_2(ObjectAccessor, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(ObjectAccessor, ZThread::FastMutex);

namespace LeGACY
{
	struct LEGACY_DLL_DECL BuildUpdateForPlayer
	{

		Player &i_player;
	};
}

ObjectAccessor::ObjectAccessor() {}
ObjectAccessor::~ObjectAccessor() {}

Creature*
ObjectAccessor::GetNPCIfCanInteractWith(Player const &player, uint64 guid, uint32 npcflagmask)
{
	// unit checks
	if (!guid)
		return NULL;

	// exist
	Creature *unit = GetCreature(player, guid);
	if(!unit)
		return NULL;

	return unit;
}



Creature*
ObjectAccessor::GetCreature(WorldObject const &u, uint64 guid)
{
	sLog.outString("ObjectAccessor::GetCreature %u", guid);
	Creature * ret = GetObjectInWorld(guid, (Creature*)NULL);
	if(ret && ret->GetMapId() != u.GetMapId()) ret = NULL;
	if(ret)
	{
		sLog.outString("ret is not NULL GUID(%u) MAPID(%u)", ret->GetGUIDLow(), ret->GetMapId());
	}
	else
	{
		sLog.outString("ret is NULL >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	}
	return ret;
}

Unit*
ObjectAccessor::GetUnit(WorldObject const &u, uint64 guid)
{
	if(GUID_HIPART(guid)==HIGHGUID_PLAYER)
		return FindPlayer(guid);

	return GetCreature(u, guid);
}

Pet*
ObjectAccessor::GetPet(uint64 guid)
{
	return GetObjectInWorld(guid, (Pet*)NULL);
}

Player*
ObjectAccessor::FindPlayer(uint64 guid)
{
	return GetObjectInWorld(guid, (Player*)NULL);
}

Player*
ObjectAccessor::FindPlayerByAccountId(uint32 acc_id)
{
	uint32 guid = objmgr.GetPlayerGuidByAccountId(acc_id);
	return GetObjectInWorld(MAKE_GUID(guid, HIGHGUID_PLAYER), (Player*)NULL);
}

Player*
ObjectAccessor::FindPlayerByName(const char *name)
{
	//TODO: Player Guard
	HashMapHolder<Player>::MapType& m = HashMapHolder<Player>::GetContainer();
	HashMapHolder<Player>::MapType::iterator iter = m.begin();
	for(; iter != m.end(); ++iter)
		if( ::strcmp(name, iter->second->GetName()) == 0 )
			return iter->second;
	return NULL;
}

void ObjectAccessor::Update(uint32 diff)
{
	{
		typedef std::multimap<uint32, Player *> CreatureLocationHolderType;
		CreatureLocationHolderType creature_locations;
		//TODO: Player guard
		HashMapHolder<Player>::MapType& playerMap = HashMapHolder<Player>::GetContainer();
		for(HashMapHolder<Player>::MapType::iterator iter = playerMap.begin(); iter != playerMap.end(); ++iter)
		{
			if(iter->second->IsInWorld()) iter->second->Update(diff);
			creature_locations.insert( CreatureLocationHolderType::value_type(iter->second->GetMapId(), iter->second) );
		}

		Map *map;

		LeGACY::ObjectUpdater updater(diff);
		// for creature
		TypeContainerVisitor<LeGACY::ObjectUpdater, GridTypeMapContainer  > grid_object_update(updater);
		// for pets, not yet
		TypeContainerVisitor<LeGACY::ObjectUpdater, WorldTypeMapContainer > world_object_update(updater);

		for(CreatureLocationHolderType::iterator iter=creature_locations.begin(); iter != creature_locations.end(); ++iter)
		{
			MapManager::Instance().GetMap((*iter).first, (*iter).second)->resetMarkedCells();
		}

		for(CreatureLocationHolderType::iterator iter=creature_locations.begin(); iter != creature_locations.end(); ++iter)
		{
			Player *player = (*iter).second;
			map = MapManager::Instance().GetMap((*iter).first, player);

			CellPair standing_cell(LeGACY::ComputeCellPair(player->GetPositionX(), player->GetPositionY()));

			// Check for correctness of standing_cell, it also avoids problems with update_cell
			if(standing_cell.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || standing_cell.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
				continue;

			CellPair update_cell(standing_cell);

			update_cell << 1;
			update_cell -= 1;

			for( ; abs(int32(standing_cell.x_coord) - int32(update_cell.x_coord)) < 2; update_cell >> 1)
			{

				for(CellPair cell_iter=update_cell; abs(int32(standing_cell.y_coord) - int32(cell_iter.y_coord)) < 2; cell_iter += 1 )
				{
					uint32 cell_id = (cell_iter.y_coord*TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_iter.x_coord;

					if( !map->isCellMarked(cell_id) )
					{
						map->markCell(cell_id);
						Cell cell(cell_iter);
						cell.data.Part.reserved = CENTER_DISTRICT;
						cell.SetNoCreate();
						CellLock<NullGuard> cell_lock(cell, cell_iter);
						cell_lock->Visit(cell_lock, grid_object_update, *map);
						cell_lock->Visit(cell_lock, world_object_update, *map);
					}

					if(cell_iter.y_coord == TOTAL_NUMBER_OF_CELLS_PER_MAP)
						break;
				}

				if(update_cell.x_coord == TOTAL_NUMBER_OF_CELLS_PER_MAP)
					break;
			}
		}
	}

	_update();
}

GameObject*
ObjectAccessor::GetGameObject(Unit const &u, uint64 guid)
{
	GameObject * ret = GetObjectInWorld(guid, (GameObject*)NULL);
	if(ret && ret->GetMapId() != u.GetMapId()) ret = NULL;
	return ret;
}

void
ObjectAccessor::SaveAllPlayers()
{
}

void
ObjectAccessor::_update()
{
}

void
ObjectAccessor::UpdateObject(Object* obj, Player* exceptPlayer)
{
}

void
ObjectAccessor::AddUpdateObject(Object *obj)
{
	Guard guard(i_updateGuard);
	i_objects.insert(obj);
}

void
ObjectAccessor::RemoveUpdateObject(Object *obj)
{
	Guard guard(i_updateGuard);
	std::set<Object *>::iterator iter = i_objects.find(obj);
	if( iter != i_objects.end() )
		i_objects.erase( iter );
}

void ObjectAccessor::AddObjectToRemoveList(WorldObject *obj)
{
	if(!obj) return;

	Guard guard(i_removeGuard);
	i_objectsToRemove.insert(obj);
}

void ObjectAccessor::DoDelayedMovesAndRemoves()
{
	MapManager::Instance().MoveAllCreaturesInMoveList();
	RemoveAllObjectsInRemoveList();
}

void ObjectAccessor::RemoveAllObjectsInRemoveList()
{
	if(i_objectsToRemove.empty())
		return;

	Guard guard(i_removeGuard);

	while(!i_objectsToRemove.empty())
	{
		WorldObject* obj = *i_objectsToRemove.begin();
		i_objectsToRemove.erase(i_objectsToRemove.begin());
		switch(obj->GetTypeId())
		{
			case TYPEID_UNIT:
				//MapManager::Instance().GetMap(obj->GetMapId(), obj)->Remove((Creature*)obj,true);
				break;
			default:
				sLog.outError("Non-grid object (TypeId: %u) in grid object removing list, ignored.", obj->GetTypeId());
				break;
		}
	}
}

/// Define the static member of HashMapHolder

template <class T> HM_NAMESPACE::hash_map< uint64, T* > HashMapHolder<T>::m_objectMap;
template <class T> ZThread::FastMutex HashMapHolder<T>::i_lock;

/// Global definitions for the hashmap storage

template class HashMapHolder<Player>;
template class HashMapHolder<Pet>;
template class HashMapHolder<GameObject>;
template class HashMapHolder<DynamicObject>;
template class HashMapHolder<Creature>;
template class HashMapHolder<Corpse>;




template Player* ObjectAccessor::GetObjectInWorld<Player>(uint32 mapid, uint16 x, uint16 y, uint64 guid, Player* /*fake*/);
template Pet* ObjectAccessor::GetObjectInWorld<Pet>(uint32 mapid, uint16, uint16 u, uint64 guid, Pet* /*fake*/);
template Creature* ObjectAccessor::GetObjectInWorld<Creature>(uint32 mapid, uint16, uint16 u, uint64 guid, Creature* /*fake*/);
template Corpse* ObjectAccessor::GetObjectInWorld<Corpse>(uint32 mapid, uint16, uint16 u, uint64 guid, Corpse* /*fake*/);
template GameObject* ObjectAccessor::GetObjectInWorld<GameObject>(uint32 mapid, uint16, uint16 u, uint64 guid, GameObject* /*fake*/);
template DynamicObject* ObjectAccessor::GetObjectInWorld<DynamicObject>(uint32 mapid, uint16, uint16 u, uint64 guid, DynamicObject* /*fake*/);

