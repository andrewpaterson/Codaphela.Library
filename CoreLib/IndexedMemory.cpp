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
#include "IndexedMemory.h"
#include "BaseLib/FastFunctions.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/PointerFunctions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedMemory::Init(void)
{
	mcDatas.Init(512);
	ClearIndexCache();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedMemory::Kill(void)
{
	mcDatas.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedMemory::ClearIndexCache(void)
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
int CompareOIndexToTransactionData(const void* pv1, const void* pv2)
{
	OIndex*				psOI;
	SIndexedMemory*		psData2;

	psOI = (OIndex*)((SPointerAndSize*)pv1)->pvData;
	psData2 = (SIndexedMemory*)((SPointerAndSize*)pv2)->pvData;

	if (*psOI < psData2->oi)
	{
		return -1;
	}
	if (*psOI > psData2->oi)
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMemory::Add(OIndex oi, void* pvData, unsigned int uiSize)
{
	int					iPos;
	BOOL				bExists;
	SIndexedMemory*		psIndexedMemory;
	void*				pvCache;
	BOOL				bResult;

	bExists = mcDatas.FindInSorted(&oi, CompareOIndexToTransactionData, &iPos);
	if (!bExists)
	{
		bResult = PrivateAdd(oi, uiSize, &pvCache, iPos);
		if (bResult)
		{
			memcpy_fast(pvCache, pvData, uiSize);
		}
		return bResult;
	}
	else
	{
		psIndexedMemory = (SIndexedMemory*)mcDatas.Get(iPos);
		if (psIndexedMemory->IsRemoved())
		{
			return Set(oi, pvData, uiSize);
		}
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMemory::PrivateAdd(OIndex oi, unsigned int uiSize, void** ppvData, int iIndex)
{
	SIndexedMemory*	psIndexedMemory;
	unsigned int	iTotalSize;
	void*			pvDest;

	iTotalSize = uiSize + sizeof(SIndexedMemory);
	psIndexedMemory = (SIndexedMemory*)mcDatas.InsertAt(iIndex, iTotalSize);
	if (!psIndexedMemory)
	{
		return FALSE;
	}

	pvDest = RemapSinglePointer(psIndexedMemory, sizeof(SIndexedMemory));
	*ppvData = pvDest;

	psIndexedMemory->uiSize = uiSize;
	psIndexedMemory->oi = oi;
	ClearIndexCache();
	SetIndexCache(oi, iIndex);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMemory::AddRemoved(OIndex oi)
{
	SIndexedMemory*		psIndexedMemory;
	unsigned int		iTotalSize;
	void*				pvData;
	BOOL				bExists;
	unsigned int		uiSize;
	int					iIndex;

	bExists = PrivateGetDetail(oi, &pvData, &uiSize, &iIndex);

	iTotalSize = sizeof(SIndexedMemory);
	psIndexedMemory = (SIndexedMemory*)mcDatas.InsertAt(iIndex, iTotalSize);
	if (psIndexedMemory)
	{
		psIndexedMemory->uiSize = 0;
		psIndexedMemory->oi = oi;
		ClearIndexCache();
		SetIndexCache(oi, iIndex);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMemory::PrivateSetRemoved(OIndex oi, int iIndex)
{
	SIndexedMemory*	psIndexedMemory;
	unsigned int	iTotalSize;

	iTotalSize = sizeof(SIndexedMemory);
	psIndexedMemory = (SIndexedMemory*)mcDatas.Resize(iIndex, iTotalSize);
	if (psIndexedMemory)
	{
		psIndexedMemory->uiSize = 0;
		psIndexedMemory->oi = oi;
		SetIndexCache(oi, iIndex);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMemory::Set(OIndex oi, void* pvData, unsigned int uiSize)
{
	SIndexedMemory*		psIndexedMemory;
	unsigned int		iTotalSize;
	int					iIndex;
	void*				pvCurrent;
	unsigned int		uiCurrentSize;
	BOOL				bExists;

	if (uiSize > 0)
	{
		bExists = PrivateGetDetail(oi, &pvCurrent, &uiCurrentSize, &iIndex);
		if (!bExists)
		{
			return FALSE;
		}

		iTotalSize = uiSize + sizeof(SIndexedMemory);
		if (uiCurrentSize != uiSize)
		{
			psIndexedMemory = (SIndexedMemory*)mcDatas.Resize(iIndex, iTotalSize);
			if (!psIndexedMemory)
			{
				return FALSE;
			}
			psIndexedMemory->uiSize = uiSize;
			pvCurrent = RemapSinglePointer(psIndexedMemory, sizeof(SIndexedMemory));
		}

		memcpy_fast(pvCurrent, pvData, uiSize);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMemory::PrivateGetDetail(OIndex oi, void** ppvData, unsigned int* puiSize, int* piIndex)
{
	if (asIndexCache[0].oi == oi)
	{
		*piIndex = asIndexCache[0].iIndex;
		return GetDirect(asIndexCache[0].iIndex, ppvData, puiSize);
	}
	else if (asIndexCache[1].oi == oi)
	{
		*piIndex = asIndexCache[1].iIndex;
		return GetDirect(asIndexCache[1].iIndex, ppvData, puiSize);
	}
	else if (asIndexCache[2].oi == oi)
	{
		*piIndex = asIndexCache[2].iIndex;
		return GetDirect(asIndexCache[2].iIndex, ppvData, puiSize);
	}
	else if (asIndexCache[3].oi == oi)
	{
		*piIndex = asIndexCache[3].iIndex;
		return GetDirect(asIndexCache[3].iIndex, ppvData, puiSize);
	}
	else
	{
		return GetNoCacheTest(oi, ppvData, puiSize, piIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMemory::GetDetail(OIndex oi, void** ppvData, unsigned int* puiSize)
{
	int		iIndex;

	return PrivateGetDetail(oi, ppvData, puiSize, &iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMemory::GetDirect(int iIndex, void** ppvData, unsigned int* puiSize)
{
	SIndexedMemory*	psIndexData;

	mcDatas.Get(iIndex, (void**)&psIndexData);
	if (!psIndexData->IsRemoved())
	{
		*ppvData = RemapSinglePointer(psIndexData, sizeof(SIndexedMemory));
		*puiSize = psIndexData->uiSize;
	}
	else
	{
		*ppvData = NULL;
		*puiSize = 0;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMemory::GetNoCacheTest(OIndex oi, void** ppvData, unsigned int* puiSize, int* piIndex)
{
	BOOL				bExists;
	SIndexedMemory*		psIndexedMemory;

	bExists = mcDatas.FindInSorted(&oi, CompareOIndexToTransactionData, piIndex);
	if (!bExists)
	{
		return FALSE;
	}

	SetIndexCache(oi, *piIndex);
	psIndexedMemory = (SIndexedMemory*)mcDatas.Get(*piIndex);
	*puiSize = psIndexedMemory->uiSize;
	if (!psIndexedMemory->IsRemoved())
	{
		*ppvData = RemapSinglePointer(psIndexedMemory, sizeof(SIndexedMemory));
	}
	else
	{
		*ppvData = NULL;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMemory::Get(OIndex oi, void* pvDest)
{
	BOOL			bResult;
	void*			pvData;
	unsigned int	uiSize;

	bResult = GetDetail(oi, &pvData, &uiSize);
	if (bResult)
	{
		memcpy_fast(pvDest, pvData, uiSize);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedMemory::Size(OIndex oi)
{
	BOOL			bResult;
	void*			pvData;
	unsigned int	uiSize;

	bResult = GetDetail(oi, &pvData, &uiSize);
	if (bResult)
	{
		return uiSize;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMemory::Remove(OIndex oi)
{
	BOOL				bExists;
	SIndexedMemory*		psIndexedMemory;
	unsigned int		uiSize;
	int					iIndex;
	void*				pvData;

	bExists = PrivateGetDetail(oi, &pvData, &uiSize, &iIndex);
	if (!bExists)
	{
		//If one removes something that doesn't exist then it must be added and marked as removed.
		return AddRemoved(oi);
	}
	else
	{
		mcDatas.Get(iIndex, (void**)&psIndexedMemory);
		if (psIndexedMemory->IsRemoved())
		{
			//Already removed.
			return FALSE;
		}

		return PrivateSetRemoved(oi, iIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexedMemory* CIndexedMemory::GetIndexedData(int iIndexedDataIndex)
{
	return (SIndexedMemory*)mcDatas.Get(iIndexedDataIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedMemory::SetIndexCache(OIndex oi, int iIndex)
{
	asIndexCache[miNext].oi = oi;
	asIndexCache[miNext].iIndex = iIndex;
	miNext++;
	if (miNext == 4)
	{
		miNext = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedMemory::NumDatas(void)
{
	return mcDatas.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedMemory::TestOrder(void)
{
	int				i;
	SIndexedMemory*	psData;
	OIndex			iLast;

	iLast = -1;
	for (i = 0; i < mcDatas.NumElements(); i++)
	{
		psData = (SIndexedMemory*)mcDatas.Get(i);
		if (psData->oi <= iLast)
		{
			return FALSE;
		}
		iLast = psData->oi;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexedMemory::GetData(SIndexedMemory* psIndexedMemory)
{
	return RemapSinglePointer(psIndexedMemory, sizeof(SIndexedMemory));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedMemory::Dump(void)
{
	int					iNumElements;
	int					i;
	SIndexedMemory*		psIndexedMemory;
	void*				pvDest;
	CChars				sz;

	sz.Init();
	iNumElements = mcDatas.NumElements();

	sz.Append("Transaction Data (");
	sz.Append(iNumElements);
	sz.Append(") (Memory)\n-----------------\n");

	for (i = 0; i < iNumElements; i++)
	{
		mcDatas.Get(i, (void**)&psIndexedMemory);
		pvDest = RemapSinglePointer(psIndexedMemory, sizeof(SIndexedMemory));

		sz.Append("Index[");
		sz.Append((int)psIndexedMemory->oi);
		sz.Append("] Size[");
		sz.Append((int)psIndexedMemory->uiSize);
		sz.Append("] ");
		if (psIndexedMemory->uiSize > 0)
		{
			sz.Append("\"");
			sz.AppendData((char*)pvDest, 80);
			sz.Append("\" ");
		}
		if (psIndexedMemory->IsRemoved())
		{
			sz.Append("(Removed)");
		}
		sz.Append("\n");
	}
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}

