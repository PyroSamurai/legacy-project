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

#ifndef __LEGACY_COMMON_H
#define __LEGACY_COMMON_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "Utilities/HashMap.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <signal.h>

#if PLATFORM == PLATFORM_WIN32
#define STRCASECMP stricmp
#else
#define STRCASECMP strcasecmp
#endif

#include <set>
#include <list>
#include <string>
#include <map>
#include <queue>
#include <sstream>
#include <algorithm>

#include <zthread/FastMutex.h>
#include <zthread/LockedQueue.h>
#include <zthread/Runnable.h>
#include <zthread/Thread.h>

#if PLATFORM == PLATFORM_WIN32
#	define FD_SETSIZE 1024
#	include <winsock2.h>
#	include <ws2tcpip.h>
#else
#	include <sys/types.h>
#	include <sys/ioctl.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <unistd.h>
#	include <netdb.h>
#endif

#if COMPILER == COMPILER_MICROSOFT

#include <float.h>

#define I64FMT "%016I64X"
#define I64FMTD "%I64u"
#define SI64FMTD "%I64d"
#define snprintf _snprintf
#define atoll __atoi64
#define vsnprintf _vsnprintf
#define strdup _strdup
#define finit(X) _finit(X)

#else

#define stricmp strcasecmp
#define strnicmp strncasecmp
#define I64FMT "%016llX"
#define I64FMTD "%llu"
#define SI64FMTD "%lld"

#endif

#define atol(a) strtoul( a, NULL, 10)

#define STRINGIZE(a) #a

#define for if(true) for

enum TimeConstants
{
	MINUTE = 60,
	HOUR   = MINUTE*60,
	DAY    = HOUR*24,
	MONTH  = DAY*30
};

enum AccountTypes
{
	SEC_PLAYER         = 0,
	SEC_MODERATOR      = 1,
	SEC_GAMEMASTER     = 2,
	SEC_ADMINISTRATOR  = 3
};

enum LocaleConstant
{
	LOCALE_ENG = 0, // en_us
	LOCALE_KO  = 1, // ko_kr
	MAX_LOCALE = 2
};

// we always use stdlibc++ std::max/std::min, undefine some not C++ standart defines (Win API and some other platforms)
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifndef M_PI
#define M_PI         3.14159265358979323846
#endif

#endif
