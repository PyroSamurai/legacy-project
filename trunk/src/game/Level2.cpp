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
	if( !args || !m_session )
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

bool ChatHandler::HandleChangeLevelCommand(const char* args)
{
	if( !args || !m_session )
		return false;

	char* namepart = strtok((char*)args, " ");
	char* level_text = strtok(NULL, " ");
	if(!namepart || !level_text)
		return false;

	// .changelevel <player> <level>
	sLog.outDebug("COMMAND: HandleChangeLevelCommand");

	std::string player_name = namepart;
	int32 level = atoi(level_text);

	if( level <= 0 || level > 200)
	{
		PSendGmMessage("Invalid level. Must be between 1 and 200.");
		return true;
	}

	uint32 guid = objmgr.GetPlayerGUIDByName(player_name);
	Player* player = objmgr.GetPlayer(guid);
	if( !player )
	{
		PSendGmMessage("Player '%s' not found.", player_name.c_str());
		return true;
	}

	player->SetLevel(level);
	player->UpdatePlayerLevel();

	player->SetUInt32Value(UNIT_FIELD_INT, 0);
	player->SetUInt32Value(UNIT_FIELD_ATK, 0);
	player->SetUInt32Value(UNIT_FIELD_DEF, 0);
	player->SetUInt32Value(UNIT_FIELD_HPX, 0);
	player->SetUInt32Value(UNIT_FIELD_SPX, 0);
	player->SetUInt32Value(UNIT_FIELD_AGI, 0);
	player->SetUInt32Value(UNIT_FIELD_STAT_POINT, level*2+4);

	player->ResetSpells();
	player->SetUInt32Value(UNIT_FIELD_SPELL_POINT, level-1);

	WorldPacket data;
	player->BuildUpdateBlockStatusPacket(&data);
	player->GetSession()->SendPacket(&data);

	return false;
}

bool ChatHandler::HandleChangeLevelPetCommand(const char* args)
{
	if( !args || !m_session )
		return false;

	char* namepart = strtok((char*)args, " ");
	char* slot_text = strtok(NULL, " ");
	char* level_text = strtok(NULL, " ");
	if(!namepart || !slot_text || !level_text)
		return false;

	// .changelevelpet <player> <slot> <level>
	sLog.outDebug("COMMAND: HandleChangeLevelPetCommand");

	std::string player_name = namepart;
	uint8 slot = atoi(slot_text);
	int32 level = atoi(level_text);

	if( slot == PET_SLOT_START || slot > MAX_PET_SLOT )
	{
		PSendGmMessage("Slot must be between 1 and 4.");
		return true;
	}
	if( level <= 0 || level > 200 )
	{
		PSendGmMessage("Invalid level. Must be between 1 and 200.");
		return true;
	}

	uint32 player_guid = objmgr.GetPlayerGUIDByName(player_name);
	Player* player = objmgr.GetPlayer(player_guid);
	if( !player )
	{
		PSendGmMessage("Player '%s' not found.", player_name.c_str());
		return true;
	}

	Pet *pet = player->GetPet(slot);
	if( !pet )
	{
		PSendGmMessage("Player '%s' does not have pet in slot %u", player_name.c_str(), slot);
		return true;
	}

	pet->SetLevel(level);
	player->UpdatePetLevel(pet);

	/// TODO: reset stats based on prototype
	/// TODO: add randomly stat in level gained.
	/// TODO: add spell point
}

bool ChatHandler::HandleSaveAllCommand(const char* args)
{
	if( !m_session )
		return false;

	sLog.outDebug("COMMAND: HandleSaveAllCommand");
	return false;
}

bool ChatHandler::HandleNpcAddCommand(const char* args)
{
	if( !args || !m_session )
		return false;

	char* map_npcid_text = strtok((char*)args, " ");
	char* model_text = strtok(NULL, " ");
	char* x_text = strtok(NULL, " ");
	char* y_text = strtok(NULL, " ");

	if( !map_npcid_text || !model_text || !x_text || !y_text )
		return false;

	uint8  map_npcid = atoi(map_npcid_text);
	uint16 modelid   = atoi(model_text);
	uint16 pos_x     = atoi(x_text);
	uint16 pos_y     = atoi(y_text);

	// .npc add <mapnpcid> <entry> <x> <y>
	sLog.outDebug("COMMAND: HandleAddNpcCommand");

	Player* GM = m_session->GetPlayer();

	if( !GM )
		return true;

	uint32 mapid = GM->GetMapId();
	uint32 guid = objmgr.GenerateNpcGuidLow(mapid, map_npcid);

	CreatureData const* npc = objmgr.GetCreatureData(guid);

	if( npc )
	{
		PSendGmMessage("Npc %u exists.", guid);
		return true;
	}

	CreatureInfo const* cinfo = objmgr.GetCreatureTemplateByModelId(modelid);

	if( !cinfo )
	{
		PSendGmMessage("Npc %u prototype %u invalid.", guid, modelid);
		return true;
	}

	WorldDatabase.BeginTransaction();

	WorldDatabase.PExecute("INSERT INTO creature (guid, entry, mapid, map_npcid, pos_x, pos_y, MovementType) VALUES (%u, %u, %u, %u, %u, %u, %u)", guid, cinfo->Entry, mapid, map_npcid, pos_x, pos_y, cinfo->MovementType);

	WorldDatabase.CommitTransaction();

	///- do a small delay to make sure data is inserted
	ZThread::Thread::sleep(10);

	objmgr.LoadCreature(guid);

	Creature* obj = new Creature(NULL);

	if( !obj->LoadFromDB(guid, 0) )
	{
		PSendGmMessage("Npc %u failed to load from database.", guid);
		WorldDatabase.RollbackTransaction();
		delete obj;
		return true;
	}

	Map* map = MapManager::Instance().GetMap(GM->GetMapId(), GM);
	map->Add(obj);
	PSendGmMessage("Npc %u created.", guid);

	return true;
}

bool ChatHandler::HandleNpcDeleteCommand(const char* args)
{
	char* map_npcid_text = strtok((char*)args, " ");

	if( !map_npcid_text )
		return false;

	uint8 map_npcid = atoi(map_npcid_text);

	// .npc del <mapnpcid>
	sLog.outDebug("COMMAND: HandleNpcDeleteCommand");

	Player* GM = m_session->GetPlayer();

	if( !GM )
		return true;

	uint32 mapid = GM->GetMapId();
	uint32 guid = objmgr.GenerateNpcGuidLow(mapid, map_npcid);

	CreatureData const* npc = objmgr.GetCreatureData(guid);

	if( !npc )
	{
		PSendGmMessage("Npc %u does not exists. Cannot delete.", guid);
		return true;
	}

	uint64 GUID = objmgr.GetNpcGuidByMapNpcId(mapid, map_npcid);
	Creature* obj = ObjectAccessor::GetCreature(*GM, GUID);

	if( !obj )
	{
		PSendGmMessage("Npc %u is invalid.", guid);
		return true;
	}

	Map* map = MapManager::Instance().GetMap(GM->GetMapId(), GM);
	map->Remove(obj, false);
	obj->DeleteFromDB();
	PSendGmMessage("Npc '%s' deleted.", obj->GetName());
	delete obj;

	return true;
}

bool ChatHandler::HandleNpcEditCommand(const char* args)
{
	char* cmd = strtok((char*)args, " ");
	char* map_npcid_text = strtok(NULL, " ");

	if( !cmd || !map_npcid_text )
		return false;

	uint8  map_npcid = atoi(map_npcid_text);

	// .npc edit team <mapnpcid> [group1] ... [group10]
	// .npc edit pos  <mapnpcid> idle|random <x> <y>
	sLog.outDebug("COMMAND: HandleNpcEditCommand command: [%s]", cmd);

	Player* GM = m_session->GetPlayer();

	if( !GM )
		return true;

	uint32 mapid = GM->GetMapId();
	uint32 guid = objmgr.GenerateNpcGuidLow(mapid, map_npcid);

	CreatureData const* npc = objmgr.GetCreatureData(guid);

	if( !npc )
	{
		PSendGmMessage("Npc %u does not exists. Please create it first.", guid);
		return true;
	}

	CreatureInfo const* cinfo = objmgr.GetCreatureTemplate(npc->id);

	if( !cinfo )
	{
		PSendGmMessage("Npc %u prototype %u invalid.", guid, npc->id);
		return true;
	}

	if(::strcmp(cmd, "team") == 0)
	{
		sLog.outDebug("Editing Team");
		char* t00_text = strtok(NULL, " ");
		char* t01_text = strtok(NULL, " ");
		char* t03_text = strtok(NULL, " ");
		char* t04_text = strtok(NULL, " ");

		char* t10_text = strtok(NULL, " ");
		char* t11_text = strtok(NULL, " ");
		char* t12_text = strtok(NULL, " ");
		char* t13_text = strtok(NULL, " ");
		char* t14_text = strtok(NULL, " ");

		uint32 team[2][5];

		team[0][0] = (t00_text ? atoi(t00_text) : 0);
		team[0][1] = (t01_text ? atoi(t01_text) : 0);
		team[0][3] = (t03_text ? atoi(t03_text) : 0);
		team[0][4] = (t04_text ? atoi(t04_text) : 0);

		team[1][0] = (t10_text ? atoi(t10_text) : 0);
		team[1][1] = (t11_text ? atoi(t11_text) : 0);
		team[1][2] = (t12_text ? atoi(t12_text) : 0);
		team[1][3] = (t13_text ? atoi(t13_text) : 0);
		team[1][4] = (t14_text ? atoi(t14_text) : 0);

		sLog.outDebug("Update Team pos 0-0 = %u", team[0][0]);
		sLog.outDebug("Update Team pos 0-1 = %u", team[0][1]);
		sLog.outDebug("Update Team pos 0-3 = %u", team[0][3]);
		sLog.outDebug("Update Team pos 0-4 = %u", team[0][4]);

		sLog.outDebug("Update Team pos 1-0 = %u", team[1][0]);
		sLog.outDebug("Update Team pos 1-1 = %u", team[1][1]);
		sLog.outDebug("Update Team pos 1-2 = %u", team[1][2]);
		sLog.outDebug("Update Team pos 1-3 = %u", team[1][3]);
		sLog.outDebug("Update Team pos 1-4 = %u", team[1][4]);

		std::ostringstream ss;
		ss << "UPDATE creature SET ";

		uint8 count = 0;

		for(uint16 i = 0; i < 2; i++)
			for(uint16 j = 0; j < 5; j++)
			{
				count++;

				if(i == 0 && j == 2)
					continue;

				if(i > 0 || j > 0)
					ss << ", ";

				CreatureInfo const* cinfo2;
				cinfo2 = objmgr.GetCreatureTemplateByModelId(team[i][j]);
				if( !cinfo2 && team[i][j] != 0 )
				{
					PSendGmMessage("Npc team %u prototype %u invalid.", count, team[i][j]);
					team[i][j] = 0;
				}

				if( cinfo2 )
					sLog.outDebug("Team %u set %s", count, cinfo2->Name);

				uint32 guid2 = objmgr.GetCreatureGuidByModelId(team[i][j]);

				if( !guid2 && team[i][j] != 0)
				{
					PSendGmMessage("Npc team %u npc %u does not exists. Please create it first.", count, team[i][j]);
					guid2 = 0;
				}

				ss << "team_" << i << "_" << j << " = " << guid2 << " ";
			}

		ss << " WHERE guid = " << guid;

		WorldDatabase.Execute( ss.str().c_str() );

		ZThread::Thread::sleep(10);

		///- Reload the npc
		objmgr.LoadCreature(guid);

		PSendGmMessage("Npc %u battle team is updated.", guid);

		return true;
	}
}

bool ChatHandler::HandleItemAddCommand(const char* args)
{
	if( !args || !m_session )
		return false;

	char* namepart = strtok((char*)args, " ");
	char* model_text = strtok(NULL, " ");
	char* count_text = strtok(NULL, " ");

	if( !namepart || !model_text )
		return false;

	//. item add <player> <model> [count]

	sLog.outDebug("COMMAND: HandleItemAddCommand");

	std::string player_name = namepart;
	uint16 modelid = atoi(model_text);
	uint16 count = count_text ? atoi(count_text) : 1;

	if( count <= 0 ) count = 1;
	if( count > 50 ) count = 50;

	uint32 player_guid = objmgr.GetPlayerGUIDByName(player_name);
	Player* player = objmgr.GetPlayer(player_guid);
	if( !player )
	{
		PSendGmMessage("Player '%s' not found.", player_name.c_str());
		return true;
	}

	uint32 entry = objmgr.GetItemEntryByModelId(modelid);
	ItemPrototype const* pProto = objmgr.GetItemPrototype(entry);

	if( !pProto )
	{
		PSendGmMessage("Item %u is invalid.", modelid);
		return true;
	}

	if( !player->AddNewInventoryItem(entry, count) )
	{
		PSendGmMessage("Failed to add item '%s' to '%s' inventory.", pProto->Name, player->GetName());
		return true;
	}

	PSendGmMessage("Item '%s' added for '%s' to inventory.", pProto->Name, player->GetName());

	///- visualize item for player inventory
	player->UpdateInventoryForItem(modelid, count);

	return true;
}

bool ChatHandler::HandlePetAddCommand(const char* args)
{
	if( !args || !m_session )
		return false;

	char* namepart = strtok((char*)args, " ");
	char* model_text = strtok(NULL, " ");

	if( !namepart || !model_text )
		return false;

	// .pet add <player> <model>

	sLog.outDebug("COMMAND: HandlePetAddCommand");

	std::string player_name = namepart;
	uint16 modelid = atoi(model_text);

	uint32 player_guid = objmgr.GetPlayerGUIDByName(player_name);
	Player* player = objmgr.GetPlayer(player_guid);
	if( !player )
	{
		PSendGmMessage("Player '%s' not found.", player_name.c_str());
		return true;
	}

	CreatureInfo const *cinfo = objmgr.GetCreatureTemplateByModelId(modelid);

	if( !cinfo )
	{
		PSendGmMessage("Pet %u is invalid.", modelid);
		return true;
	}

	Pet* pet = player->CreatePet( cinfo->Entry );
	if( !pet )
	{
		PSendGmMessage("Failed to create pet %u.", modelid); 
		return true;
	}

	uint8 dest;
	if( player->CanSummonPet( NULL_PET_SLOT, dest, pet, false ) == PET_ERR_OK )
	{
		pet->SetLoyalty(60);
		player->SummonPet( dest, pet );
		player->UpdatePetCarried();
		WorldPacket data;
		player->BuildUpdateBlockTeam(&data);
		player->SendMessageToSet(&data, true);
		PSendGmMessage("Pet '%s' created for player '%s'", pet->GetName(), player->GetName());
	}
	else
	{
		PSendGmMessage("Player '%s' can not summon pet '%s'. Slot is full or already have.", player->GetName(), pet->GetName());
		delete pet;
	}

	return true;
}

bool ChatHandler::HandlePetReleaseCommand(const char* args)
{
	if( !args || !m_session )
		return false;

	char* namepart = strtok((char*)args, " ");
	char* slot_text = strtok(NULL, " ");

	if( !namepart )
		return false;

	// .pet release <player> [slot]

	sLog.outDebug("COMMAND: HandlePetReleaseCommand");

	std::string player_name = namepart;
	uint8 slot = (slot_text ? atoi(slot_text) : 1);

	if( slot == PET_SLOT_START || slot > MAX_PET_SLOT )
	{
		PSendGmMessage("Slot must be between 1 and 4.");
		return true;
	}

	uint32 player_guid = objmgr.GetPlayerGUIDByName(player_name);
	Player* player = objmgr.GetPlayer(player_guid);
	if( !player )
	{
		PSendGmMessage("Player '%s' not found.", player_name.c_str());
		return true;
	}

	player->ReleasePet(slot-1);

	PSendGmMessage("Player '%s' pet slot %u released.", player->GetName(), slot);
}

bool ChatHandler::HandleVendorAddItemCommand(const char* args)
{
	if( !args || !m_session )
		return false;

	char* map_npcid_text = strtok((char*)args, " ");
	char* slot_text      = strtok(NULL, " ");
	char* model_text     = strtok(NULL, " ");

	if( !map_npcid_text || !slot_text || !model_text )
		return false;

	uint8  map_npcid = atoi(map_npcid_text);
	uint8  slot      = atoi(slot_text);
	uint32 modelid   = atoi(model_text);

	if( slot == 0 || slot > 25) 
	{
		PSendGmMessage("Vendor item slot must be between 1 - 25");
		return true;
	}

	// .vendor add <map_npcid> <slot> <itemid>
	sLog.outDebug("COMMAND: HandleVendorAddItemGoodsCommand");

	Player* GM = m_session->GetPlayer();

	if( !GM )
		return true;

	uint32 mapid = GM->GetMapId();
	uint64 guid  = objmgr.GetNpcGuidByMapNpcId(mapid, map_npcid);

	Creature* vendor = ObjectAccessor::GetNPCIfCanInteractWith(*GM, guid, UNIT_NPC_FLAG_NONE);

	if( !vendor )
	{
		PSendGmMessage("Vendor %u is not exists. Please create it first.", GUID_LOPART(guid));
		return true;
	}

	if( !vendor->isVendor() )
	{
		PSendGmMessage("Npc '%s' - GUID(%u) flag is not a vendor.", vendor->GetName(), GUID_LOPART(guid));
		return true;
	}

	uint32 entry = objmgr.GetItemEntryByModelId(modelid);

	ItemPrototype const *pProto = objmgr.GetItemPrototype(entry);
	if(!pProto)
	{
		PSendGmMessage("Item %u is invalid.", modelid);
		return true;
	}

	// load vendor items if not yet
	vendor->LoadGoods();

	if(vendor->FindItem(entry))
	{
		PSendGmMessage("Vendor '%s' already have item <%s>", vendor->GetName(), pProto->Name);
		return true;
	}

	if(vendor->GetItem(slot-1))
	{
		PSendGmMessage("Vendor '%s' already have item <%s> in slot %u", vendor->GetName(), objmgr.GetItemPrototype(vendor->GetItem(slot-1)->entry)->Name, slot);
		return true;
	}

	// add to DB and to current ingame vendor
	WorldDatabase.PExecuteLog("INSERT INTO npc_vendor (guid, slot, item) VALUES (%u, %u, %u)", GUID_LOPART(guid), slot-1, entry);

	vendor->AddItem(slot-1, entry);

	PSendGmMessage("Item <%s> added to '%s' vendor goods slot %u", pProto->Name, vendor->GetName(), slot);
	return true;
}

bool ChatHandler::HandleVendorDelItemCommand(const char* args)
{
	if( !args || !m_session )
		return false;

	char* map_npcid_text = strtok((char*)args, " ");
	char* slot_text      = strtok(NULL, " ");

	if( !map_npcid_text || !slot_text )
		return false;

	uint8 map_npcid = atoi(map_npcid_text);
	uint8 slot      = atoi(slot_text);

	if( slot == 0 || slot > 25 )
	{
		PSendGmMessage("Vendor item slot must be between 1 - 25");
		return true;
	}

	// .vendor del <map_npcid> <slot>
	sLog.outDebug("COMMAND: HandleVendorDelItemCommand");

	Player* GM = m_session->GetPlayer();

	if( !GM )
		return true;

	uint32 mapid = GM->GetMapId();
	uint64 guid  = objmgr.GetNpcGuidByMapNpcId(mapid, map_npcid);

	Creature* vendor = ObjectAccessor::GetNPCIfCanInteractWith(*GM, guid, UNIT_NPC_FLAG_NONE);

	if( !vendor )
	{
		PSendGmMessage("Vendor %u is not exists. Please create it first.", GUID_LOPART(guid));
		return true;
	}

	if( !vendor->isVendor() )
	{
		PSendGmMessage("Npc '%s' - GUID(%u) flag is not a vendor.", vendor->GetName(), GUID_LOPART(guid));
		return true;
	}

	// load vendor items if not yet
	vendor->LoadGoods();

	CreatureItem * citem = vendor->GetItem(slot-1);
	if( !citem )
	{
		PSendGmMessage("Vendor '%s' item slot %u is empty.", vendor->GetName(), slot);
		return true;
	}

	!vendor->RemoveItem(citem->entry);

	WorldDatabase.PExecuteLog("DELETE FROM npc_vendor WHERE item = %u AND slot = %u AND guid = %u", citem->entry, slot-1, GUID_LOPART(guid));

	PSendGmMessage("Vendor '%s' item slot %u deleted.", vendor->GetName(), slot);
	return true;

}
