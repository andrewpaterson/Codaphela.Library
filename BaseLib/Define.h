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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER

#ifdef WIN_GNU_32
#error WIN32 and WIN_GNU_32 cannot both be defined
#endif
#ifdef LINUX_GNU_32
#error WIN32 and LINUX_GNU_32 cannot both be defined
#endif

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include "Bool.h"
#define ENGINE_SIZE_T size_t
#define FILE_SEPARATOR "\\"
#define _FS_ FILE_SEPARATOR
#define OTHER_OS_FILE_SEPARATOR "/"
#define __ENGINE_DECORATED_FUNCTION__ __FUNCDNAME__
#define __ENGINE_PRETTY_FUNCTION__ __FUNCTION__
#endif // WIN32


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
#ifdef WIN_GNU_32
#include <stdio.h>
#include <stdarg.h>
typedef int BOOL;
#define FALSE 0
#define TRUE 1
#define engine_stricmp(p,q) strcmpi(p,q)
#define ENGINE_SIZE_T
#define EngineOutput(p)	printf(p)
//#define MAX_PATH 260
//#define _IOREAD         0x0001
//#define _IOWRT          0x0002
//#define _IOMYBUF        0x0008
//#define _IOEOF          0x0010
//#define _IOERR          0x0020
//#define _IOSTRG         0x0040
//#define _IORW           0x0080
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
typedef int BOOL;
#define FALSE 0
#define TRUE 1
#define engine_stricmp(p,q) strcmpi(p,q)
#define ENGINE_SIZE_T
#define EngineOutput(p)	printf(p)
#define MAX_PATH 4096
#define _IOREAD         0x0001
#define _IOWRT          0x0002
#define _IOMYBUF        0x0008
#define _IOEOF          0x0010
#define _IOERR          0x0020
#define _IOSTRG         0x0040
#define _IORW           0x0080
#define INT_MAX         0x7fffffff
#define FILE_SEPARATOR "/"
#define OTHER_OS_FILE_SEPARATOR "\\"
#define __ENGINE_FUNCTION__ __FUNCTION__
#define __ENGINE_PRETTY_FUNCTION__ __PRETTY_FUNCTION__
#endif // LINUX_GNU_32


#endif // __DEFINE_H__

