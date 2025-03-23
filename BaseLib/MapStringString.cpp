/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "MapStringString.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringString::Put(char* szKey, char* szValue)
{
	size	iStrLen;
	bool	bResult;

	if (szValue == NULL)
	{
		return false;
	}

	iStrLen = strlen(szValue);
	bResult = CMapStringBlock::Put(szKey, szValue, iStrLen + 1);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMapStringString::Put(uint8* szKey, char* szValue)
{
	return Put((char*)szKey, szValue);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CMapStringString::Get(char* szKey)
{
	return (char*)CMapStringBlock::Get(szKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CMapStringString::Get(uint8* szKey)
{
	return (char*)CMapStringBlock::Get((char*)szKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringString::Dump(void)
{
	SMapIterator	sIter;
	char*			szKey;
	char*			szValue;
	size			iKeySize;
	size			iValueSize;
	bool			bHasNext;
	CChars			sz;
	size			uiCount;

	uiCount = 0;
	sz.Init();
	bHasNext = StartIteration(&sIter, (void**)&szKey, &iKeySize, (void**)&szValue, &iValueSize);
	while (bHasNext)
	{
		sz.Append("[");
		sz.Append(szKey);
		sz.Append("] -> [");
		sz.Append(szValue);
		sz.Append("]");
		sz.AppendNewLine();
		bHasNext = Iterate(&sIter, (void**)&szKey, &iKeySize, (void**)&szValue, &iValueSize);
		uiCount++;
	}

	sz.DumpKill();
}

