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
	mcFreeLists.Init(&gcDataMemoryFreeListParams);
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
bool CListVariable::Get(int iIndex, void** pvData)
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
void* CListVariable::Get(int iIndex)
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
void* CListVariable::Get(int iIndex, size_t* puiElementSize)
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
int CListVariable::GetSize(int iIndex)
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
void* CListVariable::Add(size_t iElementSize)
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
void* CListVariable::Add(void* pvElement, size_t iElementSize)
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
void CListVariable::Set(int iIndex, void* pvData, int iElementSize)
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
void* CListVariable::Resize(int iIndex, int iElementSize)
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
void CListVariable::RemoveAt(int iIndex, bool bPreserveOrder)
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
int CListVariable::NumElements(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CListVariable::PrivateMalloc(int iElementSize)
{
	return mcFreeLists.Add(iElementSize);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListVariable::Remove(void* pvElement)
{
	int		iIndex;

	iIndex = IndexOf(pvElement);
	if (iIndex != -1)
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
int CListVariable::IndexOf(void* pvElement)
{
	int					i;
	SPointerAndSize*	psPtr;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		psPtr = mcArray.Get(i);
		if (psPtr->pvData == pvElement)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CListVariable::InsertAt(int iIndex, int iElementSize)
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
int CListVariable::InsertIntoSorted(DataCompare fCompare, void* pvElement, bool bOverwriteExisting, int iElementSize)
{
	int		iPos;
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
bool CListVariable::FindInSorted(void* pvElement, DataCompare fCompare, int* piIndex)
{
	SPointerAndSize	sPointerAndSize;

	sPointerAndSize.pvData = pvElement;
	sPointerAndSize.iSize = 0;
	return mcArray.FindInSorted(&sPointerAndSize, fCompare, piIndex);
}
