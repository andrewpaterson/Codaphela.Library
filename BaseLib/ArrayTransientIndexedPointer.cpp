/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "ArrayTransientIndexedPointer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STransientIndexedPointer::Init(OIndex oi, unsigned int uiSize)
{
	iFileIndex = -1;
	iIndexInFile = -1;
	sIndexedMemory.oi = oi;
	sIndexedMemory.uiSize = uiSize;
	pvCache = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL STransientIndexedPointer::IsRemoved(void)
{
	return sIndexedMemory.uiSize == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayTransientIndexedPointer::Init(int iChunkSize)
{
	__CArrayTransientIndexedPointer::Init(iChunkSize);
	ClearIndexCache();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayTransientIndexedPointer::Kill(void)
{
	__CArrayTransientIndexedPointer::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareOIndexToTransientIndexedPointer(const void* pv1, const void* pv2)
{
	OIndex*						psOI;
	STransientIndexedPointer*	psPointer2;

	psOI = (OIndex*)pv1;

	psPointer2 = (STransientIndexedPointer*)pv2;

	if (*psOI < psPointer2->sIndexedMemory.oi)
	{
		return -1;
	}
	if (*psOI > psPointer2->sIndexedMemory.oi)
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayTransientIndexedPointer::ClearIndexCache(void)
{
	int		i;

	miNext = 0;
	for (i = 0; i < 4; i++)
	{
		asIndexCache[i].oi = INVALID_O_INDEX;
		asIndexCache[i].iIndex = -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STransientIndexedPointer* CArrayTransientIndexedPointer::Add(OIndex oi, int iIndex)
{
	STransientIndexedPointer*	psPointer;

	psPointer = __CArrayTransientIndexedPointer::InsertAt(iIndex);
	ClearIndexCache();
	SetIndexCache(oi, iIndex);
	return psPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayTransientIndexedPointer::GetNoCacheTest(OIndex oi, STransientIndexedPointer** ppsPointer, int* piIndex)
{
	BOOL						bResult;
	STransientIndexedPointer*	psPointer;

	bResult = FindInSorted((STransientIndexedPointer*)&oi, CompareOIndexToTransientIndexedPointer, piIndex);
	if (bResult)
	{
		psPointer = __CArrayTransientIndexedPointer::Get(*piIndex);
		*ppsPointer = psPointer;
		SetIndexCache(oi, *piIndex);
	}
	else
	{
		*ppsPointer = NULL;
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayTransientIndexedPointer::Get(OIndex oi, STransientIndexedPointer** ppsPointer, int* piIndex)
{
	if (asIndexCache[0].oi == oi)
	{
		*piIndex = asIndexCache[0].iIndex;
		return GetDirect(asIndexCache[0].iIndex, ppsPointer);
	}
	else if (asIndexCache[1].oi == oi)
	{
		*piIndex = asIndexCache[1].iIndex;
		return GetDirect(asIndexCache[1].iIndex, ppsPointer);
	}
	else if (asIndexCache[2].oi == oi)
	{
		*piIndex = asIndexCache[2].iIndex;
		return GetDirect(asIndexCache[2].iIndex, ppsPointer);
	}
	else if (asIndexCache[3].oi == oi)
	{
		*piIndex = asIndexCache[3].iIndex;
		return GetDirect(asIndexCache[3].iIndex, ppsPointer);
	}
	else
	{
		return GetNoCacheTest(oi, ppsPointer, piIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayTransientIndexedPointer::GetDirect(int iIndex, STransientIndexedPointer** ppsPointer)
{
	*ppsPointer =  __CArrayTransientIndexedPointer::Get(iIndex);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayTransientIndexedPointer::SetIndexCache(OIndex oi, int iIndex)
{
	asIndexCache[miNext].oi = oi;
	asIndexCache[miNext].iIndex = iIndex;
	miNext++;
	if (miNext == 4)
	{
		miNext = 0;
	}
}

