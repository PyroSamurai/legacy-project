# Copyright (C) 2008-2008 LeGACY <http://code.google.com/p/legacy-project/>
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

## Process this file with automake to produce Makefile.in

## Sub-directories to parse

## CPP flags for includes, defines, etc.
AM_CPPFLAGS = $(MYSQL_INCLUDES) $(POSTGRE_INCLUDES) -I$(srcdir) -I$(srcdir)/../../../dep/include -I$(srcdir)/../../shared/  -I$(srcdir)/../../framework/

## Build LeGACY script library as shared library.
#  liblegacyscript shared library will later be reused by world server daemon.
lib_LTLIBRARIES = liblegacyscript.la
liblegacyscript_la_SOURCES = \
	ScriptMgr.cpp \
	ScriptMgr.h \
	config.h \
	scripts/npc/npc_innkeeper.cpp \
	system.cpp \
	scripts/sc_default.cpp \
	scripts/sc_defines.cpp \
	scripts/sc_defines.h

## libtool settings
#  API versioning
#  Link against dependencies 
#  How to increase version info:
#  - only bug fixes implemented:
#    bump the version to LTLEGACY_CURRENT:LTLEGACY_REVISION+1:LTLEGACY_AGE
#  - augmented the interface:
#    bump the version to LTLEGACY_CURRENT+1:0:LTLEGACY_AGE+1
#  - broken old interface:
#    bump the version to LTLEGACY_CURRENT+1:0:0
LTLEGACY_CURRENT = 0
LTLEGACY_REVISION = 0
LTLEGACY_AGE = 0
liblegacyscript_la_LIBADD = $(MYSQL_LIBS) $(POSTGRE_LIBS)
liblegacyscript_la_LIBFLAGS = -version-info $(LTLEGACY_CURRENT):$(LTLEGACY_REVISION):$(LTLEGACY_AGE)

## Additional files to include when running 'make dist'
#  Scripts defaults.
EXTRA_DIST = \
	Scripts/sc_default.cpp \
	Scripts/sc_defines.cpp \
	Scripts/sc_defines.h
