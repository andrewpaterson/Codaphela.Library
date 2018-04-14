/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "ListVariable.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListVariable::Init(void)
{
	Init(1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListVariable::Init(int iChunkSize)
{
	mcArray.Init(iChunkSize);
	mcFreeLists.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListVariable::ReInit(void)
{
	int		iChunkSize;

	iChunkSize = mcArray.ChunkSize();
	Kill();
	Init(iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListVariable::Kill(void)
{
	int					i;
	SPointerAndSize*	psData;
	int					iElementSize;

	//If we know the types they can be removed faster.
	for (i = 0; i < mcArray.NumElements(); i++)
	{
		psData = mcArray.Get(i);
		iElementSize = psData->iSize;
		PrivateFree(psData->pvData, iElementSize);
	}

	mcFreeLists.Kill();
	mcArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CListVariable::Get(int iIndex, void** pvData)
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
	return ps->pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CListVariable::Add(int iElementSize)
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
void* CListVariable::Add(void* pvElement, int iElementSize)
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
void CListVariable::RemoveAt(int iIndex, int bPreserveOrder)
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
	PrivateFree(psType->pvData, psType->iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListVariable::PrivateFree(void* pvElement, int iElementSize)
{
	CFreeListMaybe*		psFreelist;

	if (iElementSize != 0)
	{
		psFreelist = GetFreeListForSize(iElementSize);
		psFreelist->Remove(pvElement);
		return;
	}
	else
	{
		psFreelist = mcFreeLists.GetHead();
		while (psFreelist)
		{
			if (psFreelist->SafeRemove(pvElement))
			{
				return;
			}
			psFreelist = mcFreeLists.GetNext(psFreelist);
		}
	}
	//Couldn't find it in any freelists.
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
CFreeListMaybe* CListVariable::GetFreeListForSize(int iSize)
{
	CFreeListMaybe*		psFreelist;

	psFreelist = mcFreeLists.GetHead();
	while (psFreelist)
	{
		if (psFreelist->GetElementSize() == iSize)
		{
			return psFreelist;
		}
		psFreelist = mcFreeLists.GetNext(psFreelist);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CListVariable::PrivateMalloc(int iElementSize)
{
	CFreeListMaybe* psFreelist;

	psFreelist = GetFreeListForSize(iElementSize);
	if (psFreelist == NULL)
	{
		psFreelist = mcFreeLists.InsertAfterTail();
		psFreelist->Init(iElementSize);
	}
	return psFreelist->Add();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CListVariable::Remove(void* pvElement)
{
	int		iIndex;

	iIndex = IndexOf(pvElement);
	if (iIndex != -1)
	{
		RemoveAt(iIndex);
		return TRUE;
	}
	return FALSE;
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
int CListVariable::InsertIntoSorted(int(*Func)(const void*, const void*), void* pvElement, BOOL bOverwriteExisting, int iElementSize)
{
	int		iPos;
	BOOL	bExists;
	void*	pvData;

	bExists = FindInSorted(pvElement, Func, &iPos);
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
BOOL CListVariable::FindInSorted(void* pvElement, int(*Func)(const void*, const void*), int* piIndex)
{
	SPointerAndSize	sPointerAndSize;

	sPointerAndSize.pvData = pvElement;
	sPointerAndSize.iSize = 0;
	return mcArray.FindInSorted(&sPointerAndSize, Func, piIndex);
}
