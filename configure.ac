#
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

AC_INIT([LeGACY],
		[0.0.0-ep5-SVN],
		[http://www.code.google.com/p/legacy-project],
		[legacy])

AM_CONFIG_HEADER([src/config.h])
AC_CONFIG_SRCDIR([src/legacyd/Main.cpp])

# Turn on all warning and error messages, and enforce GNU
# standart for the package
AM_INIT_AUTOMAKE([-Wall -Werror])
AM_MAINTAINER_MODE

### Check for required dependencies

## Check for a valid build environment.
#  Valid equals having:
#  - a C++ compiler compliant with the ISO98 C++ specification.
#  - a working library tool for creating convenience libraries.
#  - a working linker for creating static and shared libraries.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_INSTALL

## Check for required libraries.
AC_CHECK_LIB( pthread, pthread_create, [],
	[LDFLAGS="-pthread $LDFLAGS"
	 AC_TRY_LINK([char pthread_create();],
		 pthread_create();,
		 [], [AC_MSG_ERROR([Missing pthread])])
	 ])
AC_CHECK_LIB( z, compress, [ZLIB=-lz],[AC_MSG_ERROR([Missing zlib])] )
AC_CHECK_LIB( compat, ftime, [COMPATLIB=-lcompat] )
AC_CHECK_LIB( crypto, SHA1_Init, [SSLLIB=-lssl], [AC_MSG_ERROR([Missing openssl])])

AC_ARG_WITH(mysql,
	--with-mysql         Use MySQL as a backend (default: yes)],
[case "${withval}" in
	yes) DO_MYSQL=yes ;;
	no)  DO_MYSQL=no ;;
	maybe) DO_MYSQL=maybe ;;
	*)   AC_MSG_ERROR(Bad value ${withval} for --with-mysql) ;;
	esac],
[DO_MYSQL=yes])

# here mysql
AC_MSG_CHECKING(whether to build/link MYSQL)
if test "x$DO_MYSQL" = "xyes"; then
AC_MSG_RESULT($DO_MYSQL)
AC_PATH_PROGS(MYSQL_CONFIG, mysql_config, mysql_config, $PATH)
	if test -x "$MYSQL_CONFIG"
		then
	# MySQL v4 uses --include while v3 uses --cflags
		MYSQL_INCLUDES="`$MYSQL_CONFIG --include`" || \
		MYSQL_INCLUDES="`$MYSQL_CONFIG --cflags`"
		MYSQL_LIBS="`$MYSQL_CONFIG --libs_r`"
	CXXFLAGS="-DDO_MYSQL $CXXFLAGS"
	fi
else
AC_MSG_RESULT($DO_MYSQL)
fi

### Check for options
#   Include debug info in library?
AC_MSG_CHECKING(whether to include debug info in library)
LEGACYD_DEBUG_INFO=yes
AC_ARG_WITH(debug-info,
[
Debugging options:

  --with-debug-info         Include debug info in library],
[
	if test "$withval" = "yes" ; then
	CFLAGS="-g -DLEGACY_DEBUG $CFLAGS"
	CXXFLAGS="-g -DLEGACY_DEBUG $CXXFLAGS"
	LEGACYD_DEBUG_INFO=yes
	elif test "$withval" != "no" ; then
	AC_MSG_ERROR(Please choose yes or no)
	fi
])
AC_MSG_RESULT($LEGACYD_DEBUG_INFO)

## Check for required header files.
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS([ arpa/inet.h fcntl.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/ioctl.h sys/param.h sys/socket.h sys/timeb.h sys/time.h termios.h unistd.h  ])

AC_CHECK_HEADERS([pthread.h])
AC_CHECK_HEADERS([openssl/md5.h openssl/rand.h openssl/ssl.h openssl/sha.h openssl/bn.h])
AC_CHECK_HEADERS([mysql.h mysql/mysql.h])
#AC_CHECK_HEADERS([libpq-fe.h])
AC_CHECK_HEADERS([zlib.h])

## Check for typedefs, structures, and compiler characteristics
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UINT64_T
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t])

## Check for required library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit ftime gethostbyaddr gethostbyname gethostname gettimeofday memmove memset pow realpath select socket sqrt strchr strdup strerror strstr])

## Export defines variables
AC_SUBST(ZLIB)
AC_SUBST(COMPATLIB)
AC_SUBST(SSLLIB)
AC_SUBST(MYSQL_INCLUDES)
AC_SUBST(MYSQL_LIBS)
AC_SUBST(LEGACYD_DEBUG_INFO)

## Set output files.
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	dep/include/Makefile
	dep/src/Makefile
	dep/src/sockets/Makefile
	Makefile
	src/framework/Makefile
	src/shared/Makefile
	src/shared/Config/Makefile
	src/shared/Database/Makefile
	src/shared/SystemConfig.h
	src/game/Makefile
	src/legacyd/Makefile
	src/legacyd/legacyd.conf
])

## Disabled Makefiles, until they are ready for a successful make and make dist run.


## Output files.
AC_OUTPUT
