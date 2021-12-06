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
	void 	Init(CMallocator* pcMalloc, int iChunkSize);
	void	Fake(M* pvData, int iNum, int iChunkSize = 1);
	void	Kill(void);

	M*		Add(void);
	M*		Add(M* pvData);
	M* 		AddGetIndex(int* piIndex);
	int 	AddIfUnique(M* pData);
	int 	AddIfUniqueKey(M* pData, int iKeyOffset, int iKeySize);

	void 	Copy(CArrayTemplate<M>* pcTemplateArray);

	M*		Get(int iIndex);
	M*		SafeGet(int iIndex);
	M*		GetData(void);
	int		GetIndex(M* pvData);
	M*		Tail(void);
	M&		operator[](int iIndex);

	M*		InsertArrayAfterEnd(CArrayTemplate<M>* pcTemplateArray);
	M*		InsertArrayAt(CArrayTemplate<M>* pcTemplateArray, int iIndex);
	M*		InsertArrayBeforeStart(CArrayTemplate<M>* pcTemplateArray);
	M* 		InsertAt(int iIndex);
	M*	 	InsertAt(M* pvData, int iIndex);
	M*		InsertBlockAfterEnd(M* paElements, int iLength);
	M*		InsertBlockAt(M* paElements, int iLength, int iIndex);
	M*		InsertBlockBeforeStart(M* paElements, int iLength);
	int		InsertIntoSorted(DataCompare fCompare, M* pvData, BOOL bOverwriteExisting);
	M*		InsertNumAt(int iNumElements, int iIndex);

	BOOL	Pop(M* pvData);
	BOOL	Pop(void);
	void 	Push(M* pvData);
	M*		Push(void);
	M*		PushCopy(void);

	BOOL	Contains(M* pData);
	BOOL	Equals(CArrayTemplate<M>* pcTemplateArray);
	int 	Find(M* pData);
	BOOL	FindInSorted(M* pData, DataCompare fCompare, int* piIndex);
	int 	FindWithKey(M* pData, int iKeyOffset, int iKeySize);

	void	Set(int iIndex, M* pvData);
	BOOL	SafeSet(int iIndex, M* pvData);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Init(void)
{
	CArrayBlock::Init(sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Init(CMallocator* pcMalloc)
{
	CArrayBlock::Init(pcMalloc, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Init(CMallocator* pcMalloc, int iChunkSize)
{
	CArrayBlock::Init(pcMalloc, sizeof(M), iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Kill(void)
{
	CArrayBlock::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::SafeGet(int iIndex)
{
	return (M*)CArrayBlock::SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplate<M>::SafeSet(int iIndex, M* pvData)
{
	return CArrayBlock::SafeSet(iIndex, pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::Add(void)
{
	return this->PostMalloc((M*)CArrayBlock::Add());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::AddGetIndex(int* piIndex)
{
	return this->PostMalloc((M*)CArrayBlock::AddGetIndex(piIndex));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::Add(M* pvData)
{
	return (M*)CArrayBlock::Add(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Set(int iIndex, M* pvData)
{
	return CArrayBlock::Set(iIndex, pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertAt(int iIndex)  //The new element will be at iIndex
{
	return this->PostMalloc((M*)CArrayBlock::InsertAt(iIndex));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertAt(M* pvData, int iIndex)
{
	return (M*)CArrayBlock::InsertAt(pvData, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Copy(CArrayTemplate* pcTemplateArray)
{
	CArrayBlock::Copy(pcTemplateArray);
}


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
BOOL CArrayTemplate<M>::Pop(M* pvData)
{
	return CArrayBlock::Pop(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplate<M>::Pop(void)
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
int CArrayTemplate<M>::AddIfUnique(M* pData)
{
	return CArrayBlock::AddIfUnique(pData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplate<M>::Find(M* pData)
{
	return CArrayBlock::Find(pData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplate<M>::Contains(M* pData)
{
	return CArrayBlock::Contains(pData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplate<M>::AddIfUniqueKey(M* pData, int iKeyOffset, int iKeySize)
{
	return CArrayBlock::AddIfUniqueKey(pData, iKeyOffset, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplate<M>::FindWithKey(M* pData, int iKeyOffset, int iKeySize)
{
	return CArrayBlock::FindWithKey(pData, iKeyOffset, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplate<M>::Equals(CArrayTemplate<M>* pcTemplateArray)
{
	return CArrayBlock::Equals(pcTemplateArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplate<M>::InsertIntoSorted(DataCompare fCompare, M* pvData, BOOL bOverwriteExisting)
{
	return CArrayBlock::InsertIntoSorted(fCompare, pvData, bOverwriteExisting);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplate<M>::FindInSorted(M* pData, DataCompare fCompare, int* piIndex)
{
	return CArrayBlock::FindInSorted(pData, fCompare, piIndex);
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
M* CArrayTemplate<M>::InsertArrayAt(CArrayTemplate<M>* pcTemplateArray, int iIndex)
{
	return (M*)CArrayBlock::InsertArrayAt(pcTemplateArray, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertNumAt(int iNumElements, int iIndex)
{
	return this->PostMalloc((M*)CArrayBlock::InsertNumAt(iNumElements, iIndex), iNumElements, miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertBlockAfterEnd(M* paElements, int iLength)
{
	return (M*)CArrayBlock::InsertBlockAfterEnd(paElements, iLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertBlockBeforeStart(M* paElements, int iLength)
{
	return (M*)CArrayBlock::InsertBlockBeforeStart(paElements, iLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertBlockAt(M* paElements, int iLength, int iIndex)
{
	return (M*)CArrayBlock::InsertBlockAt(paElements, iLength, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::Get(int iIndex)
{
	return (M*)CArrayBlock::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplate<M>::GetIndex(M* pvData)
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
void CArrayTemplate<M>::Fake(M* pvData, int iNum, int iChunkSize)
{
	CArrayBlock::Fake(sizeof(M), pvData, iNum, iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M& CArrayTemplate<M>::operator[](int iIndex)
{
	return ((M*)this->mpvArray)[iIndex];
}


#endif // !__TEMPLATE_ARRAY_H__

