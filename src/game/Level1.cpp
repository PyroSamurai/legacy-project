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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"
#include "Log.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "NameTables.h"

bool ChatHandler::HandleLookupAreaCommand(const char* args)
{
	sLog.outDebug("COMMAND: HandleLookupAreaCommand");
	return false;
}

bool ChatHandler::HandleLookupNpcCommand(const char* args)
{
	if( !args || !m_session )
		return false;

	// .lookup npc <name>
	sLog.outDebug("COMMAND: HandleLookupNpcCommand");

	std::string npcname = args;
	WorldDatabase.escape_string(npcname);

	QueryResult *result = WorldDatabase.PQuery("SELECT entry, modelid, name, level, element, npcflag FROM creature_template where name "_LIKE_" '""%%%s%%%""' ORDER BY name, entry", npcname.c_str());

	if( !result )
	{
		PSendGmMessage("Npc '%s' not found.", npcname.c_str());
		return true;
	}

	do
	{
		Field *f = result->Fetch();
		uint16 entry     = f[0].GetUInt16();
		uint16 modelid   = f[1].GetUInt16();
		std::string name = f[2].GetCppString();
		uint8  lvl       = f[3].GetUInt8();
		uint8  el        = f[4].GetUInt8();
		uint32 npcflag   = f[5].GetUInt32();
		PSendGmMessage("Npc lvl %3u, el %5s, entry %4u, npcid %5u, flag %u, name '%s'", lvl, LookupNameElement(el, g_elementNames), entry, modelid, npcflag, name.c_str());
	} while (result->NextRow());

	delete result;
	return true;
}

bool ChatHandler::HandleLookupItemCommand(const char* args)
{
	if( !args || !m_session )
		return false;

	// .lookup item <name>
	sLog.outDebug("COMMAND: HandleLookupItemCommand");

	std::string itemname = args;
	WorldDatabase.escape_string(itemname);

	QueryResult *result = WorldDatabase.PQuery("SELECT modelid, name, type, level, attribute, slot FROM item_template WHERE name "_LIKE_" '""%%%s%%%""' ORDER BY name, entry", itemname.c_str());

	if( !result )
	{
		PSendGmMessage("Item '%s' not found.", itemname.c_str());
		return true;
	}

	do
	{
		Field *f = result->Fetch();
		uint16 modelid           = f[0].GetUInt16();
		std::string name         = f[1].GetCppString();
		std::string type         = f[2].GetCppString();
		uint8 level              = f[3].GetUInt8();
		std::string attribute    = f[4].GetCppString();
		std::string slot         = f[5].GetCppString();
		PSendGmMessage("Item lvl %3u, itemid %5u, attr [%s], name '%s'", level, modelid, (!attribute.empty() ? attribute.c_str() : "-"), name.c_str());
	} while (result->NextRow());

	delete result;
	return true;
		
}

bool ChatHandler::HandleModifyGoldCommand(const char* args)
{
	sLog.outDebug("COMMAND: HandleModifyGoldCommand");
	return false;
}
