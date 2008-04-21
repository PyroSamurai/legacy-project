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

#ifndef __LEGACY_SERVERLOG_H
#define __LEGACY_SERVERLOG_H

#include "Common.h"
#include "Policies/Singleton.h"

class Config;

enum Color
{
	BLACK,
	RED,
	GREEN,
	BROWN,
	BLUE,
	MAGENTA,
	CYAN,
	GREY,
	YELLOW,
	LRED,
	LGREEN,
	LBLUE,
	LMAGENTA,
	LCYAN,
	WHITE
};

const int Color_count = int(WHITE)+1;

class Log : public LeGACY::Singleton<Log, LeGACY::ClassLevelLockable<Log, ZThread::FastMutex> >
{
	friend class LeGACY::OperatorNew<Log>;
	Log() : logfile(NULL) { Initialize(); }
	~Log()
	{
		if( logfile != NULL )
			fclose(logfile);
		logfile = NULL;
	}

	public:
		void Initialize();

		void InitColors(std::string init_str);
		void outTitle( const char * str);
		void outCommand( const char * str, ...) ATTR_PRINTF(2,3);
		void outString();                       // any log level
		                                        // any log level
		void outString(const char * str, ...)   ATTR_PRINTF(2,3);
		                                        // any log level
		void outError( const char * str, ...)   ATTR_PRINTF(2,3);
		                                        // log level >= 1
		void outBasic( const char * str, ...)   ATTR_PRINTF(2,3);
		                                        // log level >= 2
		void outDetail( const char * str, ...)  ATTR_PRINTF(2,3);
		                                        // log level >= 3
		void outDebug( const char * str, ...)   ATTR_PRINTF(2,3);
		                                        // log level >= 3
		void outDebugInLine( const char * str, ...)  ATTR_PRINTF(2,3);
		                                        // any log level
		void outErrorDb( const char * str, ...) ATTR_PRINTF(2,3);
		                                        // any log level
		void outChar( const char * str, ...)    ATTR_PRINTF(2,3);

		void SetLogLevel(char * Level);
		void SetLogFileLevel(char * Level);
		void SetColor(bool stdout_stream, Color color);
		void ResetColor(bool stdout_stream);
		void outTime();
		void outTimestamp(FILE* file);
		uint32 getLogFilter() const { return m_logFilter; }

		bool IsOutDebug() const { return m_logLevel > 2 || (m_logFileLevel > 2 && logfile); }

		bool IsIncludeTime() const { return m_includeTime; }
	private:
		FILE* logfile;
		FILE* gmLogFile;
		FILE* charLogfile;
		FILE* dberLogfile;

		// log/console control
		uint32 m_logLevel;
		uint32 m_logFileLevel;
		bool   m_colored;
		bool   m_includeTime;
		Color  m_colors[4];

		uint32 m_logFilter;

};

#define sLog LeGACY::Singleton<Log>::Instance()
#define LEGACY_DEBUG
#ifdef LEGACY_DEBUG
#define DEBUG_LOG LeGACY::Singleton<Log>::Instance().outDebug
#else
#define DEBUG_LOG
#endif

// primary for script library
void LEGACY_DLL_SPEC outstring_log(const char * str, ...) ATTR_PRINTF(1,2);
void LEGACY_DLL_SPEC debug_log(const char * str, ...) ATTR_PRINTF(1,2);
void LEGACY_DLL_SPEC error_log(const char * str, ...) ATTR_PRINTF(1,2);


#endif
