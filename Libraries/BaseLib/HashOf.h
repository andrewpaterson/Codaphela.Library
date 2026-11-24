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
#ifndef __HASH_OF_H__
#define __HASH_OF_H__
#include <string.h>
#include "PrimitiveTypes.h"
#include "FNVHash.h"


//Whether the key a map was asked about (pvKey) is the key kept in an entry (pvEntryKey).  NULL in a hash's policy means
//the keys are equal when their bytes are.
typedef bool (*HashEquals)(const void* pvKey, const void* pvEntryKey);


//The hash table takes a hash's low bits, so these mix the high bits down: an address's low bits are always 0, and
//FNV-1a's low bits depend only on the low bits of each byte.
inline uint32 HashMix32(uint32 ui)
{
	ui *= 0x9E3779B9u;
	return ui ^ (ui >> 16);
}


inline uint32 HashMix64(uint64 ulli)
{
	ulli *= 0x9E3779B97F4A7C15ULL;
	return (uint32)(ulli >> 32) ^ (uint32)ulli;
}


//An FNV-1a hash made ready for the table: its high bits mixed into the low ones.
inline uint32 HashFNVFinish(uint32 uiHash)
{
	return uiHash ^ (uiHash >> 15);
}


inline uint32 HashBytes(const void* pv, size uiLength)
{
	return HashFNVFinish(FNVHash32(pv, uiLength));
}


//A hash map's or set's HASH: what a key hashes to, and how two keys are compared.  This, the default, is for a key
//whose bytes are all it is: an integer, a pointer, an enum, or a struct with no padding (padding isn't cleared, so a
//struct with any needs a HASH of its own, as does a key that points at what it means, like SHashText).
//
//  static uint32       Hash(const K* pKey);
//  static HashEquals   Equals(void);       //NULL: equal when the bytes are.
template<class K>
struct SHashOf
{
	static uint32 Hash(const K* pKey)
	{
		uint32	ui;
		uint64	ulli;

		if (sizeof(K) <= sizeof(uint32))
		{
			ui = 0;
			memcpy(&ui, pKey, sizeof(K));
			return HashMix32(ui);
		}
		else if (sizeof(K) <= sizeof(uint64))
		{
			ulli = 0;
			memcpy(&ulli, pKey, sizeof(K));
			return HashMix64(ulli);
		}
		else
		{
			return HashBytes(pKey, sizeof(K));
		}
	}

	static HashEquals Equals(void)
	{
		return NULL;
	}
};


#endif // __HASH_OF_H__

