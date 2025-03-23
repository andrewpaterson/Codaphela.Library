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
#include "BaseLib/Chars.h"
#include "MapIntString.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapIntString::Init(bool bOverwrite)
{
	mcMemory.Init();
	CMapIntTemplate<char*>::Init(bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapIntString::Init(CMallocator* pcMalloc, bool bOverwrite)
{
	mcMemory.Init();
	CMapIntTemplate<char*>::Init(pcMalloc, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapIntString::Kill(void)
{
	CMapIntTemplate<char*>::Kill();
	mcMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CMapIntString::Get(int iKey)
{
	char**	pp;

	pp = CMapIntTemplate<char*>::Get(iKey);
	if (pp)
	{
		return *pp;
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
bool CMapIntString::Put(int iKey, char* szValue)
{
	char*	szData;
	size	uiLength;

	uiLength = strlen(szValue) + 1;
	if (mbOverwrite)
	{
		szData = Get(iKey);
		if (szData)
		{
			mcMemory.Remove(szData);
		}
	}

	szData = (char*)mcMemory.Add(uiLength);
	memcpy(szData, szValue, uiLength);
	return CMapIntTemplate<char*>::Put(iKey, &szData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapIntString::Dump(void)
{
	SMapIterator	sIter;
	int*			piKey;
	char*			szValue;
	size			iKeySize;
	size			iValueSize;
	bool			bHasNext;
	CChars			sz;
	size			uiCount;

	uiCount = 0;
	sz.Init();
	bHasNext = StartIteration(&sIter, (void**)&piKey, &iKeySize, (void**)&szValue, &iValueSize);
	while (bHasNext)
	{
		sz.Append("[");
		sz.Append(*piKey);
		sz.Append("] -> [");
		sz.Append(szValue);
		sz.Append("]");
		sz.AppendNewLine();
		bHasNext = Iterate(&sIter, (void**)&piKey, &iKeySize, (void**)&szValue, &iValueSize);
		uiCount++;
	}

	sz.DumpKill();
}

