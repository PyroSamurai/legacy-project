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
	"CMSG_LOGON_CHALLENGE",                   // 0
	"CMSG_AUTH_RESPONSE",                     // 1
	"CMSG_PLAYER_CHAT_MESSAGE",               // 2
	"CMSG_UNKNOWN_3",                         // 3
	"CMSG_PLAYER_ENTER_DOOR",                 // 4
	"CMSG_UNKNOWN_5",                         // 5
	"CMSG_PLAYER_MOVE",                       // 6
	"CMSG_UNKNOWN_7",                         // 7
	"CMSG_PLAYER_STAT_ADD",                   // 8
	"CMSG_CHAR_CREATE",                       // 9
	"CMSG_UNKNOWN_10",                        // 10
	"CMSG_USE_ITEM_TO_PLAYER",                // 11
	"CMSG_PLAYER_ENTER_MAP_COMPLETED",        // 12
	"CMSG_GROUP_COMMAND",                     // 13
	"CMSG_UNKNOWN_14",                        // 14
	"CMSG_PET_COMMAND_RELEASE",               // 15
	"CMSG_UNKNOWN_16",                        // 16
	"CMSG_USE_ITEM_TO_PET",                   // 17
	"CMSG_UNKNOWN_18",                        // 18
	"CMSG_PET_COMMAND",                       // 19
	"CMSG_PLAYER_ACTION",                     // 20
	"CMSG_UNKNOWN_21",                        // 21
	"CMSG_UNKNOWN_22",                        // 22
	"CMSG_USE_INVENTORY_ITEM",                // 23
	"CMSG_UNKNOWN_24",                        // 24
	"CMSG_UNKNOWN_25",                        // 25
	"CMSG_UNKNOWN_26",                        // 26
	"CMSG_PLAYER_TRANSAC_ITEM",               // 27
	"CMSG_PLAYER_SPELL_ADD",                  // 28
	"CMSG_UNKNOWN_29",                        // 29
	"CMSG_UNKNOWN_30",                        // 30
	"CMSG_UNKNOWN_31",                        // 31
	"CMSG_PLAYER_EXPRESSION",                 // 32
	"CMSG_UNKNOWN_33",                        // 33
	"CMSG_UNKNOWN_34",                        // 34
	"CMSG_UNKNOWN_35",                        // 35
	"CMSG_UNKNOWN_36",                        // 36
	"CMSG_UNKNOWN_37",                        // 37
	"CMSG_UNKNOWN_38",                        // 38
	"CMSG_UNKNOWN_39",                        // 39
	"CMSG_UNKNOWN_40",                        // 40
	"CMSG_UNKNOWN_41",                        // 41
	"CMSG_UNKNOWN_42",                        // 42
	"CMSG_UNKNOWN_43",                        // 43
	"CMSG_UNKNOWN_44",                        // 44
	"CMSG_UNKNOWN_45",                        // 45
	"CMSG_UNKNOWN_46",                        // 46
	"CMSG_UNKNOWN_47",                        // 47
	"CMSG_UNKNOWN_48",                        // 48
	"CMSG_UNKNOWN_49",                        // 49
	"CMSG_PLAYER_ATTACK",                     // 50
};

const char* g_svrOpcodeNames[] =
{
	"SMSG_NULL_ACTION",                       // 0
	"SMSG_AUTH_LOGON", 
	"SMSG_BROADCAST_INFO",
	"SMSG_QUERY_PLAYER_VISIBILITY",
	"SMSG_QUERY_ALL_PLAYER_VISIBILITY",
	"SMSG_QUERY_PLAYER_STATS",
	"SMSG_UPDATE_PLAYER_VISIBILITY",
	"SMSG_SUBOPCODE_RESPONSE_MAP_ENTER",
	"SMSG_RESPONSE_COMPLETED",
	"SMSG_UNKNOWN_9",
	"SMSG_UNKNOWN_10",
	"SMSG_UNKNOWN_11",
	"SMSG_RESPONSE_PLAYER_RELOCATION",
	"SMSG_UNKNOWN_13",
	"SMSG_ACTION_RESPONSE",
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
	"SMSG_RESPONSE_ENTER_MAP_ACK",
};

const char* g_elementNames[] =
{
	"HEART",
	"EARTH",
	"WATER",
	"FIRE",
	"WIND",
	"NONE",
};

