/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __INDEX_OBJECT_H__
#define __INDEX_OBJECT_H__
#include "StandardLib/IndexUnknown.h"
#include "Collection.h"
#include "Pointer.h"


//Consider which methods using Iterate internally could rather use the Unsafe iterators.
class CIndexObject : public CCollection
{
CONSTRUCTABLE(CIndexObject)
DESTRUCTABLE(CIndexObject)
protected:
	CIndexUnknown	mcIndex;

public:
	Ptr<CIndexObject>	Init(void);
	void				Class(void) override;
	void				Free(void) override;

	bool				Put(char* szKey, CPointer& pObject);
	bool				Put(uint8* pvKey, size iKeySize, CPointer& pObject);

	size 				NumElements(void) override;
	size 				NonNullElements(void) override;
	bool				IsEmpty(void) override;

	void				SetPointerTosExpectedDistToRoot(int iDistToRoot) override;

	size 				NumPointerTos(void) override;
	size 				BaseNumPointerTos(void)  override;
	void				GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos) override;
	void				BaseGetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos) override;
	bool				ContainsPointerTo(CEmbeddedObject* pcEmbedded) override;
	bool				RemoveAll(void);
	void				RemoveAllPointerTosDontFree(void) override;
	void				RemoveAllPointerTosTryFree(void) override;
	void				CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters) override;
	void				UnsafePointTo(CEmbeddedObject* pcNew, CEmbeddedObject* pcOld) override;

	bool				Remove(char* szKey);
	bool				Remove(uint8* pvKey, size iKeySize);

	CPointer			Get(char* szKey);
	CPointer			Get(uint8* pvKey, size iKeySize);
	template<class M>
	Ptr<M>				Get(char* szKey);
	template<class M>
	Ptr<M>				Get(uint8* pvKey, size iKeySize);

	CPointer			StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, bool* bExists = NULL);
	CPointer			Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, bool* bExists = NULL);
	template<class M>
	Ptr<M>				StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, bool* bExists = NULL);
	template<class M>
	Ptr<M>				Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, bool* bExists = NULL);

	CPointer			StartIterationPointer(SIndexTreeMemoryIterator* psIter);
	CPointer			IteratePointer(SIndexTreeMemoryIterator* psIter);

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;
	
	void				TouchAll(void) override;
	void				KillAll(void) override;

	CEmbeddedObject*	GetEmbeddedObject(size iIndex) override;
	void				ValidatePointerTos(void) override;
	CIndexUnknown*		GetIndexForTesting(void);

protected:
	void				FreePointers(void) override;
	void				RemovePointerTo(CEmbeddedObject* pcTo) override;
	size 				RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew) override;
	void				SetPointedTosDistToRoot(int iDistToRoot);

	void				UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist) override;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CIndexObject::Get(char* szKey)
{
	return (M*)Get(szKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CIndexObject::Get(uint8* pvKey, size iKeySize)
{
	return (M*)Get(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CIndexObject::StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, bool* bExists)
{
	return (M*)StartIteration(psIterator, pvKey, piKeySize, iMaxKeySize, bExists);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CIndexObject::Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, bool* bExists)
{
	return (M*)Iterate(psIterator, pvKey, piKeySize, iMaxKeySize, bExists);
}


#endif // __INDEX_OBJECT_H__

