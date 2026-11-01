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
#include <string.h>
#include "ASCIIIndex.h"


#define ASCII_INDEX_FIRST_TABLE		256
#define ASCII_INDEX_CHUNK			(16 KB)


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* SASCIIIndexName::Text(void)
{
	return (char*)&this[1];
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CASCIIIndex::Init(void)
{
	lsize	ui;

	mcNames.Init(ASCII_INDEX_CHUNK);
	mapsNames.Init();
	maiTable.Init();
	maiTable.Resize(ASCII_INDEX_FIRST_TABLE);
	for (ui = 0; ui < ASCII_INDEX_FIRST_TABLE; ui++)
	{
		*maiTable.Get(ui) = -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CASCIIIndex::Kill(void)
{
	maiTable.Kill();
	mapsNames.Kill();
	mcNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The new name's ID, or -1 if it is already in the index.
//
//////////////////////////////////////////////////////////////////////////
int64 CASCIIIndex::Add(const char* sz, const char* szLastCharInclusive)
{
	int					iLength;
	uint32				uiHash;
	lsize				uiSlot;
	lsize				uiID;
	SASCIIIndexName*	psName;

	iLength = Length(sz, szLastCharInclusive);
	uiHash = Hash(sz, iLength);
	uiSlot = Find(sz, iLength, uiHash);
	if (*maiTable.Get(uiSlot) != -1)
	{
		return -1;
	}

	psName = (SASCIIIndexName*)mcNames.Add(sizeof(SASCIIIndexName) + iLength + 1);
	psName->uiHash = uiHash;
	psName->iLength = iLength;
	memcpy(psName->Text(), sz, iLength);
	psName->Text()[iLength] = '\0';

	uiID = mapsNames.NumElements();
	mapsNames.Add(&psName);
	*maiTable.Get(uiSlot) = (int32)uiID;
	if (mapsNames.NumElements() * 2 > maiTable.NumElements())
	{
		Grow();
	}
	return (int64)uiID;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CASCIIIndex::NumElements(void)
{
	return (int)mapsNames.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CASCIIIndex::Get(int64 lliID, int* piLength)
{
	SASCIIIndexName*	psName;

	if ((lliID < 0) || (lliID >= (int64)mapsNames.NumElements()))
	{
		return NULL;
	}
	psName = *mapsNames.Get((lsize)lliID);
	SafeAssign(piLength, (int)psName->iLength);
	return psName->Text();
}


//////////////////////////////////////////////////////////////////////////
//
// The name's ID, or -1.  Only exact matches are found (bExact is ignored).
//
//////////////////////////////////////////////////////////////////////////
int64 CASCIIIndex::Get(const char* szText, const char* szLastCharInclusive, bool bExact)
{
	int		iLength;
	int32	iID;

	iLength = Length(szText, szLastCharInclusive);
	iID = *maiTable.Get(Find(szText, iLength, Hash(szText, iLength)));
	return (int64)iID;
}


//////////////////////////////////////////////////////////////////////////
//
// The slot with the name, or the empty slot it would go in.
//
//////////////////////////////////////////////////////////////////////////
lsize CASCIIIndex::Find(const char* sz, int iLength, uint32 uiHash)
{
	lsize				uiMask;
	lsize				uiSlot;
	int32				iID;
	SASCIIIndexName*	psName;

	uiMask = maiTable.NumElements() - 1;
	uiSlot = (lsize)uiHash & uiMask;
	for (;;)
	{
		iID = *maiTable.Get(uiSlot);
		if (iID == -1)
		{
			return uiSlot;
		}
		psName = *mapsNames.Get((lsize)iID);
		if ((psName->uiHash == uiHash) && (psName->iLength == iLength) && (memcmp(psName->Text(), sz, iLength) == 0))
		{
			return uiSlot;
		}
		uiSlot = (uiSlot + 1) & uiMask;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Doubles the table and puts every name back in it.
//
//////////////////////////////////////////////////////////////////////////
void CASCIIIndex::Grow(void)
{
	lsize				uiLength;
	lsize				ui;
	lsize				uiSlot;
	SASCIIIndexName*	psName;

	uiLength = maiTable.NumElements() * 2;
	maiTable.Resize(uiLength);
	for (ui = 0; ui < uiLength; ui++)
	{
		*maiTable.Get(ui) = -1;
	}
	for (ui = 0; ui < mapsNames.NumElements(); ui++)
	{
		psName = *mapsNames.Get(ui);
		uiSlot = Find(psName->Text(), psName->iLength, psName->uiHash);
		*maiTable.Get(uiSlot) = (int32)ui;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// FNV-1a.
//
//////////////////////////////////////////////////////////////////////////
uint32 CASCIIIndex::Hash(const char* sz, int iLength)
{
	uint32	uiHash;
	int		i;

	uiHash = 2166136261u;
	for (i = 0; i < iLength; i++)
	{
		uiHash ^= (uint8)sz[i];
		uiHash *= 16777619u;
	}
	return uiHash;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CASCIIIndex::Length(const char* sz, const char* szLastCharInclusive)
{
	if (szLastCharInclusive)
	{
		return (int)(szLastCharInclusive - sz + 1);
	}
	return (int)strlen(sz);
}

