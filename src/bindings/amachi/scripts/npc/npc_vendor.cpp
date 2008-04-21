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

#include "../sc_defines.h"
#include "../../../../game/Player.h"
#include "../../../../game/GossipDef.h"
#include "WorldPacket.h"

uint16 mn_dialog = 0x00;

bool GossipHello_npc_vendor(Player *player, Creature *_Creature)
{
	outstring_log("SCRIPTS: NPC VENDOR HELLO\n");
	player->TalkedToCreature(_Creature->GetEntry(),_Creature->GetGUID());

	uint16 textId    = _Creature->GetNpcTextId();
	uint8  map_npcid = _Creature->GetMapNpcId();
	player->PlayerTalkClass->SendMenu(map_npcid, textId);
	return true;
}

bool GossipSelect_npc_vendor(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
	if(action == GOSSIP_OPTION_END)
	{
		player->PlayerTalkClass->CloseMenu();
		player->EndOfRequest();
		return true; // prevent legacy core handling
	}
	uint32 sequence = sender;
	outstring_log("SCRIPTS: NPC VENDOR SELECT DIALOG\n");
	GossipItem go = _Creature->GetNpcGossip(sequence, action);

	//if(GOSSIP_TYPE_INVENTORY != go.Type && 0 == go.TextId)
	//	return true; // prevent legacy core handling
/*
	if(0 == go.TextId)
		switch( go.Type )
		{
			case GOSSIP_TYPE_INVENTORY:
			case GOSSIP_TYPE_BANK_INVENTORY:
				// prevent legacy core handling
				return true;

			default:
				break;
		}
*/
	player->PlayerTalkClass->SendTalking(_Creature->GetMapNpcId(), go.TextId, go.Type);
	return true;
}

void AddSC_npc_vendor()
{
	Script *newscript;
	newscript = new Script;
	newscript->Name = "npc_vendor";
	newscript->pGossipHello = &GossipHello_npc_vendor;
	newscript->pGossipSelect = &GossipSelect_npc_vendor;
	m_scripts[nrscripts++] = newscript;
}
