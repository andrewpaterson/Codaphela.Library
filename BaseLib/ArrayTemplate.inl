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
//No include guard: ArrayTemplate.inl is included once per array size (ArraySizeNatural.inl and
//ArraySizeLarge.inl define the names and types it is written with).

template<class M>
class ARRAY_TEMPLATE : public ARRAY_BLOCK, protected CPostMalloc<M>
{
public:
	void 	Init(void);
	void 	Init(CMallocator* pcMalloc);
	void 	Init(CMallocator* pcMalloc, ASIZE iChunkSize);
	void	Init(CMallocator* pcMalloc, M* pvData, ASIZE uiNumData);
	void	Fake(M* pvData, ASIZE iNum, ASIZE iChunkSize = 1);
	void	Kill(void);

	M*		Add(void);
	M*		Add(M* pvData);
	M* 		AddGetIndex(ASIZE* piIndex);
	ASIZE 	AddIfUnique(M* pvData);
	ASIZE 	AddIfUniqueKey(M* pvData, ASIZE iKeyOffset, ASIZE iKeySize);

	void 	Copy(ARRAY_TEMPLATE<M>* pcOtherArray);

	M*		Get(ASIZE iIndex);
	M*		SafeGet(ASIZE iIndex);
	M*		GetData(void);
	ASIZE	GetIndex(M* pvData);
	M*		Tail(void);
	M&		operator[](ASIZE iIndex);

	M*		InsertArrayAfterEnd(ARRAY_TEMPLATE<M>* pcOtherArray);
	M*		InsertArrayAt(ARRAY_TEMPLATE<M>* pcOtherArray, ASIZE iIndex);
	M*		InsertArrayBeforeStart(ARRAY_TEMPLATE<M>* pcOtherArray);
	M* 		InsertAt(ASIZE iIndex);
	M*	 	InsertAt(M* pvData, ASIZE iIndex);
	M*		InsertBlockAfterEnd(M* paElements, ASIZE iLength);
	M*		InsertBlockAt(M* paElements, ASIZE iLength, ASIZE iIndex);
	M*		InsertBlockBeforeStart(M* paElements, ASIZE iLength);
	ASIZE	InsertIntoSorted(DataCompare fCompare, M* pvData, bool bOverwriteExisting);
	M*		InsertNumAt(ASIZE iNumElements, ASIZE iIndex);

	ASIZE	RemoveArray(ARRAY_TEMPLATE<M>* pcOtherArray, bool bPreserveOrder = true);
	ASIZE	RemoveArrayFromSorted(ARRAY_TEMPLATE<M>* pcOtherArray, DataCompare fCompare);

	bool	Pop(M* pvData);
	bool	Pop(void);
	void 	Push(M* pvData);
	M*		Push(void);
	M*		PushCopy(void);

	bool	Contains(M* pvData);
	bool	Equals(ARRAY_TEMPLATE<M>* pcOtherArray);
	ASIZE 	Find(M* pvData);
	bool	FindInSorted(M* pvData, DataCompare fCompare, ASIZE* piIndex);
	ASIZE 	FindWithKey(M* pvData, ASIZE iKeyOffset, ASIZE iKeySize);

	void	Set(ASIZE iIndex, M* pvData);
	bool	SafeSet(ASIZE iIndex, M* pvData);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M> void	ARRAY_TEMPLATE<M>::Init(void) { ARRAY_BLOCK::Init(sizeof(M)); }
template<class M> void	ARRAY_TEMPLATE<M>::Init(CMallocator* pcMalloc) { ARRAY_BLOCK::Init(pcMalloc, sizeof(M)); }
template<class M> void	ARRAY_TEMPLATE<M>::Init(CMallocator* pcMalloc, ASIZE iChunkSize) { ARRAY_BLOCK::Init(pcMalloc, sizeof(M), iChunkSize); }
template<class M> void	ARRAY_TEMPLATE<M>::Init(CMallocator* pcMalloc, M* pvData, ASIZE uiNumData) { ARRAY_BLOCK::Init(pcMalloc, sizeof(M), pvData, uiNumData); }
template<class M> void	ARRAY_TEMPLATE<M>::Kill(void) { ARRAY_BLOCK::Kill(); }
template<class M> M*	ARRAY_TEMPLATE<M>::SafeGet(ASIZE iIndex) { return (M*)ARRAY_BLOCK::SafeGet(iIndex); }
template<class M> bool	ARRAY_TEMPLATE<M>::SafeSet(ASIZE iIndex, M* pvData) { return ARRAY_BLOCK::SafeSet(iIndex, pvData); }
template<class M> M*	ARRAY_TEMPLATE<M>::Add(void) { return this->PostMalloc((M*)ARRAY_BLOCK::Add()); }
template<class M> M*	ARRAY_TEMPLATE<M>::AddGetIndex(ASIZE* piIndex) { return this->PostMalloc((M*)ARRAY_BLOCK::AddGetIndex(piIndex)); }
template<class M> M*	ARRAY_TEMPLATE<M>::Add(M* pvData) { return (M*)ARRAY_BLOCK::Add(pvData); }
template<class M> void	ARRAY_TEMPLATE<M>::Set(ASIZE iIndex, M* pvData) { return ARRAY_BLOCK::Set(iIndex, pvData); }
template<class M> M*	ARRAY_TEMPLATE<M>::InsertAt(ASIZE iIndex) { return this->PostMalloc((M*)ARRAY_BLOCK::InsertAt(iIndex)); }
template<class M> M*	ARRAY_TEMPLATE<M>::InsertAt(M* pvData, ASIZE iIndex) { return (M*)ARRAY_BLOCK::InsertAt(pvData, iIndex); }
template<class M> void	ARRAY_TEMPLATE<M>::Copy(ARRAY_TEMPLATE* pcOtherArray) { ARRAY_BLOCK::Copy(pcOtherArray); }


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void ARRAY_TEMPLATE<M>::Push(M* pvData)
{
	ARRAY_BLOCK::Push(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* ARRAY_TEMPLATE<M>::Push(void)
{
	return Add();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* ARRAY_TEMPLATE<M>::PushCopy(void)
{
	return (M*)ARRAY_BLOCK::PushCopy();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool ARRAY_TEMPLATE<M>::Pop(M* pvData)
{
	return ARRAY_BLOCK::Pop(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool ARRAY_TEMPLATE<M>::Pop(void)
{
	return ARRAY_BLOCK::Pop();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* ARRAY_TEMPLATE<M>::Tail(void)
{
	return (M*)ARRAY_BLOCK::Tail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
ASIZE ARRAY_TEMPLATE<M>::AddIfUnique(M* pvData)
{
	return ARRAY_BLOCK::AddIfUnique(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
ASIZE ARRAY_TEMPLATE<M>::Find(M* pvData)
{
	return ARRAY_BLOCK::Find(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool ARRAY_TEMPLATE<M>::Contains(M* pvData)
{
	return ARRAY_BLOCK::Contains(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
ASIZE ARRAY_TEMPLATE<M>::AddIfUniqueKey(M* pvData, ASIZE iKeyOffset, ASIZE iKeySize)
{
	return ARRAY_BLOCK::AddIfUniqueKey(pvData, iKeyOffset, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
ASIZE ARRAY_TEMPLATE<M>::FindWithKey(M* pvData, ASIZE iKeyOffset, ASIZE iKeySize)
{
	return ARRAY_BLOCK::FindWithKey(pvData, iKeyOffset, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool ARRAY_TEMPLATE<M>::Equals(ARRAY_TEMPLATE<M>* pcOtherArray)
{
	return ARRAY_BLOCK::Equals(pcOtherArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
ASIZE ARRAY_TEMPLATE<M>::InsertIntoSorted(DataCompare fCompare, M* pvData, bool bOverwriteExisting)
{
	return ARRAY_BLOCK::InsertIntoSorted(fCompare, pvData, bOverwriteExisting);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool ARRAY_TEMPLATE<M>::FindInSorted(M* pvData, DataCompare fCompare, ASIZE* piIndex)
{
	return ARRAY_BLOCK::FindInSorted(pvData, fCompare, piIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* ARRAY_TEMPLATE<M>::InsertArrayAfterEnd(ARRAY_TEMPLATE<M>* pcOtherArray)
{
	return (M*)ARRAY_BLOCK::InsertArrayAfterEnd(pcOtherArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* ARRAY_TEMPLATE<M>::InsertArrayBeforeStart(ARRAY_TEMPLATE<M>* pcOtherArray)
{
	return (M*)ARRAY_BLOCK::InsertArrayBeforeStart(pcOtherArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* ARRAY_TEMPLATE<M>::InsertArrayAt(ARRAY_TEMPLATE<M>* pcOtherArray, ASIZE iIndex)
{
	return (M*)ARRAY_BLOCK::InsertArrayAt(pcOtherArray, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* ARRAY_TEMPLATE<M>::InsertNumAt(ASIZE iNumElements, ASIZE iIndex)
{
	return this->PostMalloc((M*)ARRAY_BLOCK::InsertNumAt(iNumElements, iIndex), iNumElements, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* ARRAY_TEMPLATE<M>::InsertBlockAfterEnd(M* paElements, ASIZE iLength)
{
	return (M*)ARRAY_BLOCK::InsertBlockAfterEnd(paElements, iLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* ARRAY_TEMPLATE<M>::InsertBlockBeforeStart(M* paElements, ASIZE iLength)
{
	return (M*)ARRAY_BLOCK::InsertBlockBeforeStart(paElements, iLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* ARRAY_TEMPLATE<M>::InsertBlockAt(M* paElements, ASIZE iLength, ASIZE iIndex)
{
	return (M*)ARRAY_BLOCK::InsertBlockAt(paElements, iLength, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* ARRAY_TEMPLATE<M>::Get(ASIZE iIndex)
{
	return (M*)ARRAY_BLOCK::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
ASIZE ARRAY_TEMPLATE<M>::GetIndex(M* pvData)
{
	return ARRAY_BLOCK::GetIndex(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* ARRAY_TEMPLATE<M>::GetData(void)
{
	return (M*)ARRAY_BLOCK::GetData();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void ARRAY_TEMPLATE<M>::Fake(M* pvData, ASIZE iNum, ASIZE iChunkSize)
{
	ARRAY_BLOCK::Fake(sizeof(M), pvData, iNum, iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M& ARRAY_TEMPLATE<M>::operator[](ASIZE iIndex)
{
	return ((M*)this->mpvArray)[iIndex];
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
ASIZE ARRAY_TEMPLATE<M>::RemoveArray(ARRAY_TEMPLATE<M>* pcOtherArray, bool bPreserveOrder)
{
	return ARRAY_BLOCK::RemoveArray(pcOtherArray, bPreserveOrder);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
ASIZE ARRAY_TEMPLATE<M>::RemoveArrayFromSorted(ARRAY_TEMPLATE<M>* pcOtherArray, DataCompare fCompare)
{
	return ARRAY_BLOCK::RemoveArrayFromSorted(pcOtherArray, fCompare);
}
