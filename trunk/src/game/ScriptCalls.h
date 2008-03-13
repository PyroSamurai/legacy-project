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

#ifndef __LEGACY_SCRIPTCALLS_H
#define __LEGACY_SCRIPTCALLS_H

#include "Common.h"

class Creature;
class CreatureAI;
class GameObject;
class Item;
class Player;
class Quest;
class SpellCastTargets;
class Map;

bool LoadScriptingModule(char const* libName = "");
void UnloadScriptingModule();

typedef void(LEGACY_IMPORT * scriptCallScriptsInit) ();
typedef void(LEGACY_IMPORT * scriptCallScriptsFree) ();

typedef bool(LEGACY_IMPORT * scriptCallGossipHello) (Player *player, Creature *_Creature);
typedef bool(LEGACY_IMPORT * scriptCallQuestAccept) (Player *player, Creature *_Creature, Quest const *);
typedef bool(LEGACY_IMPORT * scriptCallGossipSelect)(Player *player, Creature *_Creature, uint32 sender, uint32 action);
typedef bool(LEGACY_IMPORT * scriptCallGossipSelectWithCode)(Player *player, Creature *_Creature, uint32 sender, uint32 action, const char* sCode);
typedef bool(LEGACY_IMPORT * scriptCallQuestSelect) (Player *player, Creature *_Creature, Quest const* );
typedef bool(LEGACY_IMPORT * scriptCallQuestComplete)(Player *player, Creature *_Creature, Quest const*);
typedef uint32(LEGACY_IMPORT * scriptCallNPCDialogStatus)( Player *player, Creature *_Creature);
typedef uint32(LEGACY_IMPORT * scriptCallGODialogStatus)( Player *player, GameObject * _GO);
typedef bool(LEGACY_IMPORT * scriptCallChooseReward)(Player *player, Creature *_Creature, Quest const*, uint32 opt);
typedef bool(LEGACY_IMPORT * scriptCallItemHello)(Player *player, Item *, Quest const*);
typedef bool(LEGACY_IMPORT * scriptCallGOHello)(Player *player, GameObject*);
typedef bool(LEGACY_IMPORT * scriptCallAreaTrigger)(Player *player, Quest const*, uint32 triggerID);
typedef bool(LEGACY_IMPORT * scriptCallItemQuestAccept)(Player *player, Item *, Quest const*);
typedef bool(LEGACY_IMPORT * scriptCallGOQuestAccept)(Player *player, Item *, Quest const*);
typedef bool(LEGACY_IMPORT * scriptCallGOChooseReward)(Player *player, GameObject *, Quest const*, uint32 opt);
typedef bool(LEGACY_IMPORT * scriptCallReceiveEmote)(Player *player, Creature *_Creature, uint32 emote);
typedef bool(LEGACY_IMPORT * scriptCallItemUse)(Player *player, Item *_Item, SpellCastTargets const& targets);
typedef CreatureAI* (LEGACY_IMPORT * scriptCallGetAI)(Creature *_Creature);
//typedef InstanceData* (LEGACY_IMPORT * scriptCallCreateInstanceData)(Map *map);

typedef struct
{
	scriptCallScriptsInit ScriptsInit;
	scriptCallScriptsFree ScriptsFree;

	scriptCallGossipHello GossipHello;
	scriptCallGOChooseReward GOChooseReward;
	scriptCallQuestAccept QuestAccept;
	scriptCallGossipSelect GossipSelect;
	scriptCallGossipSelectWithCode GossipSelectWithCode;
	scriptCallQuestSelect QuestSelect;
	scriptCallQuestComplete QuestComplete;
	scriptCallNPCDialogStatus NPCDialogStatus;
	scriptCallGODialogStatus GODialogStatus;
	scriptCallChooseReward ChooseReward;
	scriptCallItemHello ItemHello;
	scriptCallGOHello GOHello;
	scriptCallAreaTrigger scriptAreaTrigger;
	scriptCallItemQuestAccept ItemQuestAccept;
	scriptCallGOQuestAccept GOQuestAccept;
	scriptCallReceiveEmote ReceiveEmote;
	scriptCallItemUse ItemUse;
	scriptCallGetAI GetAI;
	//scriptCallCreateInstanceData CreateInstanceData;
		
	LEGACY_LIBRARY_HANDLE hScriptsLib;
}_ScriptSet,*ScriptsSet;

extern ScriptsSet Script;

#endif
