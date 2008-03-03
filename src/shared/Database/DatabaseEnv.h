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

#ifndef __LEGACY_DATABASEENV_H
#define __LEGACY_DATABASEENV_H

#include "Common.h"
#include "Log.h"
#include "Errors.h"

#include "Database/Field.h"
#include "Database/QueryResult.h"

#include "Database/QueryResultMysql.h"
#include "Database/Database.h"
#include "Database/DatabaseMysql.h"
extern DatabaseMysql WorldDatabase;
extern DatabaseMysql CharacterDatabase;
extern DatabaseMysql loginDatabase;

#define _LIKE_ "LIKE"
#define _TABLE_SIM_ "`"

#endif
