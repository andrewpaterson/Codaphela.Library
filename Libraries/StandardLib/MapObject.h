/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
	bool					mbSorted;	//Whether mcMap is in key order: it keeps no flag of its own, and a key loaded or remapped where it is changes the order.

public:
						~CMapObject();
	void				Init(void);
	void				Class(void) override;
	void				Free(void) override;

	bool				Put(const CPointer& pKey, const CPointer& pValue);

	size 				NumElements(void) override;
	size 				NonNullElements(void) override;
	bool				IsEmpty(void) override;


	size 				BaseNumPointerTos(void)  override;
	void				BaseGetPointerTos(CArrayEmbeddedObjectPtr* papcTos) override;
	bool				ClearPointerTos(bool bFree) override;

	bool				Remove(const CPointer& pKey);

	CPointer			Get(const CPointer& pKey);
	bool				Contains(const CPointer& pKey);

	CMapEntry			StartIteration(SMapIterator* psIter);
	CMapEntry			Iterate(SMapIterator* psIter);

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;
	void				Sort(void);

	void				TouchAll(void) override;
	void				KillAll(void) override;

	CEmbeddedObject*	GetEmbeddedObject(size iMap) override;
	CMapUnknownUnknown*	GetUnknownMap(void);

	bool				IsSorted(void);
	void				ValidateInternalConsistency(void) override;

protected:
	void				PreFields(void) override;
	void				FreePointers(void) override;
	void				RemovePointerTo(CEmbeddedObject* pcTo) override;
	size 				RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew) override;

	bool				CanLocateMembers(void) override;
	void				AddMemberLocations(CPointerLocations* pcLocations) override;
	bool				IsMemberAt(CEmbeddedObject* pcMember, ptr uiLocation) override;
	size				NumLocatedMembers(void) override;
	void				RemoveMemberAt(ptr uiLocation) override;
	void				ReplaceMemberAt(ptr uiLocation, CEmbeddedObject* pcMember) override;
	void				AddValueLocation(CEmbeddedObject* pcValue, CEmbeddedObject* pcKey);
	void				RemoveValueLocation(CEmbeddedObject* pcValue, CEmbeddedObject* pcKey);


	void				EnsureSorted(void);
	CMapEntry			Entry(bool bExists, CBaseObject* pcKey, CBaseObject* pcValue);
};


#endif // __MAP_OBJECT_H__

