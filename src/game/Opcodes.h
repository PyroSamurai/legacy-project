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
enum OpCodes
{
	MSG_NULL_ACTION                                           = 0,
	MSG_AUTH_LOGON                                            = 1,
	SMSG_BROADCAST_INFO                                       = 2,
	SMSG_QUERY_PLAYER                                         = 3,
	SMSG_QUERY_ALL_PLAYER                                     = 4,
	MSG_UNKNOWN_5                                             = 5,
	MSG_PLAYER_MOVE                                           = 6,
	MSG_UNKNOWN_7                                             = 7,
	CMSG_AUTH_SESSION                                         = 8,
	MSG_UNKNOWN_9                                             = 9,
	MSG_UNKNOWN_10                                            = 10,
	MSG_UNKNOWN_11                                            = 11,
	MSG_UNKNOWN_12                                            = 12,
	MSG_UNKNOWN_13                                            = 13,
	MSG_UNKNOWN_14                                            = 14,
	SMSG_LOAD_COMPLETE                                        = 15,
	MSG_UNKNOWN_16                                            = 16,
	SMSG_QUERY_INVENTORY                                      = 17,
	MSG_UNKNOWN_18                                            = 18,
	MSG_UNKNOWN_19                                            = 19,
	SMSG_AUTH_RESPONSE_PROMPT                                 = 20,
	MSG_UNKNOWN_21                                            = 21,
	MSG_UNKNOWN_22                                            = 21,
	MSG_UNKNOWN_23                                            = 21,
	MSG_UNKNOWN_24                                            = 21,
	MSG_UNKNOWN_25                                            = 21,
	MSG_UNKNOWN_26                                            = 21,
	MSG_UNKNOWN_27                                            = 21,
	MSG_UNKNOWN_28                                            = 21,
	MSG_UNKNOWN_29                                            = 21,
	MSG_UNKNOWN_30                                            = 30,
	MSG_UNKNOWN_31                                            = 31,
	MSG_UNKNOWN_32                                            = 32,
	MSG_UNKNOWN_33                                            = 33,
	MSG_UNKNOWN_34                                            = 34,
	MSG_UNKNOWN_35                                            = 35,
	MSG_UNKNOWN_36                                            = 36,
	MSG_UNKNOWN_37                                            = 37,
	MSG_UNKNOWN_38                                            = 38,
	MSG_UNKNOWN_39                                            = 39,
	MSG_UNKNOWN_40                                            = 40,

	MSG_MOVEMENT                                              = 41,
};

// if you add new opcode .. Do NOT forget to change the following define MAX_OPCODE_ID and also add new opcode to table in opcodes.cpp
#define MAX_OPCODE_ID 42

#endif
