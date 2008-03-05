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
#include "GameObject.h"
//#include "DynamicObject.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "Opcodes.h"

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

Unit*
ObjectAccessor::GetUnit(WorldObject const &u, uint32 accountId)
{
	return FindPlayer(accountId);
}

Player*
ObjectAccessor::FindPlayer(uint32 accountId)
{
	return GetObjectInWorld(accountId, (Player*)NULL);
}

Player*
ObjectAccessor::FindPlayerByName(const char *name)
{
	//TODO: Player Guard
	return NULL;
}

void ObjectAccessor::Update(uint32 diff)
{
	_update();
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

/// Define the static member of HashMapHolder

template <class T> HM_NAMESPACE::hash_map< uint32, T* > HashMapHolder<T>::m_objectMap;
template <class T> ZThread::FastMutex HashMapHolder<T>::i_lock;

/// Global definitions for the hashmap storage

template class HashMapHolder<Player>;




//template Player* ObjectAccessor::GetObjectInWorld<Player>(uint32 mapid, uint16 x, uint16 y, uint64 guid, Player* /*fake*/);
