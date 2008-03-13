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

#ifndef __LEGACY_QUEST_H
#define __LEGACY_QUEST_H

#include "Platform/Define.h"
#include "Database/DatabaseEnv.h"

#include <string>
#include <vector>

class Player;

class ObjectMgr;

#define MAX_QUEST_LOG_SIZE 25

#define QUEST_OBJECTIVES_COUNT 4

enum QuestFailedReasons
{
	INVALIDREASONS_DONT_HAVE_REQ       = 0
};

enum QuestStatus
{
	QUEST_STATUS_NONE                  = 0,
	QUEST_STATUS_COMPLETE              = 1,
	QUEST_STATUS_UNAVAILABLE           = 2,
	QUEST_STATUS_INCOMPLETE            = 3,
	QUEST_STATUS_AVAIABLE              = 4,
	MAX_QUEST_STATUS
};

enum __QuestGiverStatus
{
	DIALOG_STATUS_NONE                 = 0
};



// This Quest class provides a convenient way to access a few pretotaled (cached) quest details,
// all base quest information, and any utility functions such as generating the amount of xp to give
class Quest
{
	friend class ObjectMgr;
	public:
		Quest(Field * questRecord) {}

};

#endif

