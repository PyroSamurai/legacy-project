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

#include "GridNotifiers.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "UpdateData.h"
#include "Map.h"
#include "MapManager.h"
#include "ObjectAccessor.h"

using namespace LeGACY;

void
LeGACY::PlayerNotifier::Visit(PlayerMapType &m)
{
	int i = 0;
	for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
	{
		if( iter->getSource() == &i_player )
			continue;

		i++;
		sLog.outString("GridNotifier %u PlayerNotifier from '%s' to '%s'",
			i, iter->getSource()->GetName(), i_player.GetName());

		iter->getSource()->UpdateVisibilityOf(&i_player);
//		if ( iter->getSource() != &i_player )
			i_player.UpdateVisibilityOf(iter->getSource());
	}
}
/*
void
VisibleChangesNotifier::Visit(PlayerMapType &m)
{
	for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
	{
		if(iter->getSource() == &i_object)
			continue;

		iter->getSource()->UpdateVisibilityOf(&i_object);
	}
}

void
VisibleNotifier::Visit(PlayerMapType &m)
{
	for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
	{
		if( iter->getSource() == &i_player )
			continue;

		iter->getSource()->UpdateVisibilityOf(&i_player);
		//i_player.UpdateVisibilityOf(iter->getSource(),i_data,i_data_updates,i_visibleNow);
		i_clientGUIDs.erase(iter->getSource()->GetGUID());
	}
}

void
VisibleNotifier::Notify()
{
}
*/
void
MessageDeliverer::Visit(PlayerMapType &m)
{
	for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
	{
		if( (iter->getSource() != &i_player || i_toSelf)
			&& (!i_ownTeamOnly || iter->getSource()->GetTeam() == i_player.GetTeam()) )
		{
			if(WorldSession* session = iter->getSource()->GetSession()) {
				sLog.outString("GridNotifiers MessageDeliverer::Visit PlayerMapType for '%s'", iter->getSource()->GetName());
				session->SendPacket(i_message);
			}
		}

	}
}

void
ObjectMessageDeliverer::Visit(PlayerMapType &m)
{
	for(PlayerMapType::iterator iter=m.begin(); iter != m.end(); ++iter)
	{
		if(WorldSession* session = iter->getSource()->GetSession())
			session->SendPacket(i_message);
	}
}

template<class T>void
ObjectUpdater::Visit(GridRefManager<T> &m)
{
	for(typename GridRefManager<T>::iterator iter = m.begin(); iter != m.end(); ++iter)
	{
		iter->getSource()->Update(i_timeDiff);
	}
}

template void ObjectUpdater::Visit<GameObject>(GameObjectMapType &);
template void ObjectUpdater::Visit<DynamicObject>(DynamicObjectMapType &);
