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
#ifndef __ARRAY_TEMPLATE_BASE_H__
#define __ARRAY_TEMPLATE_BASE_H__


//CArrayTemplate<M> (an array of M counted in size, a CArrayBlock) and CArrayLargeTemplate<M> (counted in lsize, a
//CArrayLargeBlock) are this, written once: BLOCK is the class each derives from.
template<class M, class BLOCK>
class CArrayTemplateBase : public BLOCK, protected CPostMalloc<M>
{
public:
	void					Init(void);
	void					Init(CMallocator* pcMalloc);
	void					Init(CMallocator* pcMalloc, typename BLOCK::Count iChunkSize);
	void					Init(CMallocator* pcMalloc, M* pvData, typename BLOCK::Count uiNumData);
	void					Fake(M* pvData, typename BLOCK::Count iNum, typename BLOCK::Count iChunkSize = 1);
	void					TakeFrom(CArrayTemplateBase<M, BLOCK>* pcSource);
	void					Kill(void);

	M*						Add(void);
	M*						Add(M* pvData);
	M*						AddGetIndex(typename BLOCK::Count* piIndex);
	typename BLOCK::Count	AddIfUnique(M* pvData);
	typename BLOCK::Count	AddIfUniqueKey(M* pvData, typename BLOCK::Count iKeyOffset, typename BLOCK::Count iKeySize);

	void					Copy(CArrayTemplateBase<M, BLOCK>* pcOtherArray);

	M*						Get(typename BLOCK::Count iIndex);
	M*						SafeGet(typename BLOCK::Count iIndex);
	M*						GetData(void);
	typename BLOCK::Count	GetIndex(M* pvData);
	M*						Tail(void);
	M&						operator[](typename BLOCK::Count iIndex);

	M*						InsertArrayAfterEnd(CArrayTemplateBase<M, BLOCK>* pcOtherArray);
	M*						InsertArrayAt(CArrayTemplateBase<M, BLOCK>* pcOtherArray, typename BLOCK::Count iIndex);
	M*						InsertArrayBeforeStart(CArrayTemplateBase<M, BLOCK>* pcOtherArray);
	M*						InsertAt(typename BLOCK::Count iIndex);
	M*						InsertAt(M* pvData, typename BLOCK::Count iIndex);
	M*						InsertBlockAfterEnd(M* paElements, typename BLOCK::Count iLength);
	M*						InsertBlockAt(M* paElements, typename BLOCK::Count iLength, typename BLOCK::Count iIndex);
	M*						InsertBlockBeforeStart(M* paElements, typename BLOCK::Count iLength);
	typename BLOCK::Count	InsertIntoSorted(DataCompare fCompare, M* pvData, bool bOverwriteExisting);
	M*						InsertNumAt(typename BLOCK::Count iNumElements, typename BLOCK::Count iIndex);

	typename BLOCK::Count	RemoveArray(CArrayTemplateBase<M, BLOCK>* pcOtherArray, bool bPreserveOrder = true);
	typename BLOCK::Count	RemoveArrayFromSorted(CArrayTemplateBase<M, BLOCK>* pcOtherArray, DataCompare fCompare);

	bool					Pop(M* pvData);
	bool					Pop(void);
	void					Push(M* pvData);
	M*						Push(void);
	M*						PushCopy(void);

	bool					Contains(M* pvData);
	bool					Equals(CArrayTemplateBase<M, BLOCK>* pcOtherArray);
	typename BLOCK::Count	Find(M* pvData);
	bool					FindInSorted(M* pvData, DataCompare fCompare, typename BLOCK::Count* piIndex);
	typename BLOCK::Count	FindWithKey(M* pvData, typename BLOCK::Count iKeyOffset, typename BLOCK::Count iKeySize);

	void					Set(typename BLOCK::Count iIndex, M* pvData);
	bool					SafeSet(typename BLOCK::Count iIndex, M* pvData);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::Init(void) { BLOCK::Init(sizeof(M)); }
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::Init(CMallocator* pcMalloc) { BLOCK::Init(pcMalloc, sizeof(M)); }
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::Init(CMallocator* pcMalloc, typename BLOCK::Count iChunkSize) { BLOCK::Init(pcMalloc, sizeof(M), iChunkSize); }
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::Init(CMallocator* pcMalloc, M* pvData, typename BLOCK::Count uiNumData) { BLOCK::Init(pcMalloc, sizeof(M), pvData, uiNumData); }
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::TakeFrom(CArrayTemplateBase<M, BLOCK>* pcSource) { BLOCK::TakeFrom(pcSource); }
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::Kill(void) { BLOCK::Kill(); }
template<class M, class BLOCK> M*	CArrayTemplateBase<M, BLOCK>::SafeGet(typename BLOCK::Count iIndex) { return (M*)BLOCK::SafeGet(iIndex); }
template<class M, class BLOCK> bool	CArrayTemplateBase<M, BLOCK>::SafeSet(typename BLOCK::Count iIndex, M* pvData) { return BLOCK::SafeSet(iIndex, pvData); }
template<class M, class BLOCK> M*	CArrayTemplateBase<M, BLOCK>::Add(void) { return this->PostMalloc((M*)BLOCK::Add()); }
template<class M, class BLOCK> M*	CArrayTemplateBase<M, BLOCK>::AddGetIndex(typename BLOCK::Count* piIndex) { return this->PostMalloc((M*)BLOCK::AddGetIndex(piIndex)); }
template<class M, class BLOCK> M*	CArrayTemplateBase<M, BLOCK>::Add(M* pvData) { return (M*)BLOCK::Add(pvData); }
template<class M, class BLOCK> void	CArrayTemplateBase<M, BLOCK>::Set(typename BLOCK::Count iIndex, M* pvData) { return BLOCK::Set(iIndex, pvData); }
template<class M, class BLOCK> M*	CArrayTemplateBase<M, BLOCK>::InsertAt(typename BLOCK::Count iIndex) { return this->PostMalloc((M*)BLOCK::InsertAt(iIndex)); }
template<class M, class BLOCK> M*	CArrayTemplateBase<M, BLOCK>::InsertAt(M* pvData, typename BLOCK::Count iIndex) { return (M*)BLOCK::InsertAt(pvData, iIndex); }
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
typename BLOCK::Count CArrayTemplateBase<M, BLOCK>::AddIfUniqueKey(M* pvData, typename BLOCK::Count iKeyOffset, typename BLOCK::Count iKeySize)
{
	return BLOCK::AddIfUniqueKey(pvData, iKeyOffset, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
typename BLOCK::Count CArrayTemplateBase<M, BLOCK>::FindWithKey(M* pvData, typename BLOCK::Count iKeyOffset, typename BLOCK::Count iKeySize)
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
bool CArrayTemplateBase<M, BLOCK>::FindInSorted(M* pvData, DataCompare fCompare, typename BLOCK::Count* piIndex)
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
M* CArrayTemplateBase<M, BLOCK>::InsertArrayAt(CArrayTemplateBase<M, BLOCK>* pcOtherArray, typename BLOCK::Count iIndex)
{
	return (M*)BLOCK::InsertArrayAt(pcOtherArray, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::InsertNumAt(typename BLOCK::Count iNumElements, typename BLOCK::Count iIndex)
{
	return this->PostMalloc((M*)BLOCK::InsertNumAt(iNumElements, iIndex), iNumElements, this->miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::InsertBlockAfterEnd(M* paElements, typename BLOCK::Count iLength)
{
	return (M*)BLOCK::InsertBlockAfterEnd(paElements, iLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::InsertBlockBeforeStart(M* paElements, typename BLOCK::Count iLength)
{
	return (M*)BLOCK::InsertBlockBeforeStart(paElements, iLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::InsertBlockAt(M* paElements, typename BLOCK::Count iLength, typename BLOCK::Count iIndex)
{
	return (M*)BLOCK::InsertBlockAt(paElements, iLength, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CArrayTemplateBase<M, BLOCK>::Get(typename BLOCK::Count iIndex)
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
M* CArrayTemplateBase<M, BLOCK>::GetData(void)
{
	return (M*)BLOCK::GetData();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
void CArrayTemplateBase<M, BLOCK>::Fake(M* pvData, typename BLOCK::Count iNum, typename BLOCK::Count iChunkSize)
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


#endif // __ARRAY_TEMPLATE_BASE_H__

