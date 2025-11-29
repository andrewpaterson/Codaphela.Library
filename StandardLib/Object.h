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
#ifndef __OBJECT_H__
#define __OBJECT_H__
#include "Integer.h"
#include "Float.h"
#include "Character.h"
#include "PrimitiveVoid.h"
#include "BaseObject.h"

/* Implement these on anything extending CObject
	void	Init(void);
	void	Class(void) override;
	void 	Free(void) override;

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;
*/


class CObject : public CBaseObject
{
template<class M>
friend class Ptr;
friend class CPointer;
friend class CObjects;
friend class CExternalObjectDeserialiser;

CONSTRUCTABLE(CObject);
protected:
	//Both pointers and embedded objects are now explicitly setup in the class.  Remove these fields.
	CArrayTemplateEmbedded<CPointer*, 5>		mapPointers;  //Pointers in this object.  
	CArrayTemplateEmbedded<CBaseObject*, 3>		mapEmbedded;  //Objects embedded in this object

public:
						CObject();
	void				Allocate(CObjects* pcObjects);
	void				Kill(void) final;
	bool				IsCollection(void) override;
	bool				IsObject(void) override;
	void				SetPointerTosExpectedDistToRoot(int iDistToRoot);

	bool				IsDirty(void);
	size				GetEmbeddedIndex(CEmbeddedObject* pcEmbedded);
	size				NumEmbedded(void);
	CEmbeddedObject*	GetEmbeddedObject(size iIndex);
	CBaseObject*		Dehollow(void);
	size				NumHeapFroms(void);
	size				NumStackFroms(void);
	void				SetFlag(uint16 iFlag, uint16 iFlagValue);
	void				GetHeapFroms(CArrayTemplateEmbeddedBaseObjectPtr* papcFroms);
	void				GetStackFroms(CArrayStackPointer* papcFroms);
	CBaseObject*		GetClosestFromToRoot(void);
	CBaseObject*		GetClosestFromToStack(void);
	size				NumPointerTos(void);
	size				BaseNumPointerTos(void);
	void				GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
	void				BaseGetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos);
	bool				ContainsPointerTo(CEmbeddedObject* pcEmbedded);
	void				CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters);
	void				BaseCollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters);
	size				CollectDetachedFroms(CDistCalculatorParameters* pcParameters);

	size				GetFieldPointerToIndex(CPointer* pcFieldPointer);
	size				GetNumFieldPointerTos(void);
	CPointer*			GetFieldPointerTo(size iIndex);

	void				ValidatePointerTos(void);
	void				ValidateConsistency(void);

public:


protected:
	void				EmbedFields(void);
	void				FreeInternal(bool bAllocatedInObjects) override;
	void				FreePointers(void) override;
	void				RemovePointerTo(CEmbeddedObject* pcTo);
	void				RemoveAllPointerTosDontFree(void);
	void				RemoveAllPointerTosTryFree(void);
	void				RemoveAllHeapFroms(void);
	void				RemoveAllStackFroms(void);
	CBaseObject*		GetClosestFromForCanFindRoot(void);
	size				RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew) override;
	void				UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist);
	void				BaseUpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist);
	int					CalculateDistToRootFromPointedFroms(int iDistToRoot);
	void				SetPointedTosDistToRoot(int iDistToRoot);
	bool				SetDistToRoot(int iDistToRoot) override;
	void				SetDistToStack(int iDistToStack);
	bool				RecurseGetEmbeddedIndex(CEmbeddedObject* pcTest, size* piIndex);
	CEmbeddedObject*	RecurseGetEmbeddedObject(size iIndex, size* iCount);
	bool				RecurseGetFieldPointerToIndex(CPointer* pcTest, size* piIndex);
	CPointer*			RecurseGetFieldPointerTo(size iIndex, size* piCount);
	void				BaseValidatePointerTos(void);
	void				ValidateEmbeddedConsistency(void);
	bool				IsDistToRootValid(void);
	void				EmbedPointerFields(void);
	void				EmbedEmbeddedObjectFields(void);
	void				ClassNotImplemented(void);
};


#endif // __OBJECT_H__

