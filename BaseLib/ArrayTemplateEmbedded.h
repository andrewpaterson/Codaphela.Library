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
#ifndef __ARRAY_EMBEDDED_H__
#define __ARRAY_EMBEDDED_H__
#include "PrimitiveTypes.h"
#include "ArrayTemplate.h"
#include "ConstructorCall.h"


template<class M, int8 I>
class CArrayTemplateEmbedded : protected CPostMalloc<M>
{
protected:
	uint16		miUsedElements;

	union
	{
		CArrayTemplate<M>	mcArray;
		M					mam[I];
	};

protected:
	void	BecomeArray(size iUsedElements);
	void	BecomeEmbedded(size iUsedElements);

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
	M*		Add(M* pvData);
	void	AddPtr(void* pv);
	size 	ByteSize(void);
	size	EmbeddedSize(void);
	size 	Find(M* pvElement);
	M*		Get(size iIndex);
	M*		GetData(void);
	size	GetIndex(M* pvElement);
	void*	GetPtr(size iIndex);
	size	AddNum(size iNumElements);
	size	Resize(size iNumElements);
	M* 		InsertAt(size iIndex);
	M*		InsertAt(M* pvData, size iIndex);
	bool	IsEmpty(void);
	bool	IsNotEmpty(void);
	bool	IsEmbedded(void);
	bool	IsArray(void);
	size	NumElements(void);
	bool	Remove(M* pvElement, bool bPreserveOrder = true);
	void 	RemoveAt(size iIndex, bool bPreserveOrder = true);
	M*		SafeGet(size iIndex);
	void	Copy(CArrayTemplateEmbedded<M, I>* pcArrayTemplateEmbedded);
	bool	TestInternalConsistency(void);
	void 	Zero(void);
	bool	Write(CFileWriter* pcFileWriter);
	bool	Read(CFileReader* pcFileReader);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
void CArrayTemplateEmbedded<M, I>::Init(void)
{
	this->miUsedElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
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
template<class M, int8 I>
void CArrayTemplateEmbedded<M, I>::ReInit(void)
{
	Kill();
	Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
bool CArrayTemplateEmbedded<M, I>::IsEmbedded(void)
{
	return miUsedElements <= I;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
bool CArrayTemplateEmbedded<M, I>::IsArray(void)
{
	return miUsedElements > I;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
void CArrayTemplateEmbedded<M, I>::BecomeArray(size iUsedElements)
{
	M	am[I];  //Problem

	memcpy(am, mam, miUsedElements * sizeof(M));
	mcArray.Init();
	mcArray.AddNum(iUsedElements);
	memcpy(mcArray.GetData(), am, miUsedElements * sizeof(M));
	miUsedElements = (uint16)iUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
void CArrayTemplateEmbedded<M, I>::BecomeEmbedded(size iUsedElements)
{
	M	am[I];  //Problem

	memcpy(am, mcArray.GetData(), iUsedElements * sizeof(M));
	mcArray.Kill();

	memcpy(mam, am, iUsedElements * sizeof(M));
	miUsedElements = (uint16)iUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
M* CArrayTemplateEmbedded<M, I>::Add(void)
{
	M*	pv;
	if (IsEmbedded())
	{
		if (miUsedElements + 1 > I)
		{
			BecomeArray(miUsedElements + 1);
			pv = mcArray.Tail();
		}
		else
		{
			miUsedElements++;
			pv = &mam[miUsedElements - 1];
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
template<class M, int8 I>
M* CArrayTemplateEmbedded<M, I>::Add(M* pvData)
{
	M*	pAdded;

	pAdded = Add();
	memcpy(pAdded, pvData, sizeof(M));
	return pAdded;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
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
template<class M, int8 I>
size CArrayTemplateEmbedded<M, I>::ByteSize(void)
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
template<class M, int8 I>
size CArrayTemplateEmbedded<M, I>::EmbeddedSize(void)
{
	return I;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
M* CArrayTemplateEmbedded<M, I>::Get(size iIndex)
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
template<class M, int8 I>
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
template<class M, int8 I>
void* CArrayTemplateEmbedded<M, I>::GetPtr(size iIndex)
{
	return *Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
size CArrayTemplateEmbedded<M, I>::GetIndex(M* pvElement)
{
	size	uiIndex;
	size	tDifference;

	tDifference = (size)((ptr)pvElement - (ptr)GetData());

	//Make sure the element is correctly aligned.
	if (tDifference % sizeof(M) != 0)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	//Make sure the element lies within the array.
	uiIndex = (tDifference / sizeof(M));
	if (uiIndex >= miUsedElements)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	return uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
size CArrayTemplateEmbedded<M, I>::Find(M* pvData)
{
	M*		pIndex;
	size	i;

	for (i = 0; i < miUsedElements; i++)
	{
		pIndex = Get(i);
		if (memcmp(pIndex, pvData, sizeof(M)) == 0)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
size CArrayTemplateEmbedded<M, I>::AddNum(size iNumElements)
{
	size	iOldUsedElements;

	if (IsEmbedded())
	{
		iOldUsedElements = miUsedElements;
		if (miUsedElements + (uint16)iNumElements > I)
		{
			BecomeArray(miUsedElements + (uint16)iNumElements);
			return iOldUsedElements;
		}
		else
		{
			miUsedElements += (uint16)iNumElements;
			return iOldUsedElements;
		}
	}
	else
	{
		miUsedElements += (uint16)iNumElements;
		return mcArray.AddNum(iNumElements);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
size CArrayTemplateEmbedded<M, I>::Resize(size iNumElements)
{
	size	iOldUsedElements;

	iOldUsedElements = miUsedElements;
	if (IsEmbedded())
	{
		if (iNumElements > I)
		{
			BecomeArray(iNumElements);
			return iOldUsedElements;
		}
		else
		{
			miUsedElements = (uint16)iNumElements;
			return iOldUsedElements;
		}
	}
	else
	{
		if (iNumElements <= I)
		{
			BecomeEmbedded(iNumElements);
			return iOldUsedElements;
		}
		else
		{
			miUsedElements = (uint16)iNumElements;
			return mcArray.Resize(iNumElements);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
M* CArrayTemplateEmbedded<M, I>::InsertAt(size iIndex)
{
	void*	pSource;
	void*	pDest;
	M*		pv;

	if (IsEmbedded())
	{
		if (miUsedElements+1 > I)
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
template<class M, int8 I>
M* CArrayTemplateEmbedded<M, I>::InsertAt(M* pvData, size iIndex)
{
	M*	pAdded;

	pAdded = InsertAt(iIndex);
	memcpy(pAdded, pvData, sizeof(M));
	return pAdded;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
bool CArrayTemplateEmbedded<M, I>::IsEmpty(void)
{
	return miUsedElements == 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
bool CArrayTemplateEmbedded<M, I>::IsNotEmpty(void)
{
	return miUsedElements != 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
size CArrayTemplateEmbedded<M, I>::NumElements(void)
{
	return miUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
bool CArrayTemplateEmbedded<M, I>::Remove(M* pvData, bool bPreserveOrder)
{
	size		iIndex;

	iIndex = Find(pvData);
	if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		RemoveAt(iIndex, bPreserveOrder);
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
template<class M, int8 I>
void CArrayTemplateEmbedded<M, I>::RemoveAt(size iIndex, bool bPreserveOrder)
{
	void*	pSource;
	void*	pDest;
	size	iMove;

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
		if (miUsedElements <= I)
		{
			BecomeEmbedded(miUsedElements);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
M* CArrayTemplateEmbedded<M, I>::SafeGet(size iIndex)
{
	return Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
void CArrayTemplateEmbedded<M, I>::Copy(CArrayTemplateEmbedded<M, I>* pcArrayTemplateEmbedded)
{
	size	i;
	size	iNumElements;
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
template<class M, int8 I>
bool CArrayTemplateEmbedded<M, I>::TestInternalConsistency(void)
{
	if (IsEmbedded())
	{
		return true;
	}
	else
	{
		if (miUsedElements != mcArray.NumElements())
		{
			return false;
		}
		if (sizeof(M) != mcArray.ElementSize())
		{
			return false;
		}
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
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
template<class M, int8 I>
bool CArrayTemplateEmbedded<M, I>::Write(CFileWriter* pcFileWriter)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, int8 I>
bool CArrayTemplateEmbedded<M, I>::Read(CFileReader* pcFileReader)
{
	return false;
}


#endif // __ARRAY_EMBEDDED_H__

