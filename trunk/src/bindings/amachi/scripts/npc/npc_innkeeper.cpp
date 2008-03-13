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
//#include "../../../../game/GossipDef.h"

bool GossipHello_npc_innkeeper(Player *player, Creature *_Creature)
{
	outstring_log("NPC INNKEEPER HELLO\n");
	player->TalkedToCreature(_Creature->GetEntry(),_Creature->GetGUIDLow());
	_Creature->prepareGossipMenu(player, 0);

	_Creature->sendPreparedGossip( player );
	return true;
}

bool GossipSelect_npc_innkeeper(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
	return true;
}

void AddSC_npc_innkeeper()
{
	Script *newscript;
	newscript = new Script;
	newscript->Name = "npc_innkeeper";
	newscript->pGossipHello = &GossipHello_npc_innkeeper;
	newscript->pGossipSelect = &GossipSelect_npc_innkeeper;
	m_scripts[nrscripts++] = newscript;
}
