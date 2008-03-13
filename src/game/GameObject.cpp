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

#include "Common.h"
#include "GameObject.h"
#include "ObjectMgr.h"
#include "Spell.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "Database/DatabaseEnv.h"
#include "MapManager.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"

GameObject::GameObject( WorldObject *instantiator ) : WorldObject( instantiator )
{
	m_objectType |= TYPE_GAMEOBJECT;
	m_objectTypeId = TYPEID_GAMEOBJECT;

//	m_valuesCount = GAMEOBJECT_END;
}

GameObject::~GameObject()
{
}

void GameObject::AddToWorld()
{
	///- Register the gameobject for guid lookup
	if(!IsInWorld()) ObjectAccessor::Instance().AddObject(this);
	Object::AddToWorld();
}

void GameObject::RemoveFromWorld()
{
	///- Remove the gameobject from the accessor
	if(IsInWorld()) ObjectAccessor::Instance().RemoveObject(this);
	Object::RemoveFromWorld();
}

bool GameObject::Create(uint32 guidlow, uint32 name_id, uint32 mapid, uint16 x, uint16 y, uint32 go_state)
{
	Relocate(x,y);
	SetMapId(mapid);
}

void GameObject::Update(uint32 /*p_time*/)
{
}
