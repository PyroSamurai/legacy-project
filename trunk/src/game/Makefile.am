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
			  -I$(srcdir)/../framework \
			  -I$(srcdir)/../../dep/include \
			  -fpermissive

noinst_LIBRARIES = liblegacygame.a

liblegacygame_a_SOURCES = \
	Cell.h \
	CellImpl.h \
	ChatHandler.cpp \
	CharacterHandler.cpp \
	GameObject.cpp GameObject.h \
	GridDefines.h \
	GridNotifiers.h GridNotifiers.cpp \
	NameTables.h \
	Map.cpp Map.h \
	MapManager.cpp MapManager.h \
	MotionMaster.cpp MotionMaster.h \
	MovementHandler.cpp \
	ObjectAccessor.cpp ObjectAccessor.h \
	ObjectDefines.h \
	ObjectGridLoader.cpp ObjectGridLoader.h \
	Object.cpp Object.h \
	ObjectMgr.cpp ObjectMgr.h \
	Opcodes.cpp Opcodes.h \
	Player.cpp Player.h \
	RedZoneDistrict.cpp RedZoneDistrict.h \
	SharedDefines.h \
	Unit.cpp Unit.h \
	UpdateData.cpp UpdateData.h \
	UpdateFields.h \
	World.cpp World.h \
	WorldLog.cpp WorldLog.h \
	WorldSession.cpp WorldSession.h \
	WorldSocket.cpp WorldSocket.h \
	WorldSocketMgr.cpp WorldSocketMgr.h

## Link against shared library
liblegacygame_a_LIBADD = ../shared/liblegacyshared.a ../shared/Config/liblegacyconfig.a ../shared/Database/liblegacydatabase.a /usr/local/lib/libSockets.a
