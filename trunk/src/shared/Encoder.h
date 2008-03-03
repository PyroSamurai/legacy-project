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

#ifndef __LEGACY_ENCODER_H
#define __LEGACY_ENCODER_H

#define UINT8_ENCODER  0xAD
#define UINT16_ENCODER 0xADAD
#define UINT32_ENCODER 0xADADADAD

const uint8 Encode(const uint8 x)
{
	return x ^ UINT8_ENCODER;
}

const uint16 Encode(const uint16 x)
{
	return x ^ UINT16_ENCODER;
}

const uint32 Encode(const uint32 x)
{
	return x ^ UINT32_ENCODER;
}

std::string Encode(const std::string str)
{
	std::string estr;
	estr = str;
	for (int i = 0; estr[i] != '\0'; i++) {
		estr[i] = Encode((uint8) estr[i]);
	}
	return estr;
}

#endif
