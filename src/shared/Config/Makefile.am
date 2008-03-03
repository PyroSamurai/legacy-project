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

AM_CPPFLAGS = -I$(top_builddir)/src/shared \
			  -I$(srcdir) \
			  -I$(srcdir)/../../framework \
			  -fpermissive

#			  -I$(srcdir)/../../../dep/include

noinst_LIBRARIES = liblegacyconfig.a

liblegacyconfig_a_SOURCES = \
	dotconfpp/dotconfpp.cpp \
	dotconfpp/dofconfpp.h \
	dotconfpp/mempool.cpp \
	dotconfpp/mempool.h \
	Config.cpp \
	Config.h \
	ConfigEnv.h

