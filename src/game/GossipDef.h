/*
 * Copyright (C) 2008-2008 LeGACY <http://code.google.com/p/legacy-project/>
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

#ifndef __LEGACY_GOSSIPDEF_H
#define __LEGACY_GOSSIPDEF_H

#include "Common.h"
#include "NPCHandler.h"

class WorldSession;

#define GOSSIP_MAX_MENU_ITEMS 64

#define DEFAULT_GOSSIP_MESSAGE              10051

struct GossipMenuItem
{
	uint8       m_gIcon;
	bool        m_gCoded;
	std::string m_gMessage;
	uint32      m_gSender;
	uint32      m_gAction;
	std::string m_gBoxMessage;
	uint32      m_gBoxMoney;
};

typedef std::vector<GossipMenuItem> GossipMenuItemList;

struct QuestMenuItem
{
	uint32      m_qId;
	uint8       m_qIcon;
};

typedef std::vector<QuestMenuItem> QuestMenuItemList;

class LEGACY_DLL_SPEC GossipMenu
{
	public:
		GossipMenu() {}
		~GossipMenu() {}

		void ClearMenu() {}

	protected:
		GossipMenuItemList m_gItems;
};







class LEGACY_DLL_SPEC PlayerMenu
{
	private:
		GossipMenu*   pGossipMenu;
//		QuestMenu*    pQuestMenu;
		WorldSession* pSession;

	public:
		PlayerMenu( WorldSession *Session );
		~PlayerMenu();


		void ClearMenus();

		void SendGossipMenu( uint16 textId, uint8 mapNpcId );
		void CloseGossip();
		void SendTalking( uint16 textId, uint8 mapNpcId );
};

#endif
