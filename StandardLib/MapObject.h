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
#ifndef __MAP_OBJECT_H__
#define __MAP_OBJECT_H__
#include "StandardLib/MapUnknownUnknown.h"
#include "ArrayCommonObject.h"
#include "MapEntry.h"
#include "Pointer.h"


//Do not sub-class this class (unless you clean up the destructor calls).
class CMapObject : public CCollection
{
CONSTRUCTABLE(CMapObject)
protected:
	CMapUnknownUnknown		mcMap;
	bool					mbSorted;

public:
						~CMapObject();
	Ptr<CMapObject>		Init(void);
	void				Class(void) override;
	void				Free(void) override;

	bool				Put(CPointer& pKey, CPointer& pValue);

	size 				NumElements(void) override;
	size 				NonNullElements(void) override;
	bool				IsEmpty(void) override;

	void				SetPointerTosExpectedDistToRoot(int iDistToRoot) override;

	size 				NumPointerTos(void) override;
	size 				BaseNumPointerTos(void)  override;
	void				GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos) override;
	void				BaseGetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos) override;
	bool				ContainsPointerTo(CEmbeddedObject* pcEmbedded) override;
	void				RemoveAllPointerTosDontFree(void) override;
	bool				RemoveAllPointerTosTryFree(void) override;
	void				CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters) override;

	bool				Remove(CPointer& pKey);

	CPointer			Get(CPointer& pKey);
	template<class M, class N>
	Ptr<N>				Get(Ptr<M>& pKey);

	CMapEntry			StartIteration(SMapIterator* psIter);
	CMapEntry			Iterate(SMapIterator* psIter);

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;
	void				Sort(void);

	void				TouchAll(void) override;
	void				KillAll(void) override;

	CEmbeddedObject*	GetEmbeddedObject(size iMap) override;
	void				ValidatePointerTos(void) override;
	CMapUnknownUnknown*	GetUnknownMap(void);

	bool				IsSorted(void);
	void				ValidateInternalConsistency(void);

protected:
	void				FreePointers(void) override;
	void				RemovePointerTo(CEmbeddedObject* pcTo) override;
	size 				RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew) override;
	void				SetPointedTosDistToRoot(int iDistToRoot);

	void				UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist) override;

	void				EnsureSorted(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
Ptr<N> CMapObject::Get(Ptr<M>& pKey)
{
	return (N*)Get(pKey);
}


#endif // __MAP_OBJECT_H__

