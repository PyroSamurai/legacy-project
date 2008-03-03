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

#ifndef __LEGACY_OBJECTACCESSOR_H
#define __LEGACY_OBJECTACCESSOR_H

#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "zthread/FastMutex.h"
#include "Utilities/HashMap.h"
#include "Policies/ThreadingModel.h"
#include "GameObject.h"
#include "ByteBuffer.h"

#include "GridDefines.h"
#include "Object.h"
#include "Player.h"

#include <set>

class Unit;
//class GameObject;
//class DynamicObject;
class WorldObject;
class Map;

template <class T>
class HashMapHolder
{
	public:

		typedef HM_NAMESPACE::hash_map< uint32, T* > MapType;
		typedef ZThread::FastMutex LockType;
		typedef LeGACY::GeneralLock<LockType > Guard;

		static void Insert(T* o) { m_objectMap[o->GetGUID()] = o; }
		static void Remove(T* o)
		{
			Guard guard(i_lock);
			typename MapType::iterator itr = m_objectMap.find(o->GetGUID());
			if(itr != m_objectMap.end())
				m_objectMap.erase(itr);
		}
		static T* Find(uint32 accountId)
		{
			typename MapType::iterator itr = m_objectMap.find(accountId);
			return (itr != m_objectMap.end()) ? itr->second : NULL;
		}

		static MapType& GetContainer() { return m_objectMap; }

		static LockType* GetLock() { return &i_lock; }
	private:

		// Non instantiable only static
		HashMapHolder() {}

		static LockType i_lock;
		static MapType m_objectMap;
};
		

class LEGACY_DLL_DECL ObjectAccessor : public LeGACY::Singleton<ObjectAccessor, LeGACY::ClassLevelLockable<ObjectAccessor, ZThread::FastMutex> >
{
	friend class LeGACY::OperatorNew<ObjectAccessor>;
	ObjectAccessor();
	~ObjectAccessor();
	ObjectAccessor(const ObjectAccessor &);
	ObjectAccessor& operator=(const ObjectAccessor &);

	public:

		template<class T> static T* GetObjectInWorld(uint64 guid, T* /*fake*/)
		{
			return HashMapHolder<T>::Find(guid);
		}

		static Unit* GetUnit(WorldObject const &, uint32);

		static Player* GetPlayer(Unit const &, uint32 accountId) { return FindPlayer(accountId); }


		static Player* FindPlayer(uint32);

		Player* FindPlayerByName(const char *name);

		template<class T> void AddObject(T *object)
		{
			HashMapHolder<T>::Insert(object);
		}

		template<class T> void RemoveObject(T *object)
		{
			HashMapHolder<T>::Remove(object);
		}

		void RemoveObject(Player *pl)
		{
			HashMapHolder<Player>::Remove(pl);

			Guard guard(i_updateGuard);

			std::set<Object *>::iterator iter2 = std::find(i_objects.begin(), i_objects.end(), (Object *)pl);
			if( iter2 != i_objects.end() )
				i_objects.erase(iter2);
		}

		void SaveAllPlayers();

		void AddUpdateObject(Object *obj);
		void RemoveUpdateObject(Object *obj);

		void Update(uint32 diff);


		static void UpdateObject(Object* obj, Player* exceptPlayer);
	private:

		typedef ZThread::FastMutex LockType;
		typedef LeGACY::GeneralLock<LockType > Guard;

		void _update(void);
		std::set<Object *> i_objects;
		//std::set<WorldObject *> i_objectsToRemove;
		LockType i_playerGuard;
		LockType i_updateGuard;
		LockType i_removeGuard;
};

#endif
