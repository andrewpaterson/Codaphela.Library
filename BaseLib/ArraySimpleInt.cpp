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
#include "ArraySimpleInt.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArraySimpleInt::Add(int i)
{
	int*	pi;

	pi = CArraySimple<int>::Add();
	*pi = i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArraySimpleInt::AddIfUnique(int i)
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
int CArraySimpleInt::Find(int i)
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
int CArraySimpleInt::GetValue(int iElementPos)
{
	return *Get(iElementPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArraySimpleInt::QuickSort(void)
{
	CArraySimple<int>::QuickSort(&ComparePrimitive<int>);
}

