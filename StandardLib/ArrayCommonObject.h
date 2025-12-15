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


class CArrayCommonUnknown;
class CArrayCommonObject : public CCollection
{
CONSTRUCTABLE(CArrayCommonObject);
DESTRUCTABLE(CArrayCommonObject);
protected:
	CArrayCommonUnknown		mcArray;
	bool					mbSubRoot;

public:
	Ptr<CArrayCommonObject>	Init(bool bUnique, bool bIgnoreNull, bool bPreserveOrder);
	void					Class(void) override;

	bool					Add(CPointer& pObject);
	bool					Add(CEmbeddedObject* pcObject);
	bool					AddAll(CArrayCommonObject* pcArray);
	bool					Set(size iIndex, CPointer& pObject);

	bool					Remove(CPointer& pObject);
	bool					Remove(CEmbeddedObject* pcObject);
	bool					RemoveAll(void);
	bool					Clear(void);

	size 					NumElements(void);
	size 					NonNullElements(void);
	size 					Size(void);
	size 					Length(void);
	bool					IsEmpty(void);

	void					MakeSubRoot(void);
	bool					IsSubRoot(void);
	void					SetPointerTosExpectedDistToRoot(int iDistToRoot);

	size 					NumPointerTos(void);
	size 					BaseNumPointerTos(void);
	void					GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
	void					BaseGetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
	bool					ContainsPointerTo(CEmbeddedObject* pcEmbedded);
	void					RemoveAllPointerTosDontFree(void);
	void					RemoveAllPointerTosTryFree(void);
	void					CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters);

	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	CBaseObject*			UnsafeGet(size  iIndex);
	bool					UnsafeRemove(CBaseObject* pcObject);
	CEmbeddedObject*		GetEmbeddedObject(size iIndex);
	void					UnsafePointTo(CEmbeddedObject* pcNew, CEmbeddedObject* pcOld);

	void					BaseValidatePointerTos(void);
	void					ValidatePointerTos(void);
	void					ValidateConsistency(void);

	void					TouchAll(void);
	void					KillAll(void);

protected:
	void					FreePointers(void) override;
	void					Free(void) override;
	void					RemovePointerTo(CEmbeddedObject* pcTo);
	size 					RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew);
	void					SetPointedTosDistToRoot(int iDistToRoot);

	void					UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist);

	bool					InsertAt(size iIndex, CPointer& pObject);
	bool					InsertAt(size iIndex, CEmbeddedObject* pcObject);
	bool					RemoveAt(size iIndex);
};


#endif // __ARRAY_COMMON_OBJECT_H__

