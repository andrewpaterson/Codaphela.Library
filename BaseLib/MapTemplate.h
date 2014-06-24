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
#ifndef __MAP_TEMPLATE_H__
#define __MAP_TEMPLATE_H__
#include <stdlib.h>
#include "ArrayPointer.h"
#include "PointerRemapper.h"


template<class M>
class __CMapTemplate
{
public:
	int(*Func)(const void*, const void*);
	CArrayPointer	mcArray;
	int				miKeySize;

	void	Kill(void);
	void	Remove(M* psKey);
	BOOL	GetWithKey(M* psKey, void** ppvData, int* piDataSize);
	BOOL	GetWithKeyAssumeDuplicates(M* psKey, void** ppvData, int* piDataSize);
	BOOL	GetWithKeyNextDuplicate(M* psLastKey, int iLastIndex, void** ppvData, int* piDataSize);
	BOOL	GetAtIndex(int iIndex, M** ppsKey, void** ppsData, int* piDataSize);
	void*	Put(M* psKey, int iDataSize);
	void	Put(M* psKey, void* pvData, int iDataSize);
	void*	PutAllowDuplicates(M* psKey, int iDataSize);
	void	PutAllowDuplicates(M* psKey, void* pvData, int iDataSize);
	int		GetIndex(M* psKey);
	int		NumElements(void);

protected:
	M*		AllocateNode(int iDataSize);
	void	FreeNode(M* psKey);

	void*	GetDataForKey(M* psKey);  //Rename these and drop the Privates
	M*		GetKeyForData(void* pvData);
};


template<class M, class D>
class CMapTemplate : public __CMapTemplate<M>
{
public:
	void	Init(int iChunkSize);
	void	Init(int iChunkSize, int(* Func)(const void*, const void*));

	D*		GetWithKey(M* psKey);
	D*		GetWithKeyAssumeDuplicates(M* psKey);
	BOOL	GetWithKeyNextDuplicate(M* psLastKey, int iLastIndex, D** ppsData);
	BOOL	GetAtIndex(int iIndex, M** ppsKey, D** ppsData);

	D*		Put(M* psKey);
	void	Put(M* psKey, D* psData);

protected:
	D*		GetDataForKey(M* psKey);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CMapTemplate<M>::Kill(void)
{
	int		i;
	M*		psKey;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		psKey = (M*)mcArray.GetPtr(i);
		FreeNode(psKey);
	}

	mcArray.Kill();
	Func = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* __CMapTemplate<M>::GetDataForKey(M* psKey)
{
	if (psKey == NULL)
	{
		return NULL;
	}
	return RemapSinglePointer(psKey, miKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CMapTemplate<M>::GetKeyForData(void* pvData)
{
	if (pvData == NULL)
	{
		return NULL;
	}
	return (M*)(RemapSinglePointer(pvData, -miKeySize));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CMapTemplate<M>::AllocateNode(int iDataSize)
{
	return (M*)malloc(miKeySize + iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CMapTemplate<M>::FreeNode(M* psData)
{
	free(psData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CMapTemplate<M>::Remove(M* psKey)
{
	int	iIndex;
	M*	ps;

	iIndex = GetIndex(psKey);
	if (iIndex != -1)
	{
		ps = (M*)mcArray.GetPtr(iIndex);
		FreeNode(ps);
		mcArray.RemoveAt(iIndex, 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CMapTemplate<M>::GetWithKey(M* psSearch, void** ppvData, int* piDataSize)
{
	int		i;
	M*		psKey;
	BOOL	bResult;
	int		iSize;

	bResult =  mcArray.FindInSorted(psSearch, Func, &i);
	if (!bResult)
	{
		return FALSE;
	}
	bResult = mcArray.Get(i, (void**)&psKey, &iSize);
	*ppvData = GetDataForKey(psKey);
	*piDataSize = iSize;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CMapTemplate<M>::GetWithKeyAssumeDuplicates(M* psSearch, void** ppvData, int* piDataSize)
{
	int		i;
	M*		psKey;
	BOOL	bResult;
	int		iSize;

	bResult =  mcArray.FindInSortedFirstDuplicate(psSearch, Func, &i);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = mcArray.Get(i, (void**)&psKey, &iSize);
	*ppvData = GetDataForKey(psKey);
	*piDataSize = iSize;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* __CMapTemplate<M>::Put(M* psSearch, int iDataSize)
{
	M*		psKey;
	void*	pvData;
	int		iIndex;
	int		iCurrentSize;

	iIndex = GetIndex(psSearch);
	if (iIndex != -1)
	{
		GetAtIndex(iIndex, &psKey, &pvData, &iCurrentSize);
		if (iCurrentSize != iDataSize)
		{
			FreeNode(psKey);
			psKey = AllocateNode(iDataSize);
			mcArray.Set(iIndex, psKey, iDataSize);
		}
		pvData = GetDataForKey(psKey);
		return pvData;
	}
	else
	{
		psKey = AllocateNode(iDataSize);
		memcpy(psKey, psSearch, miKeySize);
		mcArray.InsertIntoSorted(Func, psKey, iDataSize);
		pvData = GetDataForKey(psKey);
		return pvData;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* __CMapTemplate<M>::PutAllowDuplicates(M* psSearch, int iDataSize)
{
	M*		psKey;
	void*	pvData;


	psKey = AllocateNode(iDataSize);
	memcpy(psKey, psSearch, miKeySize);
	mcArray.InsertIntoSorted(Func, psKey, iDataSize);
	pvData = GetDataForKey(psKey);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CMapTemplate<M>::PutAllowDuplicates(M* psKey, void* pvData, int iDataSize)
{
	void*	ps;

	ps = PutAllowDuplicates(psKey, iDataSize);
	memcpy(ps, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CMapTemplate<M>::Put(M* psSearch, void* pvData, int iDataSize)
{
	void*	ps;

	ps = Put(psSearch, iDataSize);
	memcpy(ps, pvData, iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CMapTemplate<M>::NumElements(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CMapTemplate<M>::GetAtIndex(int iIndex, M** ppsKey, void** ppsData, int* piDataSize)
{
	M*		psKey;
	void*	psData;
	int		iSize;
	BOOL	bResult;

	bResult = mcArray.Get(iIndex, (void**)&psKey, &iSize);
	if (!bResult)
	{
		return FALSE;
	}

	if (ppsKey)
	{
		*ppsKey = psKey;
	}

	if (ppsData)
	{
		psData = GetDataForKey(psKey);
		*ppsData = psData;
	}

	if (piDataSize)
	{
		*piDataSize = iSize;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CMapTemplate<M>::GetWithKeyNextDuplicate(M* psLastKey, int iLastIndex, void** ppvData, int* piDataSize)
{
	int		i;
	BOOL	bResult;
	int		iSize;
	M*		psKey;

	bResult = mcArray.FindInSortedNextDuplicate(psLastKey, iLastIndex, Func, &i);
	if (!bResult)
	{
		return FALSE;
	}
	bResult = mcArray.Get(i, (void**)&psKey, &iSize);
	*ppvData = GetDataForKey(psKey);
	*piDataSize = iSize;
	return TRUE;

}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CMapTemplate<M>::GetIndex(M* psKey)
{
	int		iPos;
	BOOL	bResult;

	bResult = mcArray.FindInSorted(psKey, Func, &iPos);
	if (bResult)
	{
		return iPos;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
D* CMapTemplate<M, D>::GetDataForKey(M* psKey)
{
	return (D*) __CMapTemplate<M>::GetDataForKey(psKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
void CMapTemplate<M, D>::Init(int iChunkSize)
{
	this->mcArray.Init(iChunkSize);
	this->Func = NULL;
	this->miKeySize = sizeof(M);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
void CMapTemplate<M, D>::Init(int iChunkSize, int(* Func)(const void*, const void*))
{
	Init(iChunkSize);
	this->Func = Func;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
D* CMapTemplate<M, D>::GetWithKey(M* psKey)
{
	void*	pvData;
	int		iSize;
	BOOL	bResult;

	bResult = __CMapTemplate<M>::GetWithKey(psKey, &pvData, &iSize);
	if (bResult)
	{
		return (D*)pvData;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
BOOL CMapTemplate<M, D>::GetAtIndex(int iIndex, M** ppsKey, D** ppsData)
{
	int	iSize;

	return __CMapTemplate<M>::GetAtIndex(iIndex, ppsKey, (void**)ppsData, &iSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
BOOL CMapTemplate<M, D>::GetWithKeyNextDuplicate(M* psLastKey, int iLastIndex, D** ppsData)
{
	int	iSize;

	return __CMapTemplate<M>::GetWithKeyNextDuplicate(psLastKey, iLastIndex, (void**)ppsData, &iSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
D* CMapTemplate<M, D>::GetWithKeyAssumeDuplicates(M* psKey)
{
	void*	pvData;
	int		iSize;
	BOOL	bResult;

	bResult = __CMapTemplate<M>::GetWithKeyAssumeDuplicates(psKey, &pvData, &iSize);
	if  (bResult)
	{
		return (D*)pvData;
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
template<class M, class D>
D* CMapTemplate<M, D>::Put(M* psKey)
{
	return (D*)__CMapTemplate<M>::Put(psKey, sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
void CMapTemplate<M, D>::Put(M* psKey, D* psData)
{
	__CMapTemplate<M>::Put(psKey, (void*)psData, sizeof(D));
}


#endif // __MAP_TEMPLATE_H__

