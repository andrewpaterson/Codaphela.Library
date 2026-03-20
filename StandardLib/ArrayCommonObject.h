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


class CArrayCommonObject : public CCollection
{
CONSTRUCTABLE(CArrayCommonObject);
DESTRUCTABLE(CArrayCommonObject);
protected:
	CArrayCommonUnknown		mcArray;

public:
	Ptr<CArrayCommonObject>	Init(bool bUnique, bool bIgnoreNull, bool bPreserveOrder);
	void					Class(void) override;

	bool					Add(CPointer& pObject);
	bool					Add(CEmbeddedObject* pcObject);
	bool					AddAll(CArrayCommonObject* pcArray);
	bool					Set(size iIndex, CPointer& pObject);

	bool					Remove(CPointer& pObject);
	bool					Remove(CEmbeddedObject* pcObject);
	bool					RemoveEnd(size iIndexInclusive);
	bool					RemoveAll(void) override;

	size 					NumElements(void) override;
	size 					NonNullElements(void) override;
	bool					IsEmpty(void) override;

	void					SetPointerTosExpectedDistToRoot(int iDistToRoot) override;

	size 					NumPointerTos(void) override;
	size 					BaseNumPointerTos(void)  override;
	void					GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos) override;
	void					BaseGetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos) override;
	bool					ContainsPointerTo(CEmbeddedObject* pcEmbedded) override;
	void					RemoveAllPointerTosDontFree(void) override;
	void					RemoveAllPointerTosTryFree(void) override;
	void					CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters) override;

	void					GrowTo(size iNumElements);

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	CBaseObject*			UnsafeGet(size  iIndex);
	bool					UnsafeRemove(CBaseObject* pcObject);
	CEmbeddedObject*		GetEmbeddedObject(size iIndex);
	void					UnsafePointTo(CEmbeddedObject* pcNew, CEmbeddedObject* pcOld);

	void					BaseValidatePointerTos(void) override;
	void					ValidatePointerTos(void) override;
	void					ValidateConsistency(void) override;
	CPointer				StartIterationPointer(SSetIterator* psIter) override;
	CPointer				IteratePointer(SSetIterator* psIter) override;

	void					TouchAll(void) override;
	void					KillAll(void) override;

protected:
	void					FreePointers(void) override;
	void					Free(void) override;
	void					RemovePointerTo(CEmbeddedObject* pcTo) override;
	size 					RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew) override;
	void					SetPointedTosDistToRoot(int iDistToRoot);

	void					UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist) override;

	bool					InsertAt(size iIndex, CPointer& pObject);
	bool					InsertAt(size iIndex, CEmbeddedObject* pcObject);
	bool					RemoveAt(size iIndex);
};


#endif // __ARRAY_COMMON_OBJECT_H__

