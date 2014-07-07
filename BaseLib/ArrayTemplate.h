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
#ifndef __TEMPLATE_ARRAY_H__
#define __TEMPLATE_ARRAY_H__
#include "ArrayBase.h"


template<class M>
class CArrayTemplate : public CArrayBase
{
public:
	void 	Init(void);
	void 	Init(int iChunkSize);
	void	Init(CArrayTemplate<M>* pcTemplateArray);
	void 	Allocate(int iNumElements);
	void 	Allocate(int iChunkSize, int iNumElements);
	void	Fake(M* pvData, int iNum);

	M*		Add(void);
	M*		Add(M* pvData);
	M* 		AddGetIndex(int* piIndex);
	int 	AddIfUnique(M* pData);
	int 	AddIfUniqueKey(M* pData, int iKeyOffset, int iKeySize);

	BOOL 	Copy(CArrayTemplate<M>* pcTemplateArray);

	M*		Get(int iIndex);
	M*		SafeGet(int iIndex);
	M*		GetData(void);
	int		GetIndex(M* pvElement);
	M*		Tail(void);
	M&		operator[](int iIndex);

	void	InsertArrayAfterEnd(CArrayTemplate<M>* pcTemplateArray);
	void	InsertArrayAt(CArrayTemplate<M>* pcTemplateArray, int iIndex);
	void	InsertArrayBeforeStart(CArrayTemplate<M>* pcTemplateArray);
	M* 		InsertAt(int iIndex);
	M*	 	InsertAt(M* pvData, int iIndex);
	void	InsertBlockAfterEnd(M* paElements, int iLength);
	void	InsertBlockAt(M* paElements, int iIndex, int iLength);
	void	InsertBlockBeforeStart(M* paElements, int iLength);
	int		InsertIntoSorted(int(*)(const void*, const void*), M* pvElement, BOOL bOverwriteExisting);
	M*		InsertNumElementsAt(int iNumElements, int iIndex);

	void	Pop(M* pvData);
	void	Pop(void);
	void 	Push(M* pvElement);
	M*		Push(void);

	BOOL	Contains(M* pData);
	BOOL	Equals(CArrayTemplate<M>* pcTemplateArray);
	int 	Find(M* pData);
	BOOL	FindInSorted(M* pData, int(*)(const void*, const void*), int* piIndex);
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
	CArrayBase::Init(sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Init(int iChunkSize)
{
	CArrayBase::Init(sizeof(M), iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Allocate(int iNumElements)
{
	CArrayBase::Allocate(sizeof(M), iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Allocate(int iChunkSize, int iNumElements)
{
	CArrayBase::Allocate(sizeof(M), iChunkSize, iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Init(CArrayTemplate<M>* pcTemplateArray)
{
	CArrayBase::Init(pcTemplateArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::SafeGet(int iIndex)
{
	return (M*)CArrayBase::SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplate<M>::SafeSet(int iIndex, M* pvData)
{
	return CArrayBase::SafeSet(iIndex, pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::Add(void)
{
	return (M*)CArrayBase::Add();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::AddGetIndex(int* piIndex)
{
	return (M*)CArrayBase::AddGetIndex(piIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::Add(M* pvData)
{
	return (M*)CArrayBase::Add(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Set(int iIndex, M* pvData)
{
	return CArrayBase::Set(iIndex, pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertAt(int iIndex)  //The new element will be at iIndex
{
	return (M*)CArrayBase::InsertAt(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertAt(M* pvData, int iIndex)
{
	return (M*)CArrayBase::InsertAt(pvData, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplate<M>::Copy(CArrayTemplate* pcTemplateArray)
{
	return CArrayBase::Copy(pcTemplateArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Push(M* pvElement)
{
	CArrayBase::Push(pvElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::Push(void)
{
	return (M*)CArrayBase::Push();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Pop(M* pvElement)
{
	CArrayBase::Pop(pvElement);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Pop(void)
{
	CArrayBase::Pop();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::Tail(void)
{
	return (M*)CArrayBase::Tail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplate<M>::AddIfUnique(M* pData)
{
	return CArrayBase::AddIfUnique(pData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplate<M>::Find(M* pData)
{
	return CArrayBase::Find(pData);
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplate<M>::Contains(M* pData)
{
	return CArrayBase::Contains(pData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplate<M>::AddIfUniqueKey(M* pData, int iKeyOffset, int iKeySize)
{
	return CArrayBase::AddIfUniqueKey(pData, iKeyOffset, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplate<M>::FindWithKey(M* pData, int iKeyOffset, int iKeySize)
{
	return CArrayBase::FindWithKey(pData, iKeyOffset, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplate<M>::Equals(CArrayTemplate<M>* pcTemplateArray)
{
	return CArrayBase::Equals(pcTemplateArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplate<M>::InsertIntoSorted(int(* Func)(const void*, const void*), M* pvElement, BOOL bOverwriteExisting)
{
	return CArrayBase::InsertIntoSorted(Func, pvElement, bOverwriteExisting);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayTemplate<M>::FindInSorted(M* pData, int(* Func)(const void*, const void*), int* piIndex)
{
	return CArrayBase::FindInSorted(pData, Func, piIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::InsertArrayAfterEnd(CArrayTemplate<M>* pcTemplateArray)
{
	CArrayBase::InsertArrayAfterEnd(pcTemplateArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::InsertArrayBeforeStart(CArrayTemplate<M>* pcTemplateArray)
{
	CArrayBase::InsertArrayBeforeStart(pcTemplateArray);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::InsertArrayAt(CArrayTemplate<M>* pcTemplateArray, int iIndex)
{
	CArrayBase::InsertArrayAt(pcTemplateArray, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::InsertNumElementsAt(int iNumElements, int iIndex)
{
	return (M*)CArrayBase::InsertNumElementsAt(iNumElements, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::InsertBlockAfterEnd(M* paElements, int iLength)
{
	CArrayBase::InsertBlockAfterEnd(paElements, iLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::InsertBlockBeforeStart(M* paElements, int iLength)
{
	CArrayBase::InsertBlockBeforeStart(paElements, iLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::InsertBlockAt(M* paElements, int iLength, int iIndex)
{
	CArrayBase::InsertBlockAt(paElements, iLength, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::Get(int iIndex)
{
	return (M*)CArrayBase::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplate<M>::GetIndex(M* pvElement)
{
	return CArrayBase::GetIndex(pvElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayTemplate<M>::GetData(void)
{
	return (M*)CArrayBase::GetData();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplate<M>::Fake(M* pvData, int iNum)
{
	CArrayBase::Fake(sizeof(M), pvData, iNum);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M& CArrayTemplate<M>::operator[](int iIndex)
{
	return ((M*)mpvArray)[iIndex];
}


#endif //__TEMPLATE_ARRAY_H__

