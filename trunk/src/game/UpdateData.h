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

#ifndef __LEGACY_UPDATEDATA_H
#define __LEGACY_UPDATEDATA_H

class WorldPacket;

enum OBJECT_UPDATE_TYPE
{
	UPDATETYPE_VALUES               = 0,
	UPDATETYPE_MOVEMENT             = 1,
	UPDATETYPE_CREATE_OBJECT        = 2,
	UPDATETYPE_CREATE_OBJECT2       = 3,
	UPDATETYPE_OUT_OF_RANGE_OBJECTS = 4,
	UPDATETYPE_NEAR_OBJECTS         = 5
};

enum OBJECT_UPDATE_FLAGS
{
	UPDATEFLAG_NONE               = 0x00,
	UPDATEFLAG_SELF               = 0x01,
	UPDATEFLAG_LOWGUID            = 0x08,
	UPDATEFLAG_HIGHGUID           = 0x10
};

class UpdateData
{
	public:
		UpdateData();

		void AddOutOfRangeGUID(std::set<uint64>& guids);
		void AddOutOfRangeGUID(const uint64 &guid);
		void AddUpdateBlock(const ByteBuffer &block);
		bool BuildPacket(WorldPacket *packet, bool hasTransport = false);
		bool HasData() { return m_blockCount > 0 || !m_outOfRangeGUIDs.empty(); }

		void Clear();

	protected:
		uint32 m_blockCount;
		std::set<uint64> m_outOfRangeGUIDs;
		ByteBuffer m_data;

};

#endif
