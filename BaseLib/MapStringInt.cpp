/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "MapStringInt.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int* CMapStringInt::GetWithKey(CChars* psKey)
{
	return (CMapStringTemplate<int>::GetWithKey(psKey));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int* CMapStringInt::GetWithKey(char* szKey)
{
	return (CMapStringTemplate<int>::GetWithKey(szKey));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMapStringInt::GetAtIndex(int iIndex, CChars** ppsKey, int** ppiData)
{
	return CMapStringTemplate<int>::GetAtIndex(iIndex, ppsKey, ppiData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringInt::Put(CChars* psKey, int iData)
{
	int*	piData;

	piData = CMapStringTemplate<int>::Put(psKey);
	if (!piData)
	{
		piData = CMapStringTemplate<int>::GetWithKey(psKey);
	}
	*piData = iData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringInt::Put(char* szKey, int iData)
{
	CChars	sz;

	sz.Fake(szKey);
	Put(&sz, iData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CMapStringInt::GetWithValue(int iData)
{
	int		i;
	int		iNum;
	CChars*	pszKey;
	int*	piValue;

	iNum = NumElements();

	for (i = 0; i < iNum; i++)
	{
		GetAtIndex(i, &pszKey, &piValue);
		if (*piValue == iData)
		{
			return pszKey;
		}
	}
	return NULL;
}

