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
			  -I$(srcdir)/../shared \
			  -I$(srcdir)/../../dep/include \
			  -I$(srcdir)/../framework \
			  -I$(srcdir)/../game \
			  -I$(srcdir) \
			  -fpermissive

bin_PROGRAMS = legacyd

legacyd_SOURCES = \
	Main.cpp \
	Master.cpp Master.h \
	WorldRunnable.cpp WorldRunnable.h

legacyd_LDADD = \
	../../dep/src/sockets/liblegacysockets.a \
	../game/liblegacygame.a \
	../shared/Database/liblegacydatabase.a \
	../shared/Config/liblegacyconfig.a \
	../shared/liblegacyshared.a \
	../framework/liblegacyframework.a \
	/usr/local/lib/libZThread.a \
	-lpthread

legacyd_LDFLAGS = \
	-L../../dep/src/sockets \
	-L../../dep/src/zthread \
	$(MYSQL_LIBS) $(POSTGRE_LIBS) -L$(libdir) $(ZLIB) $(COMPATLIB) $(SSLLIB) \
	-export-dynamic

# install legacyd.conf to $(sysconfdir)
sysconf_DATA = \
	legacyd.conf
