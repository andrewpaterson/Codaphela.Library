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
#ifndef __ARRAY_EMBEDDED_H__
#define __ARRAY_EMBEDDED_H__
#include "ArrayTemplate.h"
#include "ConstructorCall.h"


template<class M, int I>
class CArrayTemplateEmbedded : protected CPostMalloc<M>
{
protected:
	int		miUsedElements;
	int		miChunkSize;

	union
	{
		CArrayTemplate<M>	mcArray;
		M					mam[I];
	};

protected:
	void	BecomeArray(int iUsedElements);
	void	BecomeEmbedded(int iUsedElements);

public:
	//Appease the constructor gods.
	CArrayTemplateEmbedded() {}
	CArrayTemplateEmbedded(CArrayTemplateEmbedded &&) {}
	CArrayTemplateEmbedded(const CArrayTemplateEmbedded&) = default;
	CArrayTemplateEmbedded& operator = (const CArrayTemplateEmbedded&) = default;

public:
	void	Init(void);
	void	Kill(void);
	void	ReInit(void);

	M*		Add(void);
	M*		Add(M* pData);
	void	AddPtr(void* pv);
	int 	ByteSize(void);
	int		EmbeddedSize(void);
	int 	Find(M* pvElement);
	M*		Get(int iIndex);
	M*		GetData(void);
	int		GetIndex(M* pvElement);
	void*	GetPtr(int iIndex);
	int		AddNum(int iNumElements);
	int		Resize(int iNumElements);
	M* 		InsertAt(int iIndex);
	M*		InsertAt(M* pData, int iIndex);
	BOOL	IsEmpty(void);
	BOOL	IsNotEmpty(void);
	BOOL	IsEmbedded(void);
	BOOL	IsArray(void);
	int		NumElements(void);
	BOOL	Remove(M* pvElement, int bPreserveOrder = TRUE);
	void 	RemoveAt(int iIndex, int bPreserveOrder = TRUE);
	M*		SafeGet(int iIndex);
	void	Copy(CArrayTemplateEmbedded<M, I>* pcArrayTemplateEmbedded);
	BOOL	TestInternalConsistency(void);
	void 	Zero(void);
	BOOL	Write(CFileWriter* pcFileWriter);
	BOOL	Read(CFileReader* pcFileReader);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
void CArrayTemplateEmbedded<M, I>::Init(void)
{
	//int		iEmbeddedSize;
	//int		iArraySize;
	//
	//iEmbeddedSize = sizeof(M) * I;
	//iArraySize = sizeof(CArrayTemplate<M>);

	//if (iEmbeddedSize < iArraySize)
	//{
	//	this->miChunkSize = iArraySize / sizeof(M);
	//}
	//else
	//{
		this->miChunkSize = I;
	//}

	this->miUsedElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
void CArrayTemplateEmbedded<M, I>::Kill(void)
{
	if (IsArray())
	{
		mcArray.Kill();
	}
	miUsedElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
void CArrayTemplateEmbedded<M, I>::ReInit(void)
{
	Kill();
	Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
BOOL CArrayTemplateEmbedded<M, I>::IsEmbedded(void)
{
	return miUsedElements <= miChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
BOOL CArrayTemplateEmbedded<M, I>::IsArray(void)
{
	return miUsedElements > miChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
void CArrayTemplateEmbedded<M, I>::BecomeArray(int iUsedElements)
{
	M	am[I];  //Problem

	memcpy(am, mam, miUsedElements * sizeof(M));
	mcArray.Init();
	mcArray.AddNum(iUsedElements);
	memcpy(mcArray.GetData(), am, miUsedElements * sizeof(M));
	miUsedElements = iUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
void CArrayTemplateEmbedded<M, I>::BecomeEmbedded(int iUsedElements)
{
	M	am[I];  //Problem

	memcpy(am, mcArray.GetData(), iUsedElements * sizeof(M));
	mcArray.Kill();

	memcpy(mam, am, iUsedElements * sizeof(M));
	miUsedElements = iUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
M* CArrayTemplateEmbedded<M, I>::Add(void)
{
	M*	pv;
	if (IsEmbedded())
	{
		if (miUsedElements + 1 > miChunkSize)
		{
			BecomeArray(miUsedElements + 1);
			pv = mcArray.Tail();
		}
		else
		{
			miUsedElements++;
			pv = &mam[miUsedElements-1];
		}
	}
	else
	{
		miUsedElements++;
		pv = mcArray.Add();
	}
	return PostMalloc(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
M* CArrayTemplateEmbedded<M, I>::Add(M* pData)
{
	M*	pAdded;

	pAdded = Add();
	memcpy(pAdded, pData, sizeof(M));
	return pAdded;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
void CArrayTemplateEmbedded<M, I>::AddPtr(void* pv)
{
	void** pvTemp;

	pvTemp = (void**)Add();
	*pvTemp = pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
int CArrayTemplateEmbedded<M, I>::ByteSize(void)
{
	if (IsEmbedded())
	{
		miUsedElements * sizeof(M);
	}
	else
	{
		return mcArray.ByteSize();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
int CArrayTemplateEmbedded<M, I>::EmbeddedSize(void)
{
	return miChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
M* CArrayTemplateEmbedded<M, I>::Get(int iIndex)
{
	if (IsEmbedded())
	{
		if (iIndex < miUsedElements)
		{
			return &mam[iIndex];
		}
		return NULL;
	}
	else
	{
		return mcArray.SafeGet(iIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
M* CArrayTemplateEmbedded<M, I>::GetData(void)
{
	if (IsEmbedded())
	{
		return mam;
	}
	else
	{
		return mcArray.GetData();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
void* CArrayTemplateEmbedded<M, I>::GetPtr(int iIndex)
{
	return *Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
int CArrayTemplateEmbedded<M, I>::GetIndex(M* pvElement)
{
	int tIndex;
	int tBase;
	int tDifference;

	tBase = (int)(size_t) GetData();
	tIndex = (int)(size_t) pvElement;
	tDifference = tIndex - tBase;

	//Make sure the element is correctly aligned.
	if (tDifference % sizeof(M) != 0)
	{
		return -1;
	}

	//Make sure the element lies within the array.
	tIndex = tDifference / sizeof(M);
	if ((tIndex < 0) || (tIndex >= miUsedElements))
	{
		return -1;
	}

	return tIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
int CArrayTemplateEmbedded<M, I>::Find(M* pData)
{
	int		i;
	M*		pIndex;

	for (i = 0; i < miUsedElements; i++)
	{
		pIndex = Get(i);
		if (memcmp(pIndex, pData, sizeof(M)) == 0)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
int CArrayTemplateEmbedded<M, I>::AddNum(int iNumElements)
{
	int	iOldUsedElements;

	if (IsEmbedded())
	{
		iOldUsedElements = miUsedElements;
		if (miUsedElements + iNumElements > miChunkSize)
		{
			BecomeArray(miUsedElements + iNumElements);
			return iOldUsedElements;
		}
		else
		{
			miUsedElements += iNumElements;
			return iOldUsedElements;
		}
	}
	else
	{
		miUsedElements += iNumElements;
		return mcArray.AddNum(iNumElements);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
int CArrayTemplateEmbedded<M, I>::Resize(int iNumElements)
{
	int	iOldUsedElements;

	iOldUsedElements = miUsedElements;
	if (IsEmbedded())
	{
		if (iNumElements > miChunkSize)
		{
			BecomeArray(iNumElements);
			return iOldUsedElements;
		}
		else
		{
			miUsedElements = iNumElements;
			return iOldUsedElements;
		}
	}
	else
	{
		if (iNumElements <= miChunkSize)
		{
			BecomeEmbedded(iNumElements);
			return iOldUsedElements;
		}
		else
		{
			miUsedElements = iNumElements;
			return mcArray.Resize(iNumElements);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
M* CArrayTemplateEmbedded<M, I>::InsertAt(int iIndex)
{
	void*	pSource;
	void*	pDest;
	M*		pv;

	if (IsEmbedded())
	{
		if (miUsedElements+1 > miChunkSize)
		{
			BecomeArray(miUsedElements+1);

			pSource = mcArray.Get(iIndex);
			pDest = RemapSinglePointer(pSource, sizeof(M));
			memmove(pDest, pSource, sizeof(M) * (miUsedElements - 1 - iIndex));
			pv = (M*)pSource;
		}
		else
		{
			miUsedElements++;
			pSource = &mam[iIndex];
			pDest = RemapSinglePointer(pSource, sizeof(M));
			memmove(pDest, pSource, sizeof(M) * (miUsedElements - 1 - iIndex));
			pv = (M*)pSource;
		}
	}
	else
	{
		miUsedElements++;
		pv = mcArray.InsertAt(iIndex);
	}
	return PostMalloc(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
M* CArrayTemplateEmbedded<M, I>::InsertAt(M* pData, int iIndex)
{
	M*	pAdded;

	pAdded = InsertAt(iIndex);
	memcpy(pAdded, pData, sizeof(M));
	return pAdded;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
BOOL CArrayTemplateEmbedded<M, I>::IsEmpty(void)
{
	return miUsedElements == 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
BOOL CArrayTemplateEmbedded<M, I>::IsNotEmpty(void)
{
	return miUsedElements != 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
int CArrayTemplateEmbedded<M, I>::NumElements(void)
{
	return miUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
BOOL CArrayTemplateEmbedded<M, I>::Remove(M* pData, int bPreserveOrder)
{
	int		iIndex;

	iIndex = Find(pData);
	if (iIndex != -1)
	{
		RemoveAt(iIndex, bPreserveOrder);
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
template<class M, int I>
void CArrayTemplateEmbedded<M, I>::RemoveAt(int iIndex, int bPreserveOrder)
{
	void*	pSource;
	void*	pDest;
	int		iMove;

	if (IsEmbedded())
	{
		pDest = Get(iIndex);
		pSource = RemapSinglePointer(pDest, sizeof(M));

		miUsedElements--;

		iMove = miUsedElements - iIndex;
		if (iMove > 0)
		{
			memmove(pDest, pSource, sizeof(M) * iMove);
		}
	}
	else
	{
		miUsedElements--;
		mcArray.RemoveAt(iIndex, bPreserveOrder);
		if (miUsedElements <= miChunkSize)
		{
			BecomeEmbedded(miUsedElements);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
M* CArrayTemplateEmbedded<M, I>::SafeGet(int iIndex)
{
	return Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
void CArrayTemplateEmbedded<M, I>::Copy(CArrayTemplateEmbedded<M, I>* pcArrayTemplateEmbedded)
{
	int		i;
	int		iNumElements;
	M*		pvData;

	iNumElements = pcArrayTemplateEmbedded->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pvData = pcArrayTemplateEmbedded->Get(i);
		Add(pvData);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
BOOL CArrayTemplateEmbedded<M, I>::TestInternalConsistency(void)
{
	if (IsEmbedded())
	{
		return TRUE;
	}
	else
	{
		if (miUsedElements != mcArray.NumElements())
		{
			return FALSE;
		}
		if (sizeof(M) != mcArray.ElementSize())
		{
			return FALSE;
		}
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
void CArrayTemplateEmbedded<M, I>::Zero(void)
{
	if (IsEmbedded())
	{
		if (miUsedElements != 0)
		{
			memset_fast(mam, 0, miUsedElements * sizeof(M));
		}
	}
	else
	{
		mcArray.Zero();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
BOOL CArrayTemplateEmbedded<M, I>::Write(CFileWriter* pcFileWriter)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int I>
BOOL CArrayTemplateEmbedded<M, I>::Read(CFileReader* pcFileReader)
{
	return FALSE;
}


#endif // !__ARRAY_EMBEDDED_H__

