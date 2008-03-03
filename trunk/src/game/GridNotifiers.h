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

#ifndef __LEGACY_GRIDNOTIFIERS_H
#define __LEGACY_GRIDNOTIFIERS_H

#include "ObjectGridLoader.h"
#include "ByteBuffer.h"
#include "UpdateData.h"
#include <iostream>

#include "Object.h"
#include "Player.h"
#include "Unit.h"

class Player;

namespace LeGACY
{

	struct LEGACY_DLL_DECL PlayerNotifier
	{
		explicit PlayerNotifier(Player &pl) : i_player(pl) {}
		void Visit(PlayerMapType &);
		template<class SKIP> void Visit(GridRefManager<SKIP> &) {}
		Player &i_player;
	};

	struct LEGACY_DLL_DECL MessageDeliverer
	{
		Player &i_player;
		WorldPacket *i_message;
		bool i_toSelf;
		bool i_ownTeamOnly;
		MessageDeliverer(Player &pl, WorldPacket *msg, bool to_self, bool ownTeamOnly) : i_player(pl), i_message(msg), i_toSelf(to_self), i_ownTeamOnly(ownTeamOnly) {}
		void Visit(PlayerMapType &m);
		template<class SKIP> void Visit(GridRefManager<SKIP> &) {}
	};

	struct LEGACY_DLL_DECL ObjectMessageDeliverer
	{
		Object &i_object;
		WorldPacket *i_message;
		ObjectMessageDeliverer(Object &obj, WorldPacket *msg) : i_object(obj), i_message(msg) {}
		void Visit(PlayerMapType &m);
		template<class SKIP> void Visit(GridRefManager<SKIP> &) {}
	};

	struct LEGACY_DLL_DECL PlayerRelocationNotifier
	{
		Player &i_player;
		PlayerRelocationNotifier(Player &pl) : i_player(pl) {}
		template<class T> void Visit(GridRefManager<T> &) {}
		void Visit(PlayerMapType &);
	};

	#ifndef WIN32
	template<> void PlayerRelocationNotifier::Visit<Player>(PlayerMapType &);
	#endif

}
#endif
