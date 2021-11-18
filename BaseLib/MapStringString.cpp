/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
BOOL CMapStringString::Put(char* szKey, char* szValue)
{
	int		iStrLen;
	BOOL	bResult;

	if (szValue == NULL)
	{
		return FALSE;
	}

	iStrLen = strlen(szValue);
	bResult = CMapStringBlock::Put(szKey, szValue, iStrLen + 1);
	return bResult;
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
void CMapStringString::Dump(void)
{
	SMapIterator	sIter;
	char*			szKey;
	char*			szValue;
	int				iKeySize;
	int				iValueSize;
	BOOL			bHasNext;
	CChars			sz;
	size_t			uiCount;

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

