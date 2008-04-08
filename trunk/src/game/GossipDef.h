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
//#include "Creature.h"
//#include "WorldPacket.h"

class WorldSession;

#define GOSSIP_MAX_MENU_ITEMS 64

#define DEFAULT_GOSSIP_MESSAGE              10051

enum GossipType
{
	GOSSIP_TYPE_PLAIN                  = 0x01,
	GOSSIP_TYPE_INVENTORY              = 0x02,
	GOSSIP_TYPE_SELECT                 = 0x06,
};

enum GossipOption
{
	GOSSIP_OPTION_START                = 0x1D,
	GOSSIP_OPTION_1                    = 0x1E,
	GOSSIP_OPTION_2                    = 0x1F,
	GOSSIP_OPTION_3                    = 0x20,
	GOSSIP_OPTION_4                    = 0x21,
	GOSSIP_OPTION_5                    = 0x22,
	GOSSIP_OPTION_6                    = 0x23,
	GOSSIP_OPTION_7                    = 0x24,
	GOSSIP_OPTION_8                    = 0x25,
	GOSSIP_OPTION_9                    = 0x26,
	GOSSIP_OPTION_10                   = 0x27,
	GOSSIP_OPTION_END                  = 0x28,
};

struct GossipItem
{
	GossipItem() : TextId(0), Type(GOSSIP_TYPE_PLAIN) {}

	uint16 TextId;
	uint8  Type;
};

//typedef GossipItem Gossip;
/*
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
*/






class LEGACY_DLL_SPEC PlayerMenu
{
	private:
//		GossipMenu*   pGossipMenu;
//		QuestMenu*    pQuestMenu;
		WorldSession* pSession;

	public:
		PlayerMenu( WorldSession *Session );
		~PlayerMenu();

		/* Required by ScriptCalls */
		void ClearMenus();
		void SendGossipMenu( uint8 map_npcid, uint16 textId );

		void InitTalking();
		void SendTalking( uint8 map_npcid, uint16 textId, uint8 gossip_type = GOSSIP_TYPE_PLAIN );
		bool IsMenuOpened() { return m_menuOpen; }
		void CloseMenu() { m_menuOpen = false; }

		void SendMenu( uint8 map_npcid, uint16 textId );
		void SendSellMenu( uint8 map_npcid, uint16 textId );

	private:
		bool m_menuOpen;
};

#endif
