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
#include <stddef.h>
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


//BUILD_64 is defined when pointers are 8 bytes (MSVC x64 or g++ -m64).
#if defined(_WIN64) || defined(__LP64__)
#define BUILD_64
#endif // _WIN64 || __LP64__


//MSVC aligns 64 bit types to 8 bytes in structs, g++ -m32 aligns them to 4.
//Match MSVC so structs read and written as raw bytes have the same layout.
//g++ drops ALIGN_64 when one of these types is used as a template argument so
//templates that store a template type by value must declare the member with
//MSVC_ALIGN(T) instead.  g++ -m64 already aligns them to 8.
#if defined(LINUX_GNU) && !defined(BUILD_64) && defined(__cplusplus)
//The arithmetic types that are 8 bytes in a 32 bit build.
template<class T>	struct SAlign64					{ static const bool bAlign64 = false; };
template<class T>	struct SAlign64<const T>		{ static const bool bAlign64 = SAlign64<T>::bAlign64; };
template<>			struct SAlign64<long long>			{ static const bool bAlign64 = true; };
template<>			struct SAlign64<unsigned long long>	{ static const bool bAlign64 = true; };
template<>			struct SAlign64<double>				{ static const bool bAlign64 = true; };
#define ALIGN_64		__attribute__((aligned(8)))
#define MSVC_ALIGN(T)	alignas(SAlign64<T>::bAlign64 ? 8 : alignof(T))
#elif defined(LINUX_GNU) && !defined(BUILD_64)
#define ALIGN_64		__attribute__((aligned(8)))
#define MSVC_ALIGN(T)
#else
#define ALIGN_64
#define MSVC_ALIGN(T)
#endif // LINUX_GNU

typedef int8_t			int8;
typedef int16_t			int16;
typedef int32_t			int32;
typedef uint8_t			uint8;
typedef uint32_t		uint32;
typedef uint16_t		uint16;
#if defined(LINUX_GNU) && defined(BUILD_64)
//g++ -m64 makes int64_t, size_t and uintptr_t 'long'.  Use 'long long' as MSVC
//does so int64, uint64, size, ptr and ptrdiff are the same types on both and
//overloads resolve the same way.
typedef long long			int64;
typedef unsigned long long	uint64;
#else
typedef int64_t			int64 ALIGN_64;
typedef uint64_t		uint64 ALIGN_64;
#endif // LINUX_GNU && BUILD_64
typedef int64			int128 ALIGN_64;	// Until I implement struct SInt128 etc...
typedef uint64			uint128 ALIGN_64;
typedef char			char8;
#if defined(LINUX_GNU) && defined(__cplusplus)
typedef char16_t		char16;  //wchar_t is 4 bytes on Linux.
#elif defined(LINUX_GNU)
typedef uint16_t		char16;  //C has no char16_t without <uchar.h>.
#else
typedef wchar_t			char16;
#endif // LINUX_GNU
typedef float			float32;
typedef double			float64 ALIGN_64;
typedef long double		float128;
typedef unsigned int	uint;
//SIZE_16: size is 16 bits.  On the W65C816, and in a host build that checks the W65C816's size model (BASELIB_SIZE16:
//g++ -m32 -DBASELIB_SIZE16 -Wconversion warns of every lsize, or wider, value narrowed to a size).
#if defined(__W65C816__) || defined(BASELIB_SIZE16)
#define SIZE_16
#endif // __W65C816__ || BASELIB_SIZE16
#if defined(LINUX_GNU) && defined(BUILD_64) && defined(BASELIB_SIZE16)
#error BASELIB_SIZE16 is for 32 bit builds.
#elif defined(LINUX_GNU) && defined(BUILD_64)
typedef uint64			size;
typedef uint64			ptr;
typedef int64			ptrdiff;
#elif defined(SIZE_16)
//size is the W65C816's natural width; pointers (and size_t) stay 32 bits.  Use lsize past 64KB.
typedef uint16			size;
typedef uintptr_t		ptr;
typedef ptrdiff_t		ptrdiff;
#else
typedef size_t			size;
typedef uintptr_t		ptr;
typedef ptrdiff_t		ptrdiff;
#endif // LINUX_GNU && BUILD_64

//lsize counts and measures things that can pass 64KB.  It is the same type as size except on 16 bit targets, so
//only those can overload on size and lsize (SIZE_IS_LSIZE is defined everywhere else).
#if defined(SIZE_16)
typedef uint32			lsize;
#else
typedef size			lsize;
#define SIZE_IS_LSIZE
#endif // SIZE_16
#define MAX_SIZE		((size)-1)
#define MAX_LSIZE		((lsize)-1)

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

