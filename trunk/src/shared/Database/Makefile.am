#
# Copyright (C) 2008-2008 LeGACY <http://www.legacy-project.org/>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

AM_CPPFLAGS = $(MYSQL_INCLUDES) $(POSTGRE_INCLUDES) \
			  -I$(top_builddir)/src/shared \
			  -I$(srcdir) \
			  -I$(srcdir)/../../../dep/include \
			  -I$(srcdir)/../../framework \
			  -I$(srcdir)/../shared \
			  -fpermissive

noinst_LIBRARIES = liblegacydatabase.a

liblegacydatabase_a_SOURCES = \
	Database.cpp Database.h \
	DatabaseEnv.h \
	DatabaseImpl.h \
	DatabaseMysql.cpp DatabaseMysql.h \
	Field.cpp Field.h \
	MySQLDelayThread.h \
	QueryResult.h \
	QueryResultMysql.cpp QueryResultMysql.h \
	SQLStorage.cpp SQLStorage.h \
	SqlDelayThread.cpp SqlDelayThread.h \
	SqlOperations.cpp SqlOperations.h 
