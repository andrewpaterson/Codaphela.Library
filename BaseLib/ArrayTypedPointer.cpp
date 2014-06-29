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
#include "ArrayTypedPointer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayTypedPointer::Add(void* pvData, int iType)
{
	STypedPointer*	psPtr;

	psPtr = __CArrayTypedPointer::Add();
	psPtr->iType = iType;
	psPtr->pvData = pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayTypedPointer::AddIfUnique(void* pvData, int iType)
{
	STypedPointer*	psPtr;
	int				i;

	for (i = 0; i < miUsedElements; i++)
	{
		psPtr = __CArrayTypedPointer::Get(i);
		if ((psPtr->pvData == pvData) && (psPtr->iType == iType))
		{
			return;
		}
	}
	
	Add(pvData, iType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayTypedPointer::Get(int iElementPos, void** pvData, int* iType)
{
	STypedPointer*	psPtr;

	psPtr = __CArrayTypedPointer::SafeGet(iElementPos);
	if (psPtr)
	{
		*pvData = psPtr->pvData;
		*iType = psPtr->iType;
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
void* CArrayTypedPointer::GetPtr(int iElementPos)
{
	STypedPointer*	psPtr;

	psPtr = __CArrayTypedPointer::Get(iElementPos);
	return psPtr->pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayTypedPointer::SafeGetPtr(int iElementPos)
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
int CArrayTypedPointer::GetType(int iElementPos)
{
	STypedPointer*	psPtr;

	psPtr = __CArrayTypedPointer::Get(iElementPos);
	return psPtr->iType;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayTypedPointer::SafeGetType(int iElementPos)
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
void CArrayTypedPointer::Set(int iElementPos, void* pvData, int iType)
{
	STypedPointer*	psTypedPointer;

	psTypedPointer = __CArrayTypedPointer::Get(iElementPos);
	psTypedPointer->iType = iType;
	psTypedPointer->pvData = pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayTypedPointer::Remove(void* pv)
{
	int		iIndex;

	iIndex = FindWithKey((STypedPointer*)pv, 0, sizeof(void*));
	if (iIndex != -1)
	{
		RemoveAt(iIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayTypedPointer::InsertIntoSorted(int(* Func)(const void*, const void*), void* pvElement, int iType)
{
	int				iPos;
	STypedPointer	sTypedPointer;
	BOOL			bExists;

	//This function never overwrites because it does not know how to free an existing object.

	bExists = FindInSorted(pvElement, Func, &iPos);

	sTypedPointer.iType = iType;
	sTypedPointer.pvData = pvElement;
	if (iPos < miUsedElements)
	{
		InsertAt(&sTypedPointer, iPos);
	}
	else
	{
		__CArrayTypedPointer::Add(&sTypedPointer);
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
BOOL CArrayTypedPointer::FindInSorted(void* pvElement, int(* Func)(const void*, const void*), int* piIndex)
{
	if (miUsedElements == 0)
	{
		*piIndex = 0;
		return FALSE;
	}
	return BinarySearch(pvElement, 0, miUsedElements - 1, Func, piIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayTypedPointer::FindInSortedFirstDuplicate(void* pvElement, int(* Func)(const void*, const void*), int* piIndex)
{
	BOOL	bResult;
	int		iIndex;
	int		iResult;

	if (miUsedElements == 0)
	{
		*piIndex = 0;
		return FALSE;
	}
	bResult = BinarySearch(pvElement, 0, miUsedElements - 1, Func, piIndex);
	if (!bResult)
	{
		*piIndex = 0;
		return FALSE;
	}

	//This searches back up the array for the first occurence of pvElement.
	iIndex = *piIndex - 1;
	while (iIndex >= 0)
	{
		iResult = Func(pvElement, GetPtr(iIndex));
		if (iResult != 0)
		{
			*piIndex = iIndex+1;
			return TRUE;
		}
		iIndex--;
	}
	*piIndex = 0;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayTypedPointer::BinarySearch(void* pvData, int iLeft, int iRight, int(* Func)(const void*, const void*), int* piIndex)
{
	int		iMiddle;
	int		iResultMiddle;
	void*	pvMiddle;

	iResultMiddle = 0;
	while (iLeft <= iRight)
	{
		iMiddle = (iLeft + iRight) >> 1; //Divide by 2
		pvMiddle = GetPtr(iMiddle);
		iResultMiddle = Func(pvData, pvMiddle);
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


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayTypedPointer::FindInSortedNextDuplicate(void* pvLastElement, int iLastIndex, int(* Func)(const void*, const void*), int* piIndex)
{
	int	iIndex;
	int	iResult;

	iIndex = iLastIndex+1;
	if (iIndex >= miUsedElements)
	{
		return FALSE;
	}

	iResult = Func(pvLastElement, GetPtr(iIndex));
	if (iResult == 0)
	{
		*piIndex = iIndex;
		return TRUE;
	}
	return FALSE;
}

