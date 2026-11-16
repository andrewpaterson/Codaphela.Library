/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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


//CArrayTemplate<M> (an array of M counted in size, a CArrayBlock) and CArrayLargeTemplate<M> (counted in lsize, a
//CArrayLargeBlock) are this, written once: BLOCK is the class each derives from.
template<class M, class BLOCK>
class CArrayTemplateBase : public BLOCK, protected CPostMalloc<M>
{
public:
	typedef typename BLOCK::Count	Count;

	void	Init(void);
	void	Init(CMallocator* pcMalloc);
	void	Init(CMallocator* pcMalloc, Count iChunkSize);
	void	Init(CMallocator* pcMalloc, M* pvData, Count uiNumData);
	void	Fake(M* pvData, Count iNum, Count iChunkSize = 1);
	void	TakeFrom(CArrayTemplateBase<M, BLOCK>* pcSource);
	void	Kill(void);

	M*		Add(void);
	M*		Add(M* pvData);
	M*		AddGetIndex(Count* piIndex);
	Count	AddIfUnique(M* pvData);
	Count	AddIfUniqueKey(M* pvData, Count iKeyOffset, Count iKeySize);

	void	Copy(CArrayTemplateBase<M, BLOCK>* pcOtherArray);

	M*		Get(Count iIndex);
	M*		SafeGet(Count iIndex);
	M*		GetData(void) const;
	Count	GetIndex(M* pvData);
	M*		Tail(void);
	M&						operator[](Count iIndex);

	M*		InsertArrayAfterEnd(CArrayTemplateBase<M, BLOCK>* pcOtherArray);
	M*		InsertArrayAt(CArrayTemplateBase<M, BLOCK>* pcOtherArray, Count iIndex);
	M*		InsertArrayBeforeStart(CArrayTemplateBase<M, BLOCK>* pcOtherArray);
	M*		InsertAt(Count iIndex);
	M*		InsertAt(M* pvData, Count iIndex);
	M*		InsertBlockAfterEnd(M* paElements, Count iLength);
	M*		InsertBlockAt(M* paElements, Count iLength, Count iIndex);
	M*		InsertBlockBeforeStart(M* paElements, Count iLength);
	Count	InsertIntoSorted(DataCompare fCompare, M* pvData, bool bOverwriteExisting);
	M*		InsertNumAt(Count iNumElements, Count iIndex);

	Count	RemoveArray(CArrayTemplateBase<M, BLOCK>* pcOtherArray, bool bPreserveOrder = true);
	Count	RemoveArrayFromSorted(CArrayTemplateBase<M, BLOCK>* pcOtherArray, DataCompare fCompare);

	bool	Pop(M* pvData);
	bool	Pop(void);
	void	Push(M* pvData);
	M*		Push(void);
	M*		PushCopy(void);

	bool	Contains(M* pvData);
	bool	Equals(CArrayTemplateBase<M, BLOCK>* pcOtherArray);
	Count	Find(M* pvData);
	bool	FindInSorted(M* pvData, DataCompare fCompare, Count* piIndex);
	Count	FindWithKey(M* pvData, Count iKeyOffset, Count iKeySize);

	void	Set(Count iIndex, M* pvData);
	bool	SafeSet(Count iIndex, M* pvData);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::Init(void) { BLOCK::Init(sizeof(M)); }
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::Init(CMallocator* pcMalloc) { BLOCK::Init(pcMalloc, sizeof(M)); }
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::Init(CMallocator* pcMalloc, Count iChunkSize) { BLOCK::Init(pcMalloc, sizeof(M), iChunkSize); }
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::Init(CMallocator* pcMalloc, M* pvData, Count uiNumData) { BLOCK::Init(pcMalloc, sizeof(M), pvData, uiNumData); }
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::TakeFrom(CArrayTemplateBase<M, BLOCK>* pcSource) { BLOCK::TakeFrom(pcSource); }
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::Kill(void) { BLOCK::Kill(); }
template<class M, class BLOCK> M*	CArrayTemplateBase<M, BLOCK>::SafeGet(Count iIndex) { return (M*)BLOCK::SafeGet(iIndex); }
template<class M, class BLOCK> bool	CArrayTemplateBase<M, BLOCK>::SafeSet(Count iIndex, M* pvData) { return BLOCK::SafeSet(iIndex, pvData); }
template<class M, class BLOCK> M*	CArrayTemplateBase<M, BLOCK>::Add(void) { return this->PostMalloc((M*)BLOCK::Add()); }
template<class M, class BLOCK> M*	CArrayTemplateBase<M, BLOCK>::AddGetIndex(Count* piIndex) { return this->PostMalloc((M*)BLOCK::AddGetIndex(piIndex)); }
template<class M, class BLOCK> M*	CArrayTemplateBase<M, BLOCK>::Add(M* pvData) { return (M*)BLOCK::Add(pvData); }
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::Set(Count iIndex, M* pvData) { return BLOCK::Set(iIndex, pvData); }
template<class M, class BLOCK> M*	CArrayTemplateBase<M, BLOCK>::InsertAt(Count iIndex) { return this->PostMalloc((M*)BLOCK::InsertAt(iIndex)); }
template<class M, class BLOCK> M*	CArrayTemplateBase<M, BLOCK>::InsertAt(M* pvData, Count iIndex) { return (M*)BLOCK::InsertAt(pvData, iIndex); }
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::Copy(CArrayTemplateBase<M, BLOCK>* pcOtherArray) { BLOCK::Copy(pcOtherArray); }


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
void CArrayTemplateBase<M, BLOCK>::Push(M* pvData)
{
	BLOCK::Push(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::Push(void)
{
	return Add();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::PushCopy(void)
{
	return (M*)BLOCK::PushCopy();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
bool CArrayTemplateBase<M, BLOCK>::Pop(M* pvData)
{
	return BLOCK::Pop(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
bool CArrayTemplateBase<M, BLOCK>::Pop(void)
{
	return BLOCK::Pop();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::Tail(void)
{
	return (M*)BLOCK::Tail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
typename BLOCK::Count CArrayTemplateBase<M, BLOCK>::AddIfUnique(M* pvData)
{
	return BLOCK::AddIfUnique(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
typename BLOCK::Count CArrayTemplateBase<M, BLOCK>::Find(M* pvData)
{
	return BLOCK::Find(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
bool CArrayTemplateBase<M, BLOCK>::Contains(M* pvData)
{
	return BLOCK::Contains(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
typename BLOCK::Count CArrayTemplateBase<M, BLOCK>::AddIfUniqueKey(M* pvData, Count iKeyOffset, Count iKeySize)
{
	return BLOCK::AddIfUniqueKey(pvData, iKeyOffset, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
typename BLOCK::Count CArrayTemplateBase<M, BLOCK>::FindWithKey(M* pvData, Count iKeyOffset, Count iKeySize)
{
	return BLOCK::FindWithKey(pvData, iKeyOffset, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
bool CArrayTemplateBase<M, BLOCK>::Equals(CArrayTemplateBase<M, BLOCK>* pcOtherArray)
{
	return BLOCK::Equals(pcOtherArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
typename BLOCK::Count CArrayTemplateBase<M, BLOCK>::InsertIntoSorted(DataCompare fCompare, M* pvData, bool bOverwriteExisting)
{
	return BLOCK::InsertIntoSorted(fCompare, pvData, bOverwriteExisting);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
bool CArrayTemplateBase<M, BLOCK>::FindInSorted(M* pvData, DataCompare fCompare, Count* piIndex)
{
	return BLOCK::FindInSorted(pvData, fCompare, piIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::InsertArrayAfterEnd(CArrayTemplateBase<M, BLOCK>* pcOtherArray)
{
	return (M*)BLOCK::InsertArrayAfterEnd(pcOtherArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::InsertArrayBeforeStart(CArrayTemplateBase<M, BLOCK>* pcOtherArray)
{
	return (M*)BLOCK::InsertArrayBeforeStart(pcOtherArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::InsertArrayAt(CArrayTemplateBase<M, BLOCK>* pcOtherArray, Count iIndex)
{
	return (M*)BLOCK::InsertArrayAt(pcOtherArray, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::InsertNumAt(Count iNumElements, Count iIndex)
{
	return this->PostMalloc((M*)BLOCK::InsertNumAt(iNumElements, iIndex), iNumElements, this->miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::InsertBlockAfterEnd(M* paElements, Count iLength)
{
	return (M*)BLOCK::InsertBlockAfterEnd(paElements, iLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::InsertBlockBeforeStart(M* paElements, Count iLength)
{
	return (M*)BLOCK::InsertBlockBeforeStart(paElements, iLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::InsertBlockAt(M* paElements, Count iLength, Count iIndex)
{
	return (M*)BLOCK::InsertBlockAt(paElements, iLength, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::Get(Count iIndex)
{
	return (M*)BLOCK::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
typename BLOCK::Count CArrayTemplateBase<M, BLOCK>::GetIndex(M* pvData)
{
	return BLOCK::GetIndex(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::GetData(void) const
{
	return (M*)BLOCK::GetData();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
void CArrayTemplateBase<M, BLOCK>::Fake(M* pvData, Count iNum, Count iChunkSize)
{
	BLOCK::Fake(sizeof(M), pvData, iNum, iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M& CArrayTemplateBase<M, BLOCK>::operator[](typename BLOCK::Count iIndex)
{
	return ((M*)this->mpvArray)[iIndex];
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
typename BLOCK::Count CArrayTemplateBase<M, BLOCK>::RemoveArray(CArrayTemplateBase<M, BLOCK>* pcOtherArray, bool bPreserveOrder)
{
	return BLOCK::RemoveArray(pcOtherArray, bPreserveOrder);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
typename BLOCK::Count CArrayTemplateBase<M, BLOCK>::RemoveArrayFromSorted(CArrayTemplateBase<M, BLOCK>* pcOtherArray, DataCompare fCompare)
{
	return BLOCK::RemoveArrayFromSorted(pcOtherArray, fCompare);
}


//The names the arrays are known by: each is the array of its block, so a CArrayTemplate<M> is a CArrayBlock.
template<class M> class CArrayTemplate : public CArrayTemplateBase<M, CArrayBlock> {};
template<class M> class CArrayLargeTemplate : public CArrayTemplateBase<M, CArrayLargeBlock> {};


#endif // __TEMPLATE_ARRAY_H__

