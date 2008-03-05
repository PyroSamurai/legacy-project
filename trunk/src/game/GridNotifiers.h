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
#include "GameObject.h"
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
/*
	struct LEGACY_DLL_DECL VisibleNotifier
	{
		Player &i_player;
//		UpdateData i_data;
//		UpdateDataMapType i_data_updates;
		Player::ClientGUIDs i_clientGUIDs;
		std::set<WorldObject*> i_visibleNow;

		explicit VisibleNotifier(Player &player) : i_player(player),i_clientGUIDs(player.m_clientGUIDs) {}
		template<class T> void Visit(GridRefManager<T> &m);
		void Visit(PlayerMapType &);
		void Notify(void);
	};

	struct LEGACY_DLL_DECL VisibleChangesNotifier
	{
		WorldObject &i_object;

		explicit VisibleChangesNotifier(WorldObject &object) : i_object(object) {}
		template<class T> void Visit(GridRefManager<T> &) {}
		void Visit(PlayerMapType &);
	};

	struct LEGACY_DLL_DECL GridUpdater
	{
		GridType &i_grid;
		uint32 i_timeDiff;
		GridUpdater(GridType &grid, uint32 diff) : i_grid(grid), i_timeDiff(diff) {}

		template<class T> void updateObjects(GridRefManager<T> &m)
		{
			for(typename GridRefManager<T>::iterator iter = m.begin(); iter != m.end(); ++iter)
				iter->getSource()->Update(i_timeDiff);
		}

		void Visit(PlayerMapType &m) { updateObjects<Player>(m); }

	};
*/

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
