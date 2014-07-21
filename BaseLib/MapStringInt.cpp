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
int* CMapStringInt::Get(char* szKey)
{
	return (CMapStringTemplate<int>::Get(szKey));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMapStringInt::Put(char* szKey, int iData)
{
	return CMapStringTemplate<int>::Put(szKey, &iData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CMapStringInt::GetWithValue(int iData)
{
	SMapIterator	sIter;
	char*			szKey;
	int*			piData;
	BOOL			bResult;

	bResult = StartIteration(&sIter, (void**)&szKey, (void**)&piData);
	while (bResult)
	{
		if (*piData == iData)
		{
			return szKey;
		}
		bResult = Iterate(&sIter, (void**)&szKey, (void**)&piData);
	}
	return NULL;
}

