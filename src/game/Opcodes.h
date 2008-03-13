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

#ifndef __LEGACY_OPCODES_H
#define __LEGACY_OPCODES_H

/// List of Opcodes
enum ClientOpcodes
{
	CMSG_LOGON_CHALLENGE                                      = 0,
	CMSG_AUTH_RESPONSE                                        = 1,
	CMSG_PLAYER_CHAT_MESSAGE                                  = 2,
	CMSG_UNKNOWN_3                                            = 3,
	CMSG_PLAYER_ENTER_DOOR                                    = 4,
	CMSG_UNKNOWN_5                                            = 5,
	CMSG_PLAYER_MOVE        /* -------------------> */        = 6,
	CMSG_UNKNOWN_7                                            = 7,
	CMSG_PLAYER_ENTER_DOOR2                                   = 8,
	CMSG_UNKNOWN_9                                            = 9,
	CMSG_UNKNOWN_10                                           = 10,
	CMSG_UNKNOWN_11                                           = 11,
	CMSG_PLAYER_ENTER_MAP_COMPLETED                           = 12,
	CMSG_UNKNOWN_13                                           = 13,
	CMSG_UNKNOWN_14                                           = 14,
	CMSG_UNKNOWN_15                                           = 15,
	CMSG_UNKNOWN_16                                           = 16,
	CMSG_UNKNOWN_17                                           = 17,
	CMSG_UNKNOWN_18                                           = 18,
	CMSG_UNKNOWN_19                                           = 19,
	CMSG_PLAYER_ACTION /* -----------------------------> */   = 20,
	CMSG_UNKNOWN_21                                           = 21,
	CMSG_UNKNOWN_22                                           = 22,
	CMSG_UNKNOWN_23                                           = 23,
	CMSG_UNKNOWN_24                                           = 24,
	CMSG_UNKNOWN_25                                           = 25,
	CMSG_UNKNOWN_26                                           = 26,
	CMSG_UNKNOWN_27                                           = 27,
	CMSG_UNKNOWN_28                                           = 28,
	CMSG_UNKNOWN_29                                           = 29,
	CMSG_UNKNOWN_30                                           = 30,
	CMSG_UNKNOWN_31                                           = 31,
	CMSG_PLAYER_EXPRESSION                                    = 32,
};
// if you add new server opcode .. Do NOT forget to change the following define MAX_SVR_OPCODE_ID and also add new client opcode to table in opcodes.cpp
#define MAX_CLNT_OPCODE_ID 33

enum OpCodes
{
	SMSG_NULL_ACTION                                          = 0,
	SMSG_AUTH_LOGON                                           = 1,
	SMSG_BROADCAST_INFO                                       = 2,
	SMSG_QUERY_PLAYER_VISIBILITY                              = 3,
	SMSG_QUERY_ALL_PLAYER_VISIBILITY                          = 4,
	SMSG_QUERY_PLAYER_STATS                                   = 5,
	SMSG_UPDATE_PLAYER_VISIBILITY                             = 6,
	SMSG_SUBOPCODE_RESPONSE_MAP_ENTER                         = 7,
	SMSG_REPONSE_COMPLETED                                    = 8,
	SMSG_UNKNOWN_9                                            = 9,
	SMSG_UNKNOWN_10                                           = 10,
	SMSG_UNKNOWN_11                                           = 11,
	SMSG_RESPONSE_PLAYER_RELOCATION                           = 12,
	SMSG_UNKNOWN_13                                           = 13,
	SMSG_ACTION_RESPONSE                                      = 14,
	SMSG_LOAD_COMPLETE                                        = 15,
	SMSG_UNKNOWN_16                                           = 16,
	SMSG_QUERY_INVENTORY                                      = 17,
	SMSG_UNKNOWN_18                                           = 18,
	SMSG_UNKNOWN_19                                           = 19,
	SMSG_REQUEST_RESPONSE                                     = 20,
	SMSG_UNKNOWN_21                                           = 21,
	SMSG_UNKNOWN_22                                           = 22,
	SMSG_UNKNOWN_23                                           = 23,
	SMSG_INIT_PLAYER                                          = 24,
	SMSG_UNKNOWN_25                                           = 25,
	SMSG_UNKNOWN_26                                           = 26,
	SMSG_UNKNOWN_27                                           = 27,
	SMSG_UNKNOWN_28                                           = 28,
	SMSG_UNKNOWN_29                                           = 29,
	SMSG_UNKNOWN_30                                           = 31,
	SMSG_UNKNOWN_31                                           = 32,
	SMSG_UNKNOWN_32                                           = 33,
	SMSG_UNKNOWN_33                                           = 34,
	SMSG_UNKNOWN_34                                           = 35,
	SMSG_UNKNOWN_35                                           = 35,
	SMSG_UNKNOWN_36                                           = 36,
	SMSG_UNKNOWN_37                                           = 37,
	SMSG_UNKNOWN_38                                           = 38,
	SMSG_UNKNOWN_40                                           = 39,
	SMSG_RESPONSE_ENTER_MAP_ACK                               = 41,

};

// if you add new server opcode .. Do NOT forget to change the following define MAX_SVR_OPCODE_ID and also add new server opcode to table in opcodes.cpp
#define MAX_SVR_OPCODE_ID 42


/// Results of friend related commands
enum FriendsResult
{
	FRIEND_DB_ERROR                                           = 0x00
};

/// Non Player Character flags
enum NPCFlags
{
	UNIT_NPC_FLAG_NONE                                  = 0x00000000,
	UNIT_NPC_FLAG_GOSSIP                                = 0x00000001
};

#endif
