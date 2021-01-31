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
#ifndef __BASE_OBJECT_H__
#define __BASE_OBJECT_H__
#include "CoreLib/IndexedGeneral.h"
#include "EmbeddedObject.h"


//Tested for root is only valid whilst the scene graph is calling CanFindRoot.  It stops the graph from walking already tested objects.
#define OBJECT_FLAGS_TESTED_FOR_ROOT			  0x02

//Invalidated is set when the object on the file system is changed and must be reloaded.  This objects does not use it.
#define OBJECT_FLAGS_INVALIDATED				  0x04

//Dirty must be manually set when an object needs to be written from memory to indexed data.  Objects are - by default always dirty.
#define OBJECT_FLAGS_DIRTY						  0x08

//Debug flag marking whether or not an object has had Free called on it.  An object that is killed should be removed from Memory so an object with this flag set is broken.
#define OBJECT_FLAGS_FREED						  0x10

//Debug flag marking whether or not an object has had it's graph dumped yet.
#define OBJECT_FLAGS_DUMPED						  0x20

//This object cannot be reached and is marked for killing.
#define OBJECT_FLAGS_UNREACHABLE				  0x40

//Tested for sanity is only valid whilst the scene graph is calling ValidateConsistency.  It stops the graph from walking already tested objects.
#define OBJECT_FLAGS_TESTED_FOR_SANITY			  0x80

//Object initialisation life-cycle
#define OBJECT_FLAGS_CALLED_CONSTRUCTOR			 0x100
#define OBJECT_FLAGS_CALLED_ALLOCATE			 0x200
#define OBJECT_FLAGS_CALLED_INIT				 0x400
#define OBJECT_FLAGS_CALLED_CLASS				 0x800
#define OBJECT_FLAGS_CALLED_KILL		       0x10000

#define OBJECT_FLAGS_CLEARED_DIST_TO_ROOT		0x1000
#define OBJECT_FLAGS_UPDATED_TOS_DIST_TO_ROOT	0x2000
#define OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED	0x4000
#define OBJECT_FLAGS_DIST_FROM_WALKED			0x8000

class CObjectDeserialiser;
class CObjectSerialiser;
class CObjects;
class CObjectRemapFrom;
class CBaseObject : public CEmbeddedObject
{
template<class M>
friend class Ptr;
friend class CPointer;
friend class CArrayCommonObject;
friend class CObject;
friend class CObjects;
friend class CObjectRemapFrom;
friend class CEmbeddedObject;

CONSTRUCTABLE(CBaseObject);
protected:
	OIndex				moi;
	CObjects*			mpcObjectsThisIn;
	int					miDistToRoot;
	int					miDistToStack;
	unsigned short int	miFlags;
	unsigned short int  miNumEmbedded;
	unsigned short int	miPreInits;
	unsigned short int	miPostInits;

public:
								CBaseObject();
								~CBaseObject();

	virtual void				Allocate(CObjects* pcObjects);
	virtual	void				Class(void);

			void				PreInit(void);
			void				PostInit(void);
	virtual void				Initialised(void);

			void				Kill(void);

	virtual void				Free(void) =0;

			OIndex				GetOI(void);
			void				SetObjectID(OIndex oi);
			void				ClearIndex(void);

	virtual BOOL				IsRoot(void);
	virtual BOOL				IsSubRoot(void);
			BOOL				IsUnknown(void);
			BOOL				IsHollow(void);
	virtual BOOL				IsCollection(void) =0;
	virtual BOOL				IsObject(void) =0;
	virtual BOOL				IsNamed(void);
			BOOL				IsInvalidated(void);
	virtual BOOL				IsDirty(void);
			BOOL				IsUpdateAttachedPointerTosDistToRoot(void);
			BOOL				IsInitialised(void);
			BOOL				HasClass(void);

	virtual char*				GetName(void);
	virtual BOOL				SetName(char* szName);
			int					SerialisedSize(void);
	virtual char*				GetIdentifier(CChars* psz);

			unsigned short int	GetNumEmbedded(void);

	virtual void				SetPointerTosExpectedDistToRoot(int iDistToRoot) =0;
			void				SetDirty(void);
			int					GetDistToRoot(void);
			int					GetDistToStack(void);
	virtual BOOL				SetDistToRoot(int iDistToRoot);
			BOOL				TestedForRoot(void);
			void				UpdateAttachedTosDistToRoot(CDistCalculatorParameters* pcParameters);
			void				CollectValidDistStartingObjectsAndSetClearedToRoot(CBaseObject* pcTo, CDistCalculatorParameters* pcParameters);
			void				CollectAndClearInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters);
	virtual BOOL				IsDistToRootValid(void);
			int					CollectDetachedAndSetDistToStackZero(CDistCalculatorParameters* pcParameters);
			int					CollectDetachedFroms(CDistCalculatorParameters* pcParameters);

			void				AddExpectedDistToRoot(CEmbeddedObject* pcPointedTo, int iExpectedDist, CDistCalculatorParameters* pcParameters);
			void				ClearDistTouchedFlags(void);
			BOOL				HasDistTouchedFlag(void);

			BOOL				TestedForSanity(void);
			CObjects*			GetObjects(void);
			CStackPointers*		GetStackPointers(void);
	virtual void				SetDistToStack(int iDistToStack);

	virtual BOOL				ContainsPointerTo(CEmbeddedObject* pcEmbedded);
			CEmbeddedObject* 	TestGetPointerTo(int iToIndex);
			int 				TestGetNumEmbeddedFromFlags(void);
			void				ClearFlagNumEmbedded(void);
	virtual void				SetFlag(int iFlag, int iFlagValue);
			int					GetFlags(void);
			BOOL				CanFindRoot(void);
			BOOL				CanFindRootThroughValidPath(void);

			void				DumpFroms(void);
			void				DumpPointerTos(void);
			void				Dump(void);

			void				ValidateFlagSet(int iFlag, char* szFlag);
			void				ValidateFlagNotSet(int iFlag, char* szFlag);
			void				ValidateContainerFlag(void);
			void				ValidateFlags(void);
			void				ValidateDistToRoot(void);
			void				ValidateIndex(void);
			void				ValidateObjectsThisIn(void);
			void				ValidateCanFindRoot(void);
	virtual void				ValidateEmbeddedConsistency(void);
	virtual void				ValidateObjectIdentifiers(void);
			void				ValidateBaseObjectDetail(void);
			void				ValidateAllocation(void);
			void				ValidateHasClass(void);
			void				ValidateInitCalled(void);
	
protected:
	virtual void				InternalFree(void);
			void				Kill(BOOL bHeapFromChanged);
			void				TryFree(BOOL bKillIfNoRoot, BOOL bHeapFromChanged);

	virtual void				RemoveAllPointerTosDontKill(void) =0;
	virtual void				RemoveAllPointerTos(void) =0;

	virtual void				FreeIdentifiers(void);
			void				FreePointers(void);
			int					RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew) =0;
			BOOL				RemoveToFrom(CEmbeddedObject* pcPointedTo);
	virtual void				BaseRemoveAllPointerTosDontKill(void) =0;
			void				SetExpectedDistToRoot(int iExpectedDistToRoot);
			void				SetCalculatedDistToRoot(void);
			int					CalculateDistToRootFromPointedFroms(void);
	virtual int					CalculateDistToRootFromPointedFroms(int iDistToRoot);
			BOOL				IsBaseObject(void);
			unsigned short int	GetNumEmbeddedFromFlags(void);
			void				SetFlagNumEmbedded(int iNumEmbedded);
			BOOL				IsMarkedUnreachable(void);
			void				ReplaceOneWithX(char* szDest, char* szMask);
			void				ContainerPreInit(void);
			void				ContainerPostInit(void);

	virtual void				BaseValidatePointerTos(void) = 0;
};


#endif // __BASE_OBJECT_H__

