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
#include "ArrayIntAndPointer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayIntAndPointer::Add(void* pvData, uint uiType)
{
	SIntAndPointer*	psPtr;

	psPtr = CArrayTemplate<SIntAndPointer>::Add();
	psPtr->uiType = uiType;
	psPtr->pvData = pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayIntAndPointer::AddIfUnique(void* pvData, uint uiType)
{
	SIntAndPointer*		psPtr;
	size				i;

	for (i = 0; i < miUsedElements; i++)
	{
		psPtr = CArrayTemplate<SIntAndPointer>::Get(i);
		if ((psPtr->pvData == pvData) && (psPtr->uiType == uiType))
		{
			return;
		}
	}
	
	Add(pvData, uiType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayIntAndPointer::Get(size uiIndex, void** pvData, uint* uiType)
{
	SIntAndPointer*	psPtr;

	psPtr = CArrayTemplate<SIntAndPointer>::SafeGet(uiIndex);
	if (psPtr)
	{
		*pvData = psPtr->pvData;
		*uiType = psPtr->uiType;
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayIntAndPointer::GetPtr(size uiIndex)
{
	SIntAndPointer*	psPtr;

	psPtr = CArrayTemplate<SIntAndPointer>::Get(uiIndex);
	return psPtr->pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CArrayIntAndPointer::SafeGetPtr(size uiIndex)
{
	if (uiIndex >= miUsedElements)
	{
		return NULL;
	}
	else
	{
		return GetPtr(uiIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayIntAndPointer::GetType(size uiIndex)
{
	SIntAndPointer*	psPtr;

	psPtr = CArrayTemplate<SIntAndPointer>::Get(uiIndex);
	return psPtr->uiType;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayIntAndPointer::SafeGetType(size uiIndex)
{
	if ((uiIndex < 0) || (uiIndex >= miUsedElements))
	{
		return -1;
	}
	else
	{
		return GetType(uiIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayIntAndPointer::Set(size uiIndex, void* pvData, uint uiType)
{
	SIntAndPointer*	psTypedPointer;

	psTypedPointer = CArrayTemplate<SIntAndPointer>::Get(uiIndex);
	psTypedPointer->uiType = uiType;
	psTypedPointer->pvData = pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayIntAndPointer::Remove(void* pv)
{
	size	iIndex;

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
void* CArrayIntAndPointer::InsertIntoSorted(DataCompare fCompare, void* pvElement, uint uiType)
{
	size			iPos;
	SIntAndPointer	sTypedPointer;
	bool			bExists;

	//This function never overwrites because it does not know how to free an existing object.

	bExists = FindInSorted(pvElement, fCompare, &iPos);

	sTypedPointer.uiType = uiType;
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
bool CArrayIntAndPointer::FindInSorted(void* pvElement, DataCompare fCompare, size* puiIndex)
{
	if (miUsedElements == 0)
	{
		*puiIndex = 0;
		return false;
	}
	return BinarySearch(pvElement, 0, miUsedElements - 1, fCompare, puiIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayIntAndPointer::BinarySearch(void* pvData, size iLeft, size iRight, DataCompare fCompare, size* puiIndex)
{
	size	iMiddle;
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
			*puiIndex = iMiddle;
			return true;
		}
		else if (iResultMiddle < 0)
		{
			if (iMiddle == 0)
			{
				break;
			}
			iRight = iMiddle - 1;
		}
		else
		{
			iLeft = iMiddle + 1;
		}
	}

	if (iResultMiddle > 0)
	{
		*puiIndex = (iMiddle + 1);
	}
	else
	{
		*puiIndex = iMiddle;
	}
	return false;
}

