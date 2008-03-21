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

#include "ByteBuffer.h"
#include "UpdateData.h"

#include "GridDefines.h"
#include "Object.h"
#include "Player.h"

#include <set>

class Creature;
class Corpse;
class Unit;
class GameObject;
class DynamicObject;
class WorldObject;
class Map;

template <class T>
class HashMapHolder
{
	public:

		typedef HM_NAMESPACE::hash_map< uint64, T* > MapType;
		typedef ZThread::FastMutex LockType;
		typedef LeGACY::GeneralLock<LockType > Guard;

		static void Insert(T* o)
		{
			//sLog.outString(" ++ Insert Object GUID(%u)", o->GetGUID());
			m_objectMap[o->GetGUID()] = o;
		}
		static void Remove(T* o)
		{
			Guard guard(i_lock);
			typename MapType::iterator itr = m_objectMap.find(o->GetGUID());
			if(itr != m_objectMap.end())
				m_objectMap.erase(itr);
		}
		static T* Find(uint64 guid)
		{
			typename MapType::iterator itr = m_objectMap.find(guid);
			return (itr != m_objectMap.end()) ? itr->second : NULL;
			/*
			if(itr == m_objectMap.end())
			{
				sLog.outString("GUID(%u) not found", guid);
				return NULL;
			}
			return itr->second;
			*/
			/*
			typename MapType::iterator itr;
			sLog.outString("Finding GUID(%u) in Size(%u)", guid, Size());
			for(itr = m_objectMap.begin(); itr != m_objectMap.end(); ++itr)
			{
				sLog.outString("ITR GUID(%u)", itr->first);
				if (itr->first == guid)
				{
					sLog.outString("Found");// GUID(%u)", itr->first);
					//sLog.outString("In MAPID(%u)", itr->second);
					return itr->second;
				}
			}
			return NULL;
			*/
		}
		static uint32 Size()
		{
			return m_objectMap.size();
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
		//	sLog.outString("ObjectAccessor::GetObjectInWorld <T> GUID(%u)", guid);
			return HashMapHolder<T>::Find(guid);
		}

		static Unit* GetObjectInWorld(uint64 guid, Unit* /*fake*/)
		{
		//	sLog.outString("ObjectAccessor::GetObjectInWorld Unit GUID(%u)", guid);
			if(GUID_HIPART(guid) == HIGHGUID_PLAYER)
				return (Unit*)HashMapHolder<Player>::Find(guid);


			return (Unit*)HashMapHolder<Creature>::Find(guid);
		}

		template<class T> static T* GetObjectInWorld(uint32 mapid, uint16 x, uint16 y, uint64 guid, T* /*fake*/)
		{
			T* obj = HashMapHolder<T>::Find(guid);
			if(!obj || obj->GetMapId() != mapid) return NULL;

			CellPair p = LeGACY::ComputeCellPair(x,y);
			if(p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
			{
				//sLog.outError("ObjectAccessor::GetObjectInWorld: invalid coordinates supplied X: %u Y:%u grid cell [%u:%u]", x, y, p.x_coord, p.y_coord);
				return NULL;
			}

			CellPair q = LeGACY::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
			if(q.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || q.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP )
			{
				//sLog.outError("ObjectAccessor::GetObjectInWorld: object "I64FMTD" has invalid coordinates X:%u Y:%u grid cell [%u:%u]", obj->GetGUID(), obj->GetPositionX(), obj->GetPositionY(), q.x_coord, q.y_coord);
				return NULL;
			}

			int32 dx = int32(p.x_coord) - int32(q.x_coord);
			int32 dy = int32(p.y_coord) - int32(q.y_coord);

			if (dx > -2 && dx < 2 && dy > -2 && dy < 2) return obj;
			else return NULL;
		}

		static Creature* GetNPCIfCanInteractWith(Player const &player, uint64 guid, uint32 npcflagmask);
		static Creature* GetCreature(WorldObject const &, uint64);
		static Unit* GetUnit(WorldObject const &, uint64);
		static Pet* GetPet(Unit const &, uint64 guid) { return GetPet(guid); }
		static Player* GetPlayer(Unit const &, uint64 guid) { return FindPlayer(guid); }
		static GameObject* GetGameObject(Unit const &, uint64);

		static Pet* GetPet(uint64 guid);
		static Player* FindPlayer(uint64);

		Player* FindPlayerByName(const char *name);

		template<class T> void AddObject(T *object)
		{
			//sLog.outString(" ++ ObjectAccessor::AddObject GUID(%u)",object->GetGUID());
			//sLog.outString("    ++ HashMapHolder<T> Size: %u", HashMapHolder<T>::Size()); 
			HashMapHolder<T>::Insert(object);
			//sLog.outString("    ++ HashMapHolder<T> Size: %u", HashMapHolder<T>::Size()); 
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

		void AddObjectToRemoveList(WorldObject *obj);

		void DoDelayedMovesAndRemoves();

		void Update(uint32 diff);


		static void UpdateObject(Object* obj, Player* exceptPlayer);
	private:
		void RemoveAllObjectsInRemoveList();

		typedef ZThread::FastMutex LockType;
		typedef LeGACY::GeneralLock<LockType > Guard;

		void _update(void);
		std::set<Object *> i_objects;
		std::set<WorldObject *> i_objectsToRemove;
		LockType i_playerGuard;
		LockType i_updateGuard;
		LockType i_removeGuard;
};

#endif
