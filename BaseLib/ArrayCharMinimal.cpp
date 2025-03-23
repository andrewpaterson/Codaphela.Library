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
#include "ArrayCharMinimal.h"
#include "PointerRemapper.h"
#include "FastFunctions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCharMinimal::Init(void)
{
	CArrayTemplateMinimal<char>::Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCharMinimal::Init(char* sz)
{
	size	iNumElements;

	iNumElements = strlen(sz) + 1;
	Allocate(iNumElements);
	memcpy(mpvArray, sz, iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCharMinimal::Allocate(size iNumElements)
{
	CArrayTemplateMinimal<char>::Allocate(iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCharMinimal::Allocate(char* sz)
{
	Init(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCharMinimal::Add(char c)
{
	char*	ci;

	ci = CArrayTemplateMinimal<char>::Add();
	*ci = c;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCharMinimal::AddIfUnique(char c)
{
	size	iElementNum;

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
size CArrayCharMinimal::Find(char c)
{
	size	j;

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
char CArrayCharMinimal::GetValue(size iElementPos)
{
	return *Get(iElementPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CArrayCharMinimal::GetArray(void)
{
	return mpvArray;
}

