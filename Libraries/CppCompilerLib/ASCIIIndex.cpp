/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2021 Andrew Paterson

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
#include <string.h>
#include "ASCIIIndex.h"


#define ASCII_INDEX_CHUNK			(16 KB)


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CASCIIIndex::Init(void)
{
	mcNames.Init(&gcSystemAllocator, ASCII_INDEX_CHUNK);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CASCIIIndex::Kill(void)
{
	mcNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The new name's ID, or -1 if it is in already.
//
//////////////////////////////////////////////////////////////////////////
int64 CASCIIIndex::Add(const char* sz, const char* szLastCharInclusive)
{
	lsize	uiIndex;
	bool	bAdded;

	uiIndex = mcNames.FindOrAddIndex(sz, (size)Length(sz, szLastCharInclusive), &bAdded);
	return (bAdded && (uiIndex != SArrayLargeSize::NotFound)) ? (int64)uiIndex : -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CASCIIIndex::NumElements(void) const
{
	return (int)mcNames.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CASCIIIndex::Get(int64 lliID, int* piLength)
{
	if ((lliID < 0) || (lliID >= (int64)mcNames.NumElements()))
	{
		return NULL;
	}
	SafeAssign(piLength, (int)mcNames.GetKeyLength((lsize)lliID));
	return (char*)mcNames.GetKey((lsize)lliID);
}


//////////////////////////////////////////////////////////////////////////
//
// The name's ID, or -1.
//
//////////////////////////////////////////////////////////////////////////
int64 CASCIIIndex::Get(const char* szText, const char* szLastCharInclusive)
{
	lsize	uiIndex;

	uiIndex = mcNames.Find(szText, (size)Length(szText, szLastCharInclusive));
	return (uiIndex == SArrayLargeSize::NotFound) ? -1 : (int64)uiIndex;
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

