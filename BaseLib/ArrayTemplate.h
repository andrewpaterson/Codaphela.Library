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
#ifndef __TEMPLATE_ARRAY_H__
#define __TEMPLATE_ARRAY_H__
#include "ConstructorCall.h"
#include "ArrayBlock.h"


template<class M>
class CArrayTemplate : public CArrayBlock, protected CPostMalloc<M>
{
public:
	void 	Init(void);
	void 	Init(CMallocator* pcMalloc);
	void 	Init(CMallocator* pcMalloc, size iChunkSize);
	void	Fake(M* pvData, size iNum, size iChunkSize = 1);
	void	Kill(void);

	M*		Add(void);
	M*		Add(M* pvData);
	M* 		AddGetIndex(size* piIndex);
	size 	AddIfUnique(M* pvData);
	size 	AddIfUniqueKey(M* pvData, size iKeyOffset, size iKeySize);

	void 	Copy(CArrayTemplate<M>* pcTemplateArray);

	M*		Get(size iIndex);
	M*		SafeGet(size iIndex);
	M*		GetData(void);
	size	GetIndex(M* pvData);
	M*		Tail(void);
	M&		operator[](size iIndex);

	M*		InsertArrayAfterEnd(CArrayTemplate<M>* pcTemplateArray);
	M*		InsertArrayAt(CArrayTemplate<M>* pcTemplateArray, size iIndex);
	M*		InsertArrayBeforeStart(CArrayTemplate<M>* pcTemplateArray);
	M* 		InsertAt(size iIndex);
	M*	 	InsertAt(M* pvData, size iIndex);
	M*		InsertBlockAfterEnd(M* paElements, size iLength);
	M*		InsertBlockAt(M* paElements, size iLength, size iIndex);
	M*		InsertBlockBeforeStart(M* paElements, size iLength);
	size	InsertIntoSorted(DataCompare fCompare, M* pvData, bool bOverwriteExisting);
	M*		InsertNumAt(size iNumElements, size iIndex);

	bool	Pop(M* pvData);
	bool	Pop(void);
	void 	Push(M* pvData);
	M*		Push(void);
	M*		PushCopy(void);

	bool	Contains(M* pvData);
	bool	Equals(CArrayTemplate<M>* pcTemplateArray);
	size 	Find(M* pvData);
	bool	FindInSorted(M* pvData, DataCompare fCompare, size* piIndex);
	size 	FindWithKey(M* pvData, size iKeyOffset, size iKeySize);

	void	Set(size iIndex, M* pvData);
	bool	SafeSet(size iIndex, M* pvData);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M> void	CArrayTemplate<M>::Init(void) { CArrayBlock::Init(sizeof(M)); }
template<class M> void	CArrayTemplate<M>::Init(CMallocator* pcMalloc) { CArrayBlock::Init(pcMalloc, sizeof(M)); }
template<class M> void	CArrayTemplate<M>::Init(CMallocator* pcMalloc, size iChunkSize) { CArrayBlock::Init(pcMalloc, sizeof(M), iChunkSize); }
template<class M> void	CArrayTemplate<M>::Kill(void) { CArrayBlock::Kill(); }
template<class M> M*	CArrayTemplate<M>::SafeGet(size iIndex) { return (M*)CArrayBlock::SafeGet(iIndex); }
template<class M> bool	CArrayTemplate<M>::SafeSet(size iIndex, M* pvData) { return CArrayBlock::SafeSet(iIndex, pvData); }
template<class M> M*	CArrayTemplate<M>::Add(void) { return this->PostMalloc((M*)CArrayBlock::Add()); }
template<class M> M*	CArrayTemplate<M>::AddGetIndex(size* piIndex) { return this->PostMalloc((M*)CArrayBlock::AddGetIndex(piIndex)); }
template<class M> M*	CArrayTemplate<M>::Add(M* pvData) { return (M*)CArrayBlock::Add(pvData); }
template<class M> void	CArrayTemplate<M>::Set(size iIndex, M* pvData) { return CArrayBlock::Set(iIndex, pvData); }
template<class M> M*	CArrayTemplate<M>::InsertAt(size iIndex) { return this->PostMalloc((M*)CArrayBlock::InsertAt(iIndex)); }
template<class M> M*	CArrayTemplate<M>::InsertAt(M* pvData, size iIndex) { return (M*)CArrayBlock::InsertAt(pvData, iIndex); }
template<class M> void	CArrayTemplate<M>::Copy(CArrayTemplate* pcTemplateArray) { CArrayBlock::Copy(pcTemplateArray); }


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Push(M* pvData)
{
	CArrayBlock::Push(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::Push(void)
{
	return this->PostMalloc((M*)CArrayBlock::Push());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::PushCopy(void)
{
	return (M*)CArrayBlock::PushCopy();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplate<M>::Pop(M* pvData)
{
	return CArrayBlock::Pop(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplate<M>::Pop(void)
{
	return CArrayBlock::Pop();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::Tail(void)
{
	return (M*)CArrayBlock::Tail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplate<M>::AddIfUnique(M* pvData)
{
	return CArrayBlock::AddIfUnique(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplate<M>::Find(M* pvData)
{
	return CArrayBlock::Find(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplate<M>::Contains(M* pvData)
{
	return CArrayBlock::Contains(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplate<M>::AddIfUniqueKey(M* pvData, size iKeyOffset, size iKeySize)
{
	return CArrayBlock::AddIfUniqueKey(pvData, iKeyOffset, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplate<M>::FindWithKey(M* pvData, size iKeyOffset, size iKeySize)
{
	return CArrayBlock::FindWithKey(pvData, iKeyOffset, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplate<M>::Equals(CArrayTemplate<M>* pcTemplateArray)
{
	return CArrayBlock::Equals(pcTemplateArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplate<M>::InsertIntoSorted(DataCompare fCompare, M* pvData, bool bOverwriteExisting)
{
	return CArrayBlock::InsertIntoSorted(fCompare, pvData, bOverwriteExisting);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplate<M>::FindInSorted(M* pvData, DataCompare fCompare, size* piIndex)
{
	return CArrayBlock::FindInSorted(pvData, fCompare, piIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertArrayAfterEnd(CArrayTemplate<M>* pcTemplateArray)
{
	return (M*)CArrayBlock::InsertArrayAfterEnd(pcTemplateArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertArrayBeforeStart(CArrayTemplate<M>* pcTemplateArray)
{
	return (M*)CArrayBlock::InsertArrayBeforeStart(pcTemplateArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertArrayAt(CArrayTemplate<M>* pcTemplateArray, size iIndex)
{
	return (M*)CArrayBlock::InsertArrayAt(pcTemplateArray, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertNumAt(size iNumElements, size iIndex)
{
	return this->PostMalloc((M*)CArrayBlock::InsertNumAt(iNumElements, iIndex), iNumElements, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertBlockAfterEnd(M* paElements, size iLength)
{
	return (M*)CArrayBlock::InsertBlockAfterEnd(paElements, iLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertBlockBeforeStart(M* paElements, size iLength)
{
	return (M*)CArrayBlock::InsertBlockBeforeStart(paElements, iLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertBlockAt(M* paElements, size iLength, size iIndex)
{
	return (M*)CArrayBlock::InsertBlockAt(paElements, iLength, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::Get(size iIndex)
{
	return (M*)CArrayBlock::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplate<M>::GetIndex(M* pvData)
{
	return CArrayBlock::GetIndex(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::GetData(void)
{
	return (M*)CArrayBlock::GetData();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Fake(M* pvData, size iNum, size iChunkSize)
{
	CArrayBlock::Fake(sizeof(M), pvData, iNum, iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M& CArrayTemplate<M>::operator[](size iIndex)
{
	return ((M*)this->mpvArray)[iIndex];
}


#endif // __TEMPLATE_ARRAY_H__

