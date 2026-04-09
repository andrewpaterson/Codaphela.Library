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
#ifndef __ARRAY_COMMON_OBJECT_H__
#define __ARRAY_COMMON_OBJECT_H__
#include "Collection.h"
#include "ArrayCommonUnknown.h"


//Do not sub-class this class (unless you clean up the destructor calls).
class CArrayCommonObject : public CCollection
{
CONSTRUCTABLE(CArrayCommonObject)
protected:
	CArrayCommonUnknown		mcArray;
	bool					mbSorted;

public:
							~CArrayCommonObject();
	Ptr<CArrayCommonObject>	Init(bool bUnique, bool bIgnoreNull, bool bPreserveOrder, bool bSortPointers);
	void					Class(void) override;

	bool					Add(CPointer& pObject);
	bool					Add(CEmbeddedObject* pcObject);
	bool					AddAll(CArrayCommonObject* pcArray);
	bool					Set(size iIndex, CPointer& pObject);
	CEmbeddedObject*		Dereference(CEmbeddedObject** ppcObject);

	bool					Remove(CPointer& pObject);
	bool					Remove(CEmbeddedObject* pcObject);
	bool					RemoveEnd(size iIndexInclusive);

	size 					NumElements(void) override;
	size 					NonNullElements(void) override;
	bool					IsEmpty(void) override;

	size 					NumPointerTos(void) override;
	size 					BaseNumPointerTos(void)  override;
	void					GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos) override;
	void					BaseGetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos) override;
	bool					ContainsPointerTo(CEmbeddedObject* pcEmbedded) override;
	void					RemoveAllPointerTosDontFree(void) override;
	bool					RemoveAllPointerTosTryFree(void) override;
	void					CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters) override;

	void					GrowTo(size iNumElements);

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;
	void					Sort(void);

	CBaseObject*			UnsafeGet(size  iIndex);
	bool					UnsafeRemove(CBaseObject* pcObject);

	CEmbeddedObject*		GetEmbeddedObject(size iIndex) override;

	void					ValidatePointerTos(void) override;

	CPointer				StartIterationPointer(SSetIterator* psIter, bool* pbExists);
	CPointer				IteratePointer(SSetIterator* psIter, bool* pbExists);

	void					TouchAll(void) override;
	void					KillAll(void) override;

	void					EnsureSorted(void);
	bool					IsSorted(void);
	bool					IsMustSort(void);

protected:
	void					FreePointers(void) override;
	void					Free(void) override;
	void					RemovePointerTo(CEmbeddedObject* pcTo) override;
	size 					RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew) override;
	void					SetPointedTosDistToRoot(int iDistToRoot) override;

	void					UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist) override;

	bool					InsertAt(size iIndex, CPointer& pObject);
	bool					InsertAt(size iIndex, CEmbeddedObject* pcObject);
	bool					RemoveAt(size iIndex);
};


#endif // __ARRAY_COMMON_OBJECT_H__

