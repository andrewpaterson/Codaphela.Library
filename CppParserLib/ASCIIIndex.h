/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2021 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ASCII_INDEX_H__
#define __ASCII_INDEX_H__
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Numbers.h"


//A name in the index: its hash and length, then its characters and a terminating zero.
struct SASCIIIndexName
{
	uint32	uiHash;
	int32	iLength;

	char*	Text(void);
};


//Names given IDs (0, 1, 2... in the order they are added) and found by name with a
//hash table.  The names are kept once, in a stack of chunks.
class CASCIIIndex
{
protected:
	CMemoryStackExtended						mcNames;
	CArrayLargeTemplate<SASCIIIndexName*>		mapsNames;		//By ID.
	CArrayLargeTemplate<int32>					maiTable;		//IDs by hash, -1 for an empty slot.  The length is a power of two.

public:
	void	Init(void);
	void	Kill(void);

	int64	Add(const char* sz, const char* szLastCharInclusive = NULL);

	char*	Get(int64 lliID, int* piLength = NULL);
	int64	Get(const char* szText, const char* szLastCharInclusive, bool bExact);

	int		NumElements(void);

protected:
	lsize	Find(const char* sz, int iLength, uint32 uiHash);
	void	Grow(void);
	static uint32	Hash(const char* sz, int iLength);
	static int		Length(const char* sz, const char* szLastCharInclusive);
};


#endif // __ASCII_INDEX_H__

