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
#include "Database/DatabaseEnv.h"
#include "Config/ConfigEnv.h"
#include "Log.h"
#include "Master.h"
#include "SystemConfig.h"

#ifdef WIN32
#include "ServiceWin32.h"
char serviceName[] = "legacyd";
char serviceLongName[] = "LeGACY world service";
char serviceDescription[] = "Massive Network Game Object Server";
/*
 * -1 - not in service mode
 *  0 - stopped
 *  1 - running
 *  2 - paused
 */
int m_ServiceStatus = -1;
#endif

#ifdef DO_POSTGRESQL
#else
DatabaseMysql WorldDatabase;     ///< Accessor to the world database
DatabaseMysql CharacterDatabase; ///< Accessor to the character database
DatabaseMysql loginDatabase;     ///< Accessor to the login database
#endif

/// Launch the legacy server
int main(int argc, char **argv)
{
	///- Command line parsing to get the configuration file name
	char const* cfg_file = _LEGACYD_CONFIG;
	if (!sConfig.SetSource(cfg_file))
	{
		sLog.outError("Could not find configuration file %s.", cfg_file);
		return 1;
	}
	sLog.outString("Using configuration file %s.", cfg_file);
	sMaster.Run();
	return 0;
}
