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

const char* g_worldOpcodeNames[] =
{
	"MSG_NULL_ACTION",                                  // MSG_AUTH
	"MSG_AUTH_LOGON",                 // SMSG_AUTH_RESPONSE_PROMPT
	"SMSG_BROADCAST_INFO",
	"SMSG_QUERY_PLAYER",
	"SMSG_QUERY_ALL_PLAYER",
	"MSG_UNKNOWN_5",
	"MSG_PLAYER_MOVE",
	"MSG_UNKNOWN_7",
	"CMSG_AUTH_SESSION",                       // CMSG_AUTH_CHALLENGE
	"MSG_UNKNOWN_9",
	"MSG_UNKNOWN_10",
	"MSG_UNKNOWN_11",
	"MSG_UNKNOWN_12",
	"MSG_UNKNOWN_13",
	"MSG_UNKNOWN_14",
	"SMSG_LOAD_COMPLETE_GUESS",
	"MSG_UNKNOWN_16",
	"SMSG_QUERY_INVENTORY_GUESS",
	"MSG_UNKNOWN_18",
	"MSG_UNKNOWN_19",
	"SMSG_AUTH_RESPONSE_PROMPT",                // SMSG_AUTH_RESPONSE_SUCCESS
	"MSG_UNKNOWN_21",
	"MSG_UNKNOWN_22",
	"MSG_UNKNOWN_23",
	"MSG_UNKNOWN_24",
	"MSG_UNKNOWN_25",
	"MSG_UNKNOWN_26",
	"MSG_UNKNOWN_27",
	"MSG_UNKNOWN_28",
	"MSG_UNKNOWN_29",
	"MSG_UNKNOWN_30",
	"MSG_UNKNOWN_31",
	"MSG_UNKNOWN_32",
	"MSG_UNKNOWN_33",
	"MSG_UNKNOWN_34",
	"MSG_UNKNOWN_35",
	"MSG_UNKNOWN_36",
	"MSG_UNKNOWN_37",
	"MSG_UNKNOWN_38",
	"MSG_UNKNOWN_39",
	"MSG_UNKNOWN_40",

	"MSG_MOVEMENT",                              // MSG_MOVEMENT
};

