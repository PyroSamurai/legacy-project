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

#ifndef __LEGACY_SYSTEMCONFIG_H
#define __LEGACY_SYSTEMCONFIG_H

#include "Platform/CompilerDefs.h"

#ifndef _VERSION
#if PLATFORM == PLATFORM_WIN32
#	define _VERSION "0.6.0"
#else
#	define _VERSION "0.0.0-ep5-SVN"
#endif
#endif

// Format is YYYYMMDDRR where RR is the change in the conf file
// for that day.
#ifndef _LEGACYDCONFVERSION
#define _LEGACYDCONFVERSION 2008040601
#endif

#if PLATFORM == PLATFORM_WIN32
#	define _FULLVERSION "/" _VERSION " (Win32)"
#	define _LEGACYD_CONFIG "legacyd.conf"
#else
#	define _FULLVERSION "/" _VERSION "( Unix)"
#	define _LEGACYD_CONFIG "legacyd.conf"
#endif

#define DEFAULT_LOG_LEVEL 0
#define DEFAULT_PLAYER_LIMIT 100
#define DEFAULT_WORLDSERVER_PORT 6414
#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_XP_RATE 1
#define DEFAULT_DROP_RATE 1
#define DEFAULT_SOCKET_SELECT_TIME 10000

#endif
