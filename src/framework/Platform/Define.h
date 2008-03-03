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


#ifndef __LEGACY_DEFINE_H
#define __LEGACY_DEFINE_H

#include "Platform/CompilerDefs.h"
#include <sys/types.h>

#ifdef WIN32
#define LEGACY_EXPORT __declspec(dllexport)
#define LEGACY_LIBRARY_HANDLE HMODULE
#define LEGACY_LOAD_LIBRARY(a) LoadLibrary(a)
#define LEGACY_CLOSE_LIBRARY FreeLibrary
#define LEGACY_GET_PROC_ADDR GetProcAddress
#define LEGACY_IMPORT __cdecl
#define LEGACY_SCRIPT_EXT ".dll"
#define LEGACY_SCRIPT_NAME "LeGACYSscript"
#define SIGQUIT 3
#else
#define LEGACY_LIBRARY_HANDLE void*
#define LEGACY_EXPORT export
#define LEGACY_LOAD_LIBRARY(a) dlopen(a,RTLD_NOW)
#define LEGACY_CLOSE_LIBRARY dlclose
#define LEGACY_GET_PROC_ADDR dlsym

#define LEGACY_IMPORT __attribute__ ((cdecl))

#define LEGACY_SRIPT_EXT ".so"
#define LEGACY_SCRIPT_NAME "libLeGACYScript"
#endif

#ifdef WIN32
#ifdef LEGACY_WIN32_DLL_IMPORT

#define LEGACY_DLL_DECL __declspec(dllimport)
#else
#ifdef LEGACY_WIND_DLL_EXPORT
#define LEGACY_DLL_DECL __declspec(dllexport)
#else
#define LEGACY_DLL_DECL
#endif
#endif

#else
#define LEGACY_DLL_DECL
#endif

#ifndef DEBUG
#define LEGACY_INLINE inline
#else
#ifndef LEGACY_DEBUG
#define LEGACY_DEBUG
#endif
#define LEGACY_INLINE
#endif

#if COMPILER == COMPILER_MICROSOFT
typedef __int64            int64;
typedef long               int32;
typedef short              int16;
typedef char               int8;
typedef unsigned __int64   uint64;
typedef unsigned long      uint32;
typedef unsigned short     uint16;
typedef unsigned char      uint8;
#else
typedef long long int64;
typedef __int64_t   int64;
typedef __int32_t   int32;
typedef __int16_t   int16;
typedef __int8_t    int8;
typedef __uint64_t  uint64;
typedef __uint32_t  uint32;
typedef __uint16_t  uint16;
typedef __uint8_t   uint8;
typedef uint16      WORD;
typedef uint32      DWORD;
#endif

typedef uint64       OBJECT_HANDLE;

#if PLATFORM == PLATFORM_WIN32
#	define LEGACY_DLL_SPEC __declspec(dllexport)
#	ifndef DECLSPEC_NORETURN
#		define DECLSPEC_NORETURN __declspec(noreturn)
#	endif
#else
#	define LEGACY_DLL_SPEC
#	define DECLSPEC_NORETURN
#endif

#if COMPILER == COMPILER_GNU
#	define ATTR_NORETURN __attribute__((noreturn))
#	define ATTR_PRINTF(F,V) __attribute__ ((format (printf, F, V)))
#else
#	define ATTR_NORETURN
#	define ATTR_PRINTF(F,V)
#endif
#endif
