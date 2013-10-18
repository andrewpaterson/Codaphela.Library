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
	void				PreInit(CObjects* pcObjects);
	void				PreInit(void);
	void				Kill(void);
	BOOL				IsCollection(void);
	BOOL				IsObject(void);
	void				SetDistToRootAndSetPointedTosExpectedDistToRoot(int iDistToRoot);
	CPointer*			Pointer(CPointer* pcPointer);
	void				Embedded(CBaseObject* pcObject);
	BOOL				IsDirty(void);
	int					GetEmbeddedIndex(CEmbeddedObject* pcEmbedded);
	int					GetNumEmbedded(void);
	CEmbeddedObject*	GetEmbeddedObject(int iIndex);
	CBaseObject*		Dehollow(void);
	int					NumHeapFroms(void);
	int					NumStackFroms(void);
	void				SetFlag(int iFlag, int iFlagValue);
	void				ClearDistToRoot(void);
	void				GetHeapFroms(CArrayEmbeddedBaseObjectPtr* papcFroms);
	void				GetStackFroms(CArrayPointerPtr* papcFroms);
	int					NumTos(void);
	int					UnsafeNumEmbeddedObjectTos(void);
	void				GetTos(CArrayEmbeddedObjectPtr* papcTos);
	BOOL				ContainsTo(CEmbeddedObject* pcEmbedded);
	void				UnsafeGetEmbeddedObjectTos(CArrayEmbeddedObjectPtr* papcTos);
	void				ValidateConsistency(void);

protected:
	void				KillDontFree(void);
	void				KillInternalData(void);
	void				RemoveTo(CEmbeddedObject* pcTo);
	void				RemoveAllTos(CArrayEmbeddedBaseObjectPtr* papcFromsChanged);
	void				RemoveEmbeddedObjectAllTos(CArrayEmbeddedBaseObjectPtr* papcFromsChanged);
	void				RemoveAllHeapFroms(void);
	void				RemoveAllStackFroms(void);
	CEmbeddedObject*	GetClosestFromToRoot(void);
	void				CollectPointedToToBeKilled(CArrayBaseObjectPtr* papcKilled);
	int					RemapTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew);
	int					CalculateDistToRootFromPointedFroms(int iDistToRoot);
	void				Free(void);
	void				SetPointedTosDistToRoot(int iDistToRoot);
	BOOL				RecurseGetEmbeddedIndex(CEmbeddedObject* pcTest, int* piIndex);
	CEmbeddedObject*	RecurseGetEmbeddedObject(int iIndex, int* iCount);
};


#endif // __OBJECT_H__

