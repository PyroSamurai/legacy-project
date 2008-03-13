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
#include "Log.h"
#include "Policies/SingletonImp.h"
#include "Config/ConfigEnv.h"

#include <stdarg.h>

INSTANTIATE_SINGLETON_1( Log );

enum LogType
{
	LogNormal = 0,
	LogDetails,
	LogDebug,
	LogError
};

const int LogType_count = int(LogError) + 1;

void Log::SetLogLevel(char *Level)
{
	int32 NewLevel = atoi((char*)Level);
	if(NewLevel < 0)
		NewLevel = 0;
	m_logLevel = NewLevel;

	printf( "LogLevel is %u\n", m_logLevel);
}


void Log::Initialize()
{
	logfile = fopen("logfile.log", "w");
	struct tm * now;

	m_logLevel  = sConfig.GetIntDefault("LogLevel", 3);

	dberLogfile = fopen("dberr.log", "w");
}

void Log::outString()
{
	if(logfile)
	{
		fprintf(logfile, "\n");
		fflush(logfile);
	}
	fflush(stdout);
}

void Log::outString( const char * str, ...)
{
	if( !str ) return;

	va_list ap;
	va_start(ap, str);
	vprintf( str, ap);
	va_end(ap);

	printf( "\n" );
	if(logfile)
	{
		va_start(ap, str);
		vfprintf(logfile, str, ap);
		fprintf(logfile, "\n");
		va_end(ap);
		fflush(logfile);
	}
	fflush(stdout);
}

void Log::outError( const char * err, ...)
{
	if( !err ) return;

	va_list ap;
	va_start(ap, err);
	vfprintf( stderr, err, ap );
	va_end(ap);

	fprintf( stderr, "\n");
	if(logfile)
	{
		fprintf(logfile, "ERROR:" );
		va_start(ap, err);
		vfprintf(logfile, err, ap);
		fprintf(logfile, "\n");
		va_end(ap);
		fflush(logfile);
	}
	fflush(stderr);
}

void Log::outDetail( const char * str, ...)
{
	if( !str ) return;
	va_list ap;
	if( m_logLevel > 1)
	{
		va_start(ap, str);
		vprintf( str, ap );
		va_end(ap);

		printf( "\n" );
	}
	if(logfile && m_logFileLevel > 1)
	{
		va_start(ap, str);
		vfprintf(logfile, str, ap);
		fprintf(logfile, "\n" );
		va_end(ap);
		fflush(logfile);
	}

	fflush(stdout);
}

void Log::outDebugInLine( const char * str, ...)
{
	if( !str ) return;
	va_list ap;
	if( m_logLevel > 2)
	{
		va_start(ap, str);
		vprintf( str, ap );
		va_end(ap);
	}

	if (logfile && m_logFileLevel > 2)
	{
		va_start(ap, str);
		vfprintf(logfile, str, ap);
		va_end(ap);
	}
}

void Log::outDebug( const char * str, ...)
{
	if( !str ) return;
	va_list ap;
	if( m_logLevel > 2)
	{
		va_start(ap, str);
		vprintf( str, ap );
		va_end(ap);

		printf( "\n" );
	}
	if(logfile && m_logFileLevel > 2)
	{
		va_start(ap, str);
		vfprintf(logfile, str, ap);
		fprintf(logfile, "\n");
		va_end(ap);
		fflush(logfile);
	}
	fflush(stdout);
}

void Log::outErrorDb( const char * err, ... )
{
	if( !err ) return;

	va_list ap;
	va_start(ap, err);
	vfprintf( stderr, err, ap );
	va_end(ap);

	if(logfile)
	{
		fprintf(logfile, "ERROR:" );
		va_start(ap, err);
		vfprintf(logfile, err, ap);
		fprintf(logfile, "\n");
		va_end(ap);
		fflush(logfile);
	}

	if(dberLogfile)
	{
		va_start(ap, err);
		vfprintf(dberLogfile, err, ap);
		fprintf(dberLogfile, "\n");
		va_end(ap);
		fflush(dberLogfile);
	}
	fflush(stderr);
}

void outstring_log(const char * str, ...)
{
	if( !str ) return;

	char buf[256];
	va_list ap;
	va_start(ap, str);
	vsnprintf(buf,256, str, ap);
	va_end(ap);

	LeGACY::Singleton<Log>::Instance().outString(buf);
}

void debug_log(const char * str, ...)
{
	if( !str ) return;

	char buf[256];
	va_list ap;
	va_start(ap, str);
	vsnprintf(buf,256, str, ap);
	va_end(ap);

	LeGACY::Singleton<Log>::Instance().outDebug(buf);
}

void error_log(const char * str, ...)
{
	if( !str ) return;

	char buf[256];
	va_list ap;
	va_start(ap, str);
	vsnprintf(buf,256, str, ap);
	va_end(ap);

	LeGACY::Singleton<Log>::Instance().outError(buf);
}
