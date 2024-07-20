/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "ListVariable.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListVariable::Init(void)
{
	mcArray.Init();
	mcFreeLists.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListVariable::ReInit(void)
{
	mcArray.ReInit();
	mcFreeLists.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListVariable::Kill(void)
{
	mcFreeLists.Kill();
	mcArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListVariable::Get(size iIndex, void** pvData)
{
	SPointerAndSize*	ps;

	ps = mcArray.SafeGet(iIndex);
	*pvData = ps->pvData;
	return ps != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CListVariable::Get(size iIndex)
{
	SPointerAndSize*	ps;

	ps = mcArray.SafeGet(iIndex);
	if (ps)
	{
		return ps->pvData;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CListVariable::Get(size iIndex, size* puiElementSize)
{
	SPointerAndSize* ps;

	ps = mcArray.SafeGet(iIndex);
	if (ps)
	{
		SafeAssign(puiElementSize, ps->iSize);
		return ps->pvData;
	}
	else
	{
		SafeAssign(puiElementSize, 0);
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CListVariable::GetSize(size iIndex)
{
	SPointerAndSize*	ps;

	ps = mcArray.SafeGet(iIndex);
	if (ps)
	{
		return ps->iSize;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CListVariable::Add(size iElementSize)
{
	SPointerAndSize*	psType;

	if (iElementSize == 0)
	{
		return NULL;
	}

	psType = mcArray.Add();
	psType->iSize = iElementSize;


	psType->pvData = PrivateMalloc(iElementSize);
	return psType->pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CListVariable::Add(void* pvElement, size iElementSize)
{
	void*	pvDest;

	pvDest = Add(iElementSize);
	if (pvDest)
	{
		memcpy(pvDest, pvElement, iElementSize);
	}
	
	return pvDest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListVariable::Set(size iIndex, void* pvData, size iElementSize)
{
	SPointerAndSize*	psType;

	if (iElementSize == 0)
	{
		return;
	}

	psType = mcArray.SafeGet(iIndex);
	if (!psType)
	{
		return;
	}

	if (psType->iSize == iElementSize)
	{
		memcpy(psType->pvData, pvData, iElementSize);
	}
	else
	{
		PrivateFree(psType);
		psType->pvData = PrivateMalloc(iElementSize);
		psType->iSize = iElementSize;
		memcpy(psType->pvData, pvData, iElementSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CListVariable::Resize(size iIndex, size iElementSize)
{
	SPointerAndSize*	psType;

	if (iElementSize == 0)
	{
		return NULL;
	}

	psType = mcArray.SafeGet(iIndex);
	if (!psType)
	{
		return NULL;
	}

	PrivateFree(psType);
	psType->pvData = PrivateMalloc(iElementSize);
	psType->iSize = iElementSize;
	return psType->pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListVariable::RemoveAt(size iIndex, bool bPreserveOrder)
{
	SPointerAndSize*	psType;

	psType = mcArray.SafeGet(iIndex);
	if (!psType)
	{
		return;
	}

	PrivateFree(psType);

	//Second remove the TypePointer from the array.
	mcArray.RemoveAt(iIndex, bPreserveOrder);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListVariable::PrivateFree(SPointerAndSize* psType)
{
	mcFreeLists.Remove(psType->pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CListVariable::NumElements(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CListVariable::PrivateMalloc(size iElementSize)
{
	return mcFreeLists.Add(iElementSize);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListVariable::Remove(void* pvElement)
{
	size		iIndex;

	iIndex = IndexOf(pvElement);
	if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		RemoveAt(iIndex);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CListVariable::IndexOf(void* pvElement)
{
	size				i;
	SPointerAndSize*	psPtr;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		psPtr = mcArray.Get(i);
		if (psPtr->pvData == pvElement)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CListVariable::InsertAt(size iIndex, size iElementSize)
{
	SPointerAndSize*	psType;

	if (iElementSize == 0)
	{
		return NULL;
	}

	psType = mcArray.InsertAt(iIndex);
	psType->iSize = iElementSize;
	psType->pvData = PrivateMalloc(iElementSize);

	return psType->pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CListVariable::InsertIntoSorted(DataCompare fCompare, void* pvElement, bool bOverwriteExisting, size iElementSize)
{
	size	iPos;
	bool	bExists;
	void*	pvData;

	bExists = FindInSorted(pvElement, fCompare, &iPos);
	if (iPos < mcArray.NumElements())
	{
		if ((!bExists) && (!bOverwriteExisting))
		{
			pvData = InsertAt(iPos, iElementSize);
			if (pvData)
			{
				memcpy(pvData, pvElement, iElementSize);
			}
		}
		else
		{
			Set(iPos, pvElement, iElementSize);
		}
	}
	else
	{
		pvData = Add(iElementSize);
		if (pvData)
		{
			memcpy(pvData, pvElement, iElementSize);
		}
	}
	return iPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListVariable::FindInSorted(void* pvElement, DataCompare fCompare, size* piIndex)
{
	SPointerAndSize	sPointerAndSize;

	sPointerAndSize.pvData = pvElement;
	sPointerAndSize.iSize = 0;
	return mcArray.FindInSorted(&sPointerAndSize, fCompare, piIndex);
}

