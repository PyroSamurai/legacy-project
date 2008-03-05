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

#ifndef __LEGACY_OBJECT_H
#define __LEGACY_OBJECT_H

#include "Common.h"
#include "ByteBuffer.h"
#include "UpdateFields.h"
#include "UpdateData.h"
#include "GameSystem/GridReference.h"
#include "ObjectDefines.h"

#include <set>
#include <string>

enum TYPE
{
	TYPE_OBJECT        = 1,
	TYPE_ITEM          = 2,
	TYPE_UNIT          = 6,
	TYPE_PLAYER        = 8,
	TYPE_GAMEOBJECT    = 16,
	TYPE_AREATRIGGER   = 32
};

enum TYPEID
{
	TYPEID_OBJECT      = 0,
	TYPEID_ITEM        = 1,
	TYPEID_UNIT        = 2,
	TYPEID_PLAYER      = 3,
	TYPEID_GAMEOBJECT  = 4,
	TYPEID_AREATRIGGER = 5
};

uint32 GuidHigh2TypeId(uint32 guid_hi);

class WorldPacket;
class UpdateData;
class ByteBuffer;
class WorldSession;
class Player;
class Map;

typedef HM_NAMESPACE::hash_map<Player*, UpdateData> UpdateDataMapType;

class LEGACY_DLL_SPEC Object
{
	public:
		virtual ~Object();

		const bool& IsInWorld() const { return m_inWorld; }
		virtual void AddToWorld()
		{
			if(m_inWorld)
				return;

			m_inWorld = true;

		}
		virtual void RemoveFromWorld() { m_inWorld = false; }

		const uint8& GetTypeId() const { return m_objectTypeId; }
		const uint64& GetGUID() const { return GetUInt64Value(0); }

		virtual void BuildCreateUpdateBlockForPlayer( WorldPacket *data, Player *target ) const;
		void SendUpdateToPlayer(Player* player);

		//virtual void BuildUpdateBlockVisibilityPacket( WorldPacket *data );
		//virtual void BuildUpdateBlockVisibilityForOthersPacket( WorldPacket *dat);




		const uint64& GetUInt64Value( uint16 index ) const
		{
			ASSERT( index + 1 < m_valuesCount || PrintIndexError( index, false)  );
			return *((uint64*)&(m_uint32Values[ index ]));
		}

		void SetInt32Value(  uint16 index,         int32  value );
		void SetUInt32Value( uint16 index,        uint32  value );
		void SetUInt64Value( uint16 index,  const uint64 &value );

//		bool LoadValues(const char* data);

		uint16 GetValuesCount() const { return m_valuesCount; }

		void InitValues() { _InitValues(); }

	protected:
		Object ( );

		void _InitValues();
		void _Create (uint32 guidlow, HighGuid guidhigh);

		uint16 m_objectType;

		uint8 m_objectTypeId;
		uint8 m_updateFlag;

		union
		{
			int32 *m_int32Values;
			uint32 *m_uint32Values;
			float *m_floatValues;
		};

		uint32 *m_uint32Values_mirror;

		uint16 m_valuesCount;

		bool m_objectUpdated;

	private:
		bool m_inWorld;

		ByteBuffer m_PackGUID;

		// for output helpfull error messages from asserts
		bool PrintIndexError(uint32 index, bool set) const;
		Object(const Object&);   // prevent generation copy constructor
		Object& operator=(Object const&); // prevent generation assignment operator
};

class LEGACY_DLL_SPEC WorldObject : public Object
{
	public:
		virtual ~WorldObject() {}

		virtual void Update(uint32 /*time_diff*/ ) {}

		void _Create( uint32 guidlow, HighGuid highguid, uint32 mapid, uint16 x, uint16 y);

		void Relocate(uint16 x, uint16 y)
		{
			m_positionX = x;
			m_positionY = y;
		}

		const char* GetName() const { return m_name.c_str(); }
		void SetName(std::string newname) { m_name = newname; }

		void SetMapId(uint16 newMap) { m_mapId = newMap; }

		uint16 GetMapId() const { return m_mapId; }

		uint16 GetPositionX( ) const { return m_positionX; }
		uint16 GetPositionY( ) const { return m_positionY; }
		void GetPosition(uint16 &x, uint16 &y) const
		{
			x = m_positionX; y = m_positionY;
		}

		// main visibility check function in normal case (ignore grey zone distance check)
//		bool isVisibleFor(Player const* u) const { return isVisibleForInState(u,false); }

		// low level function for visibility change code must be define in all main world object subclasses
//		virtual bool isVisibleForInState(Player const* u, bool inVisibleList) const = 0;

		Map      * GetMap() const;
		Map const* GetBaseMap() const;

	protected:
		explicit WorldObject( WorldObject *instantiator );
		std::string m_name;

	private:
		uint32 m_mapId;

		uint16 m_positionX;
		uint16 m_positionY;

};	

#endif
