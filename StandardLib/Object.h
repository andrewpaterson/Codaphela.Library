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
#include "BaseObject.h"


class CObject : public CBaseObject
{
template<class M>
friend class Ptr;
friend class CPointer;
friend class CObjects;
friend class CObjectGraphDeserialiser;

BASE_FUNCTIONS(CObject);
protected:
	CArrayEmbedded<CPointer*, 5>		mapPointers;  //Pointers in this object.  
	CArrayEmbedded<CBaseObject*, 3>		mapEmbedded;  //Objects embedded in this object

public:
						CObject();
	void				Allocate(CObjects* pcObjects);
	void				Kill(void);
	BOOL				IsCollection(void);
	BOOL				IsObject(void);
	void				SetPointerTosExpectedDistToRoot(int iDistToRoot);
	CPointer*			Pointer(CPointer* pcPointer);
	void				Embedded(CBaseObject* pcObject);
	BOOL				IsDirty(void);
	int					GetEmbeddedIndex(CEmbeddedObject* pcEmbedded);
	unsigned short int	GetNumEmbedded(void);
	CEmbeddedObject*	GetEmbeddedObject(unsigned short int iIndex);
	CBaseObject*		Dehollow(void);
	int					NumHeapFroms(void);
	int					NumStackFroms(void);
	void				SetFlag(int iFlag, int iFlagValue);
	void				GetHeapFroms(CArrayEmbeddedBaseObjectPtr* papcFroms);
	void				GetStackFroms(CArrayPointerPtr* papcFroms);
	CBaseObject*		GetClosestFromToRoot(void);
	CBaseObject*		GetClosestFromToStack(void);
	int					NumPointerTos(void);
	int					UnsafeNumEmbeddedObjectTos(void);
	void				GetPointerTos(CArrayEmbeddedObjectPtr* papcTos);
	BOOL				ContainsPointerTo(CEmbeddedObject* pcEmbedded);
	void				CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters);
	int					CollectEmbeddedObjectDetachedFroms(CDistCalculatorParameters* pcParameters);
	void				UnsafeGetEmbeddedObjectPointerTos(CArrayEmbeddedObjectPtr* papcTos);

	int					GetFieldPointerToIndex(CPointer* pcFieldPointer);
	BOOL				RecurseGetFieldPointerToIndex(CPointer* pcTest, int* piIndex);
	int					GetNumFieldPointerTos(void);
	CPointer*			GetFieldPointerTo(int iIndex);
	CPointer*			RecurseGetFieldPointerTo(int iIndex, int* piCount);

	void				ValidatePointerTos(void);
	void				ValidateConsistency(void);

protected:
	void				KillDontFree(void);
	void				KillInternalData(void);
	void				RemovePointerTo(CEmbeddedObject* pcTo);
	void				UnsafeRemoveAllPointerTos(void);
	void				RemoveAllPointerTos(void);
	void				UnsafeRemoveEmbeddedObjectAllPointerTos(void);
	void				RemoveEmbeddedObjectAllPointerTos(void);
	void				RemoveAllHeapFroms(void);
	void				RemoveAllStackFroms(void);
	CBaseObject*		GetClosestFromForCanFindRoot(void);
	int					RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew);
	void				UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist);
	int					CalculateDistToRootFromPointedFroms(int iDistToRoot);
	void				SetPointedTosDistToRoot(int iDistToRoot);
	BOOL				SetDistToRoot(int iDistToRoot);
	void				SetDistToStack(int iDistToStack);
	BOOL				RecurseGetEmbeddedIndex(CEmbeddedObject* pcTest, int* piIndex);
	CEmbeddedObject*	RecurseGetEmbeddedObject(int iIndex, int* iCount);
	void				ValidateEmbeddedObjectPointerTos(void);
	void				ValidateEmbeddedConsistency(void);
	BOOL				IsDistToRootValid(void);
};


#endif // __OBJECT_H__

