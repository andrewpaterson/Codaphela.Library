/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __F_N_V_HASH_H__
#define __F_N_V_HASH_H__
#include "BaseLib/PrimitiveTypes.h"


//FNV-1a of uiLength bytes from sz: the same bytes give the same hash in every build (a function's fingerprint, the
//define map's and the ASCII index's slots, the name sets).
inline uint32 FNVHash32(const char* sz, size uiLength)
{
	uint32	uiHash;
	size	i;

	uiHash = 2166136261u;
	for (i = 0; i < uiLength; i++)
	{
		uiHash ^= (uint8)sz[i];
		uiHash *= 16777619u;
	}
	return uiHash;
}


inline uint64 FNVHash64(const char* sz, size uiLength)
{
	uint64	ulliHash;
	size	i;

	ulliHash = 0xCBF29CE484222325ULL;
	for (i = 0; i < uiLength; i++)
	{
		ulliHash ^= (uint8)sz[i];
		ulliHash *= 0x100000001B3ULL;
	}
	return ulliHash;
}


#endif // __F_N_V_HASH_H__
