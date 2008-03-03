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

#ifndef __LEGACY_WORLDPACKET_H
#define __LEGACY_WORLDPACKET_H

#include "Common.h"
#include "ByteBuffer.h"
#include "Encoding.h"

#include <vector>

#define BLANK uint8(0)

class WorldPacket : public ByteBuffer
{
    public:
                                                            // just container for later use
        WorldPacket()                                       : ByteBuffer(0), m_opcode(0)
        {
        }
        explicit WorldPacket(uint16 opcode, size_t res=200) : ByteBuffer(res), m_opcode(opcode) { }
                                                            // copy constructor
        WorldPacket(const WorldPacket &packet)              : ByteBuffer(packet), m_opcode(packet.m_opcode)
        {
        }

        void Initialize(uint16 opcode, size_t newres=200)
        {
            clear();
            _storage.reserve(newres);
            m_opcode = opcode;
        }

        uint8 GetOpcode() const { return m_opcode; }
        void SetOpcode(uint8 opcode) { m_opcode = opcode; }
		uint16 GuessSize() const { return uint16(0); }

		uint16 GetHeader()
		{
			// 59 E9
			return 0xE959;
			//return 0x44F4;
		}

		void Prepare()
		{
			append(GetHeader());
			append(GuessSize());
			append(GetOpcode());
		}

		///- Finalize packet before sending to socket
		//   recalculate length packet data byte
		//   ensure opcode byte encoding
		//   encode all packet data byte
		void Finalize()
		{
			uint16 header;
			uint16 cur_sz, real_sz;
			uint8  opcode;

			header = read<uint16>();
			cur_sz = read<uint16>();
			opcode = read<uint8>();


			real_sz = size() - 4;
			if (cur_sz != real_sz) {
				put(2, ENCODE(uint16 (real_sz)));
			}

			put(4, ENCODE(m_opcode));
/*
			for (size_t i = 5; i <= size(); i++) {
				_storage[i] = ENCODE(_storage[i]);
			}
 */
		}

void Packed(WorldPacket *packet, uint8 opcode, uint8 value)
{
	uint16 newsize, pktsize;
	newsize = 5 + sizeof(value);
	pktsize = sizeof(value);
	packet->Initialize(newsize);
	packet->SetOpcode(opcode);
	*packet << GetHeader();
	*packet << (uint16) (pktsize ^ 0xADAD);
	*packet << (uint8) (opcode ^ 0xAD);
	*packet << value;
}

void Packed(WorldPacket *packet, uint8 opcode, uint16 value)
{
	uint16 newsize, pktsize;
	newsize = 5 + sizeof(value);
	pktsize = sizeof(value);
	packet->Initialize(newsize);
	packet->SetOpcode(opcode);
	*packet << GetHeader();
	*packet << (uint16) (pktsize ^ 0xADAD);
	*packet << (uint8) (opcode ^ 0xAD);
	*packet << value;
}

void Packed(WorldPacket *packet, uint8 opcode, uint32 value)
{
	uint16 newsize, pktsize;
	newsize = 5 + sizeof(value);
	pktsize = sizeof(value);
	packet->Initialize(newsize);
	packet->SetOpcode(opcode);
	*packet << GetHeader();
	*packet << (uint16) (pktsize ^ 0xADAD);
	*packet << (uint8) (opcode ^ 0xAD);
	*packet << value;
}

void Packed(WorldPacket *packet, uint8 opcode, std::string value)
{
	uint16 newsize, pktsize;
	newsize = 5 + value.length();
	pktsize = value.length();
//	printf("String size is %u\n", pktsize);
	packet->Initialize(newsize);
	packet->SetOpcode(opcode);
	*packet << GetHeader();
	*packet << (uint16) (pktsize ^ 0xADAD);
	*packet << (uint8) (opcode ^ 0xAD);
	for (unsigned int i = 0; i < value.length(); i++) {
		*packet << (uint8) value[i];
	}
}

void Packed(WorldPacket *packet, uint8 opcode, uint16 *value, uint16 size)
{
	uint16 newsize, pktsize;
	newsize = 5 + size;
	pktsize = size;
	sLog.outString("Array size: %u", pktsize);
	packet->Initialize(newsize);
	packet->SetOpcode(opcode);
	*packet << GetHeader();
	*packet << (uint16) (pktsize ^ 0xADAD);
	*packet << (uint8) (opcode ^ 0xAD);
	for (uint16 i = 0; i < pktsize; i++) {
		*packet << (uint16) value[i];
	}
}


    protected:
        uint8 m_opcode;
};
#endif
