/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __DEFINE_H__
#define __DEFINE_H__
#include "Bool.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER

#ifdef WIN_GNU_32
#error WIN32 and WIN_GNU_32 cannot both be defined
#endif // WIN_GNU_32
#ifdef LINUX_GNU_32
#error WIN32 and LINUX_GNU_32 cannot both be defined
#endif // LINUX_GNU_32

#ifndef WINVER
#define WINVER 0x0501
#endif // WINVER

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif // _WIN32_WINNT

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif // _WIN32_WINDOWS

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif // _WIN32_IE

#include <stdlib.h>
#include <memory.h>

#define FILE_SEPARATOR "\\"
#define _FS_ FILE_SEPARATOR
#define OTHER_OS_FILE_SEPARATOR "/"
#define __ENGINE_FUNCTION__ __FUNCTION__
#define __ENGINE_PRETTY_FUNCTION__ __FUNCSIG__
#define __ENGINE_DECORATED_FUNCTION__ __FUNCDNAME__
#endif // _MSC_VER


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
#ifdef WIN_GNU_32
#include <stdio.h>
#include <stdarg.h>
//#define MAX_PATH 260
#define INT_MAX         0x7fffffff
#define FILE_SEPARATOR "\\"
#define OTHER_OS_FILE_SEPARATOR "/"
#define __ENGINE_FUNCTION__ __FUNCTION__
#define __ENGINE_PRETTY_FUNCTION__ __PRETTY_FUNCTION__
#endif // WIN_GNU_32


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
#ifdef LINUX_GNU_32
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <memory.h>
#include <stddef.h>
#define MAX_PATH 4096
#define INT_MAX         0x7fffffff
#define FILE_SEPARATOR "/"
#define OTHER_OS_FILE_SEPARATOR "\\"
#define __ENGINE_FUNCTION__ __FUNCTION__
#define __ENGINE_PRETTY_FUNCTION__ __PRETTY_FUNCTION__
#endif // LINUX_GNU_32


#endif // __DEFINE_H__

