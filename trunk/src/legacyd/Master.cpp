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

#include "Master.h"
#include "sockets/SocketHandler.h"
#include "sockets/ListenSocket.h"
#include "WorldSocket.h"
#include "WorldSocketMgr.h"
#include "WorldRunnable.h"
#include "World.h"
#include "Log.h"
#include "Timer.h"
#include <signal.h>
#include "Policies/SingletonImp.h"
#include "SystemConfig.h"
#include "Config/ConfigEnv.h"
#include "Database/DatabaseEnv.h"
#include "Util.h"

#include "sockets/TcpSocket.h"
#include "sockets/Utility.h"
#include "sockets/Parse.h"
#include "sockets/Socket.h"

#ifdef WIN32
#include "ServiceWin32.h"
exterm int m_ServiceStatus;
#endif

//#ifdef ENABLE_CLI
//#include "CliRunnable.h"

//INSTANTIATE_SINGLETON_1( CliRunnable );
//#endif

/*
#ifdef ENABLE_RA
#include "RASocket.h"
#endif
*/

INSTANTIATE_SINGLETON_1( Master );

Master::Master()
{
}

Master::~Master()
{
}


/// Main function
void Master::Run()
{
	sLog.outString( "LeGACY daemon %s", _FULLVERSION );
	sLog.outString( "<Ctrl-C> to stop.\n" );
/*
8          8""""8 8""""8 8""""8 8    8 
8     eeee 8    " 8    8 8    " 8    8 
8e    8    8e     8eeee8 8e     8eeee8 
88    8eee 88  ee 88   8 88       88   
88    88   88   8 88   8 88   e   88   
88eee 88ee 88eee8 88   8 88eee8   88   
 */
	sLog.outString(" 8          8\"\"\"\"8 8\"\"\"\"8 8\"\"\"\"8 8    8");
	sLog.outString(" 8     eeee 8    \" 8    8 8    \" 8    8");
	sLog.outString(" 8e    8    8e     8eeee8 8e     8eeee8");
	sLog.outString(" 88    8eee 88  ee 88   8 88       88");
	sLog.outString(" 88    88   88   8 88   8 88   e   88");
	sLog.outString(" 88eee 88ee 88eee8 88   8 88eee8   88"); 
	sLog.outString( "\n");

	///- Start the databases
	if (!_StartDB())
		return;

	///- Initialize the World
	sWorld.SetInitialWorldSettings();

	///- Launch the world listener socket
	port_t wsport = sWorld.getConfig(CONFIG_PORT_WORLD);
	//port_t wsport = 6414;

	SocketHandler h;
	ListenSocket<WorldSocket> worldListenSocket(h);
	if (worldListenSocket.Bind(wsport))
	{
		clearOnlineAccounts();
		sLog.outError("LeGACY cannot bind to port %d", wsport);
		return;
	}
	sLog.outString("Listening in %d", wsport);

	h.Add(&worldListenSocket);

	///- Catch termination signals
	_HookSignals();

	///- Launch WorldRunnable thread
	ZThread::Thread t(new WorldRunnable);
	t.setPriority ((ZThread::Priority)2);

	#ifdef ENABLE_CLI
	///- Launch CliRunnable thread
	//ZThread::Thread td1(new CliRunnable);
	#endif

	#ifdef ENABLE_RA
	///- Launch the RA listener socket
	//port_t raport = sConfig.GetIntDefault( "RA.Port", 3443 );
	//ListenSocket<RASocket> RAListenSocket(h);

	/*
	if (RAListenSocket.Bind(raport))
	{
		sLog.outError( "LeGACY RA can not bind to port %d", raport );
		// return; // go on with no RA
	}

	h.Add(&RAListenSocket);
	*/
	#endif

	uint32 realCurrTime, realPrevTime;
	realCurrTime = realPrevTime = getMSTime();

	uint32 socketSelecttime = sWorld.getConfig(CONFIG_SOCKET_SELECTTIME);

	uint32 numLoops = (sConfig.GetIntDefault( "MaxPingTime", 30 ) * (MINUTE * 10000000 / socketSelecttime));
	uint32 loopCounter = 0;

	sLog.outString("Ready...");

	///- Wait for termination signal
	while (!World::m_stopEvent)
	{
		if (realPrevTime > realCurrTime)
			realPrevTime = 0;

		realCurrTime = getMSTime();
		sWorldSocketMgr.Update( realCurrTime - realPrevTime );
		realPrevTime = realCurrTime;

		h.Select(0, socketSelecttime);

		// ping if need
		if( (++loopCounter) == numLoops )
		{
			loopCounter = 0;
			sLog.outDetail("Ping MySQL to keep connection alive");
			//delete WorldDatabase.Query("SELECT 1 FROM accounts LIMIT 1");
			delete loginDatabase.Query("SELECT 1 FROM accounts LIMIT 1");
			delete CharacterDatabase.Query("SELECT 1 FROM accounts LIMIT 1");
		}

		
	}

	///- Remove signal handling before leaving
	_UnhookSignals();

	t.wait();

	///- Clean database before leaving
	clearOnlineAccounts();

	///- Wait for delay threads to end
	CharacterDatabase.HaltDelayThread();
	//WorldDatabase.HaltDelayThread();
	loginDatabase.HaltDelayThread();

	sLog.outString( "Halting process..." );

	return;
}

/// Initialize connection to the databases
bool Master::_StartDB()
{
	///- Get world database info from configuration file
	std::string dbstring;
	if(!sConfig.GetString("WorldDatabaseInfo", &dbstring))
	{
		sLog.outError("Database not specified in configuration file");
		return false;
	}
	sLog.outString("World Database: %s", dbstring.c_str());

	///- Initialize the world database

	sLog.outString("Connecting to world database...");
	if(!WorldDatabase.Initialize(dbstring.c_str())){
		sLog.outError("Cannot connect to world database %s", dbstring.c_str());
		return false;
	}

	if(!sConfig.GetString("CharacterDatabaseInfo", &dbstring))
	{
		sLog.outError("Character Database not specified in configuration file");
		return false;
	}
	sLog.outString("Character Database: %s", dbstring.c_str());

	///- Initialize the Character database
	if(!CharacterDatabase.Initialize(dbstring.c_str()))
	{
		sLog.outError("Cannot connect to Character database %s", dbstring.c_str());
		return false;
	}

	///- Initialize the login database
	sLog.outString("Connecting to login database...");
	if(!loginDatabase.Initialize(dbstring.c_str())){
		sLog.outError("Cannot connect to login database %s", dbstring.c_str());
		return false;
	}

	clearOnlineAccounts();

	return true;
}

/// Clear 'online' status for all accounts with characters in this realm
void Master::clearOnlineAccounts()
{
	loginDatabase.PExecute("UPDATE accounts SET online = 0 WHERE online = 1");
	CharacterDatabase.PExecute("UPDATE characters SET online = 0 WHERE online = 1");
}

/// Handle termination signals
/** Put the World::m_stopEvent to 'true' if a termination signal is caught **/
void Master::_OnSignal(int s)
{
	switch (s)
	{
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
		case SIGABRT:
		#ifdef _WIN32
		case SIGBREAK:
		#endif
			World::m_stopEvent = true;
		break;
	}

	signal(s, _OnSignal);
}

/// Define hook '_OnSignal' for all termination signals
void Master::_HookSignals()
{
	signal(SIGINT, _OnSignal);
	signal(SIGQUIT, _OnSignal);
	signal(SIGTERM, _OnSignal);
	signal(SIGABRT, _OnSignal);
	#ifdef _WIN32
	signal(SIGBREAK, _OnSignal);
	#endif
}

/// Unhook the signals before leaving
void Master::_UnhookSignals()
{
	signal(SIGINT, 0);
	signal(SIGQUIT, 0);
	signal(SIGTERM, 0);
	signal(SIGABRT, 0);
	#ifdef _WIN32
	signal(SIGBREAK, 0);
	#endif
}






