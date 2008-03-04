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

#include "Common.h"
#include "Encoding.h"
#include "Policies/SingletonImp.h"

INSTANTIATE_SINGLETON_1( Encoding );

Encoding::Encoding()
{
}

Encoding::~Encoding()
{
}

const char Encoding::Encode(const char x)
{
	return x ^ UINT8_ENCODER;
}

const uint8 Encoding::Encode(const uint8 x)
{
	return x ^ UINT8_ENCODER;
}

const uint16 Encoding::Encode(const uint16 x)
{
	return x ^ UINT16_ENCODER;
}

const uint32 Encoding::Encode(const uint32 x)
{
	return x ^ UINT32_ENCODER;
}

const uint64 Encoding::Encode(const uint64 x)
{
	return x ^ UINT64_ENCODER;
}

std::string Encoding::Encode(const std::string str)
{
	std::string estr;
	estr = str;
	for (int i = 0; estr[i] != '\0'; i++) {
		estr[i] = Encode((uint8) estr[i]);
	}
	return estr;
}

const char * Encoding::Encode(const char * str)
{
	std::string estr;
	estr = str;

	estr = ENCODE(estr);
	return estr.c_str();
}
