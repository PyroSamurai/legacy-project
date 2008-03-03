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

#ifndef __LEGACY_OBJECTMGR_H
#define __LEGACY_OBJECTMGR_H

#include "Log.h"
#include "Object.h"
#include "GameObject.h"
#include "Player.h"
#include "Policies/Singleton.h"

#include <string>
#include <map>

/// Player state
enum SessionStatus
{
	STATUS_NOT_LOGGEDIN      = 0,  ///< Player not logged in
	STATUS_LOGGEDIN                ///< Player in game
};

struct OpcodeHandler
{
	OpcodeHandler() : status(STATUS_LOGGEDIN), handler(NULL) {};
	OpcodeHandler( SessionStatus _status, void (WorldSession::*_handler)(WorldPacket& recvPacket) ) : status(_status), handler(_handler) {};

	SessionStatus status;
	void (WorldSession::*handler)(WorldPacket& recvPacket);
};

typedef HM_NAMESPACE::hash_map< uint16, OpcodeHandler > OpcodeTableMap;

class ObjectMgr
{
	public:
		ObjectMgr();
		~ObjectMgr();

		OpcodeTableMap opcodeTable;

};

#define objmgr LeGACY::Singleton<ObjectMgr>::Instance()
#endif
