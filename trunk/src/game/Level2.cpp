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

bool ChatHandler::HandleWarpCommand(char const* args)
{
	if(!args)
		return false;

	char* dest_text = strtok((char*)args, " ");
	if(!dest_text)
		return false;

	uint16 dest = atoi(dest_text);

	sLog.outDebug("COMMAND: HandleWarpCommand Warping to %u", dest);
	Player* pPlayer = m_session->GetPlayer();
	pPlayer->TeleportTo(dest, pPlayer->GetPositionX(), pPlayer->GetPositionY());
	pPlayer->SendMapChanged();
	return true;
}
