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
#include "ArrayIntAndPointer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayIntAndPointer::Add(void* pvData, int iInt)
{
	SIntAndPointer*	psPtr;

	psPtr = CArrayTemplate<SIntAndPointer>::Add();
	psPtr->iValue = iInt;
	psPtr->pvData = pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayIntAndPointer::AddIfUnique(void* pvData, int iInt)
{
	SIntAndPointer*	psPtr;
	int				i;

	for (i = 0; i < miUsedElements; i++)
	{
		psPtr = CArrayTemplate<SIntAndPointer>::Get(i);
		if ((psPtr->pvData == pvData) && (psPtr->iValue == iInt))
		{
			return;
		}
	}
	
	Add(pvData, iInt);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayIntAndPointer::Get(int iElementPos, void** pvData, int* iType)
{
	SIntAndPointer*	psPtr;

	psPtr = CArrayTemplate<SIntAndPointer>::SafeGet(iElementPos);
	if (psPtr)
	{
		*pvData = psPtr->pvData;
		*iType = psPtr->iValue;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayIntAndPointer::GetPtr(int iElementPos)
{
	SIntAndPointer*	psPtr;

	psPtr = CArrayTemplate<SIntAndPointer>::Get(iElementPos);
	return psPtr->pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayIntAndPointer::SafeGetPtr(int iElementPos)
{
	if ((iElementPos < 0) || (iElementPos >= miUsedElements))
	{
		return NULL;
	}
	else
	{
		return GetPtr(iElementPos);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayIntAndPointer::GetType(int iElementPos)
{
	SIntAndPointer*	psPtr;

	psPtr = CArrayTemplate<SIntAndPointer>::Get(iElementPos);
	return psPtr->iValue;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayIntAndPointer::SafeGetType(int iElementPos)
{
	if ((iElementPos < 0) || (iElementPos >= miUsedElements))
	{
		return -1;
	}
	else
	{
		return GetType(iElementPos);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayIntAndPointer::Set(int iElementPos, void* pvData, int iInt)
{
	SIntAndPointer*	psTypedPointer;

	psTypedPointer = CArrayTemplate<SIntAndPointer>::Get(iElementPos);
	psTypedPointer->iValue = iInt;
	psTypedPointer->pvData = pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayIntAndPointer::Remove(void* pv)
{
	int		iIndex;

	iIndex = FindWithKey((SIntAndPointer*)pv, 0, sizeof(void*));
	if (iIndex != -1)
	{
		RemoveAt(iIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayIntAndPointer::InsertIntoSorted(DataCompare fCompare, void* pvElement, int iInt)
{
	int				iPos;
	SIntAndPointer	sTypedPointer;
	BOOL			bExists;

	//This function never overwrites because it does not know how to free an existing object.

	bExists = FindInSorted(pvElement, fCompare, &iPos);

	sTypedPointer.iValue = iInt;
	sTypedPointer.pvData = pvElement;
	if (iPos < miUsedElements)
	{
		InsertAt(&sTypedPointer, iPos);
	}
	else
	{
		CArrayTemplate<SIntAndPointer>::Add(&sTypedPointer);
	}

	if (bExists)
	{
		return GetPtr(iPos+1);
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
BOOL CArrayIntAndPointer::FindInSorted(void* pvElement, DataCompare fCompare, int* piIndex)
{
	if (miUsedElements == 0)
	{
		*piIndex = 0;
		return FALSE;
	}
	return BinarySearch(pvElement, 0, miUsedElements - 1, fCompare, piIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayIntAndPointer::BinarySearch(void* pvData, int iLeft, int iRight, DataCompare fCompare, int* piIndex)
{
	int		iMiddle;
	int		iResultMiddle;
	void*	pvMiddle;

	iResultMiddle = 0;
	iMiddle = 0;
	while (iLeft <= iRight)
	{
		iMiddle = (iLeft + iRight) >> 1; //Divide by 2
		pvMiddle = GetPtr(iMiddle);
		iResultMiddle = fCompare(pvData, pvMiddle);
		if (iResultMiddle == 0)
		{
			*piIndex = iMiddle;
			return TRUE;
		}
		else if (iResultMiddle < 0)
		{
			iRight = iMiddle - 1;
		}
		else
		{
			iLeft = iMiddle + 1;
		}
	}

	if (iResultMiddle > 0)
	{
		*piIndex = (iMiddle + 1);
	}
	else
	{
		*piIndex = iMiddle;
	}
	return FALSE;
}

