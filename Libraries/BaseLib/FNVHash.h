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

** ------------------------------------------------------------------------ **/
#ifndef __F_N_V_HASH_H__
#define __F_N_V_HASH_H__
#include "PrimitiveTypes.h"


//FNV-1a: the same bytes give the same hash in every build.  A hash is started with its offset basis and each byte is
//mixed in, so bytes hashed in pieces (passing the hash so far as uiHash) hash as they would all together.
#define FNV_HASH32_START	2166136261u
#define FNV_HASH32_PRIME	16777619u
#define FNV_HASH64_START	0xCBF29CE484222325ULL
#define FNV_HASH64_PRIME	0x100000001B3ULL


inline uint32 FNVHash32(const void* pv, lsize uiLength, uint32 uiHash = FNV_HASH32_START)
{
	const uint8*	pui;
	lsize			i;

	pui = (const uint8*)pv;
	for (i = 0; i < uiLength; i++)
	{
		uiHash = (uiHash ^ pui[i]) * FNV_HASH32_PRIME;
	}
	return uiHash;
}


inline uint64 FNVHash64(const void* pv, lsize uiLength, uint64 ulliHash = FNV_HASH64_START)
{
	const uint8*	pui;
	lsize			i;

	pui = (const uint8*)pv;
	for (i = 0; i < uiLength; i++)
	{
		ulliHash = (ulliHash ^ pui[i]) * FNV_HASH64_PRIME;
	}
	return ulliHash;
}


//A whole word mixed in at once, as a key made of pointers or numbers is: one FNV-1a step on the word rather than on
//each of its bytes.
inline uint64 FNVMix64(uint64 ulliHash, uint64 ulliWord)
{
	return (ulliHash ^ ulliWord) * FNV_HASH64_PRIME;
}


#endif // __F_N_V_HASH_H__

