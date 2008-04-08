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

bool GossipHello_npc_grocery(Player *player, Creature *_Creature)
{
	outstring_log("NPC GROCERY HELLO\n");
	player->TalkedToCreature(_Creature->GetEntry(),_Creature->GetGUID());
	WorldPacket data;
	data.Initialize( 0x06 );
	data << (uint8 ) 0x02;
	player->GetSession()->SendPacket(&data);

	uint8 menu_type = 0;
	if( _Creature->isVendorBlacksmith() )
		menu_type = 0x04;
	///- 0x03 for pedagang buy/sell menu
	///- 0x04 for grocery buy/sell menu
	uint8 map_npcid = _Creature->GetMapNpcId();
	player->PlayerTalkClass->SendBuySellMenu(map_npcid, 0x03);
	return true;
}

bool GossipSelect_npc_grocery(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
	outstring_log("NPC GROCERY SELECT DIALOG\n");
	player->TalkedToCreature(_Creature->GetEntry(), _Creature->GetGUID());
	WorldPacket data;
	player->GetSession()->SetLogging(true);
	switch( action )
	{
		case 0x1E:   // option 1, buy from vendor
			data.Initialize( 0x14 );
			data << (uint8 ) 0x01;
			data << (uint16) 0x0000;
			data << (uint16) 0x0100; // 0100
			data << (uint16) 0x0306; // 0306
			data << (uint8 ) _Creature->GetMapNpcId();
			data << (uint8 ) 0x00;
			data << (uint16) 0x0000;
			data << (uint16) 0x0000;
			data << (uint8 ) 0x00;
			data << (uint8 ) 0x01; // 0x01 for buying menu !!!!
			data << (uint8 ) 0x00;
			player->GetSession()->SendPacket(&data);
			break;

		case 0x1F:   // option 2, sell to vendor
			///- Open up inventory sell dialog
			data.Initialize( 0x1B );
			data << (uint8 ) 0x03; // 03;
			player->GetSession()->SendPacket(&data);

			data.Initialize( 0x14 );
			data << (uint8 ) 0x09;
			player->GetSession()->SendPacket(&data);
			break;
	}

	player->GetSession()->SetLogging(false);
	return true;
}

void AddSC_npc_grocery()
{
	Script *newscript;
	newscript = new Script;
	newscript->Name = "npc_grocery";
	newscript->pGossipHello = &GossipHello_npc_grocery;
	newscript->pGossipSelect = &GossipSelect_npc_grocery;
	m_scripts[nrscripts++] = newscript;
}
