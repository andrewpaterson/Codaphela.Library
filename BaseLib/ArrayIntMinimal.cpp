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
#include "ArrayIntMinimal.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayIntMinimal::Add(int i)
{
	int*	pi;

	pi = CArrayTemplateMinimal<int>::Add();
	*pi = i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayIntMinimal::AddIfUnique(int i)
{
	int iElementNum;

	iElementNum = Find(i);
	if (iElementNum == -1)
	{
		Add(i);
	}
	return iElementNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayIntMinimal::Find(int i)
{
	int	j;

	for (j = 0; j < miUsedElements; j++)
	{
		if (*Get(j) == i)
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
int CArrayIntMinimal::GetValue(int iElementPos)
{
	return *Get(iElementPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayIntMinimal::QuickSort(void)
{
	CArrayTemplateMinimal<int>::QuickSort(&ComparePrimitive<int>);
}

