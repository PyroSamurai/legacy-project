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

#ifndef __LEGACY_ENCODING_H
#define __LEGACY_ENCODING_H

#include "Common.h"
#include "Policies/Singleton.h"

#define UINT8_ENCODER  0xAD
#define UINT16_ENCODER 0xADAD
#define UINT32_ENCODER 0xADADADAD
#define UINT64_ENCODER 0xADADADAD

#define ENCODE(x) ((sEncoding.Encode(x)))

class Encoding
{
	public:
		Encoding();
		~Encoding();

		const char   Encode(const char x);
		const uint8  Encode(const uint8 x);
		const uint16 Encode(const uint16 x);
		const uint32 Encode(const uint32 x);
		const uint64 Encode(const uint64 x);

		std::string Encode(const std::string str);

		const char * Encode(const char * str);
};

#define sEncoding LeGACY::Singleton<Encoding>::Instance()
#endif
