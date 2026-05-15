/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __PRIMITIVE_TYPES_H__
#define __PRIMITIVE_TYPES_H__
#include <stdint.h>
#include <limits.h>


#define MAX_INT8	0x7f
#define MIN_INT8	0x80
#define MAX_UINT8	0xff
#define MIN_UINT8	0x00

#define MAX_INT16	0x7fff
#define MIN_INT16	0x8000
#define MAX_UINT16	0xffff
#define MIN_UINT16	0x0000

#define MAX_INT32	0x7fffffff
#define MIN_INT32	0x80000000
#define MAX_UINT32	0xffffffff
#define MIN_UINT32	0x00000000

#define MAX_INT		INT_MAX
#define MIN_INT		INT_MIN
#define MAX_UINT	UINT_MAX
#define MIN_UINT	0

#define MAX_INT64	0x7fffffffffffffffLL
#define MIN_INT64	0x8000000000000000LL
#define MAX_UINT64	0xffffffffffffffffLL
#define MIN_UINT64	0x0000000000000000LL


typedef int8_t			int8;
typedef int16_t			int16;
typedef int32_t			int32;
typedef int64_t			int64;
typedef uint8_t			uint8;
typedef uint32_t		uint32;
typedef uint16_t		uint16;
typedef uint64_t		uint64;
typedef int64_t			int128;	// Until I implement struct SInt128 etc...
typedef uint64_t		uint128;
typedef char			char8;
typedef wchar_t			char16;
typedef float			float32;
typedef double			float64;
typedef long double		float128;
typedef size_t			size;
typedef unsigned int	uint;
typedef uintptr_t		ptr;
typedef ptrdiff_t		ptrdiff;

#if INT_MAX == MAX_INT32
typedef int64			longint;
typedef uint64			ulongint;
#elif INT_MAX == MAX_INT16
typedef int32			longint;
typedef uint32			ulongint;
#elif INT_MAX == MAX_INT64
typedef int128			longint;
typedef uint128			ulongint;
#endif



#endif // __PRIMITIVE_TYPES_H__

