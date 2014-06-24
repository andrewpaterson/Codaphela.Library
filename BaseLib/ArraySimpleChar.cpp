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
#include "ArraySimpleChar.h"
#include "PointerRemapper.h"
#include "FastFunctions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArraySimpleChar::Init(void)
{
	__CArraySimpleChar::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArraySimpleChar::Init(char* sz)
{
	int	iNumElements;

	iNumElements = (int)strlen(sz) + 1;
	Allocate(iNumElements);
	memcpy(mpvArray, sz, iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArraySimpleChar::Allocate(int iNumElements)
{
	__CArraySimpleChar::Allocate(iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArraySimpleChar::Allocate(char* sz)
{
	Init(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArraySimpleChar::Add(char c)
{
	char*	ci;

	ci = __CArraySimpleChar::Add();
	*ci = c;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArraySimpleChar::AddIfUnique(char c)
{
	int iElementNum;

	iElementNum = Find(c);
	if (iElementNum == -1)
	{
		Add(c);
	}
	return iElementNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArraySimpleChar::Find(char c)
{
	int	j;

	for (j = 0; j < miUsedElements; j++)
	{
		if (*Get(j) == c)
		{
			return j;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CArraySimpleChar::GetValue(int iElementPos)
{
	return *Get(iElementPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CArraySimpleChar::GetArray(void)
{
	return mpvArray;
}
