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
#ifndef __PRIMITIVE_TYPES_H__
#define __PRIMITIVE_TYPES_H__
#include "Bool.h"


#if (WIN32 || LINUX_GNU_32 || WIN_GNU_32)
typedef unsigned char			int8;
typedef unsigned char			uint8;
typedef int						int32;
typedef unsigned int			uint32;
typedef short					int16;
typedef char					char8;
typedef wchar_t					char16;
typedef float					float32;
typedef double					float64;
typedef unsigned int			uint32;
typedef unsigned short			uint16;
typedef long long int			int64;
typedef unsigned long long int	uint64;
#endif // (WIN32 || LINUX_GNU_32 || WIN_GNU_32)


#endif // __PRIMITIVE_TYPES_H__

