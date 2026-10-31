/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "ASCIIIndex.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CASCIIIndex::Init(void)
{
	mcNameToIDIndex.Init();
	mcIDToNameIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CASCIIIndex::Kill(void)
{
	mcNameToIDIndex.Kill();
	mcIDToNameIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int64 CASCIIIndex::Add(const char* sz, const char* szLastCharInclusive)
{
	int64	lliID;
	bool	bResult;

	lliID = mcNameToIDIndex.Add(sz, szLastCharInclusive);
	if (lliID != -1)
	{
		bResult = mcIDToNameIndex.Put(lliID, sz, szLastCharInclusive);
		if (!bResult)
		{
			mcNameToIDIndex.Remove(sz, szLastCharInclusive);
			lliID = -1;
		}
	}
	return lliID;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CASCIIIndex::NumElements(void)
{
	return mcNameToIDIndex.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CASCIIIndex::Get(int64 lliID, int* piLength)
{
	char*	psz;
	size	iLength;
	bool	bResult;

	bResult = mcIDToNameIndex.Get(lliID, &psz, &iLength);
	if (bResult)
	{
		SafeAssign(piLength, (int)iLength);
		return psz;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int64 CASCIIIndex::Get(const char* szText, const char* szLastCharInclusive, bool bExact)
{
	SASCIINameIndex*	psNameIndex;

	psNameIndex = mcNameToIDIndex.Get(szText, szLastCharInclusive, bExact);
	if (psNameIndex)
	{
		return psNameIndex->mlliID;
	}
	else
	{
		return -1;
	}
}

