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

#include "Opcodes.h"
#include "NameTables.h"

const char* g_clntOpcodeNames[] = 
{
	"CMSG_LOGON_CHALLENGE",
	"CMSG_AUTH_RESPONSE",
	"CMSG_UNKNOWN_2",
	"CMSG_UNKNOWN_3",
	"CMSG_UNKNOWN_4",
	"CMSG_UNKNOWN_5",
	"CMSG_PLAYER_MOVE",
	"CMSG_UNKNOWN_7",
	"CMSG_UNKNOWN_8",
	"CMSG_UNKNOWN_9",
	"CMSG_UNKNOWN_10",
	"CMSG_UNKNOWN_11",
	"CMSG_UNKNOWN_12",
	"CMSG_UNKNOWN_13",
	"CMSG_UNKNOWN_14",
	"CMSG_UNKNOWN_15",
	"CMSG_UNKNOWN_16",
	"CMSG_UNKNOWN_17",
	"CMSG_UNKNOWN_18",
	"CMSG_UNKNOWN_19",
	"CMSG_ENTER_DOOR",
	"CMSG_UNKNOWN_21",
	"CMSG_UNKNOWN_22",
};

const char* g_svrOpcodeNames[] =
{
	"SMSG_NULL_ACTION",
	"SMSG_AUTH_LOGON", 
	"SMSG_BROADCAST_INFO",
	"SMSG_QUERY_PLAYER_VISIBILITY",
	"SMSG_QUERY_ALL_PLAYER_VISIBILITY",
	"SMSG_QUERY_PLAYER_STATS",
	"SMSG_UPDATE_PLAYER_VISIBILITY",
	"SMSG_UNKNOWN_7",
	"SMSG_AUTH_SESSION",
	"SMSG_UNKNOWN_9",
	"SMSG_UNKNOWN_10",
	"SMSG_UNKNOWN_11",
	"SMSG_ENTER_MAP",
	"SMSG_UNKNOWN_13",
	"SMSG_UNKNOWN_14",
	"SMSG_LOAD_COMPLETE",
	"SMSG_UNKNOWN_16",
	"SMSG_QUERY_INVENTORY",
	"SMSG_UNKNOWN_18",
	"SMSG_UNKNOWN_19",
	"SMSG_REQUEST_RESPONSE",
	"SMSG_UNKNOWN_21",
	"SMSG_UNKNOWN_22",
	"SMSG_UNKNOWN_23",
	"SMSG_INIT_PLAYER",
	"SMSG_UNKNOWN_25",
	"SMSG_UNKNOWN_26",
	"SMSG_UNKNOWN_27",
	"SMSG_UNKNOWN_28",
	"SMSG_UNKNOWN_29",
	"SMSG_UNKNOWN_30",
	"SMSG_UNKNOWN_31",
	"SMSG_UNKNOWN_32",
	"SMSG_UNKNOWN_33",
	"SMSG_UNKNOWN_34",
	"SMSG_UNKNOWN_35",
	"SMSG_UNKNOWN_36",
	"SMSG_UNKNOWN_37",
	"SMSG_UNKNOWN_38",
	"SMSG_UNKNOWN_39",
	"SMSG_UNKNOWN_40",

	"SMSG_UNKNOWN_41",
};

