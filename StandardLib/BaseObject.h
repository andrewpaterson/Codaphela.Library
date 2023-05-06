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
#include "BaseLib/IndexedGeneral.h"
#include "EmbeddedObject.h"
#include "Class.h"


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
#define OBJECT_FLAGS_CALLED_KILL				 0x001

#define OBJECT_FLAGS_CLEARED_DIST_TO_ROOT		0x1000
#define OBJECT_FLAGS_UPDATED_TOS_DIST_TO_ROOT	0x2000
#define OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED	0x4000
#define OBJECT_FLAGS_DIST_FROM_WALKED			0x8000

//All flags are used!  You'll need to make miFlags bigger if you need more.

#define MAX_NAMED_OBJECT_NAME_LENGTH	4096

class CObjectReader;
class CObjectWriter;
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
friend class CObjectWriter;
friend class CObjectReader;
friend class CNamedIndexedObjects;

CONSTRUCTABLE(CBaseObject);
protected:
	OIndex		moi;
	CChars		mon;
	CObjects*	mpcObjectsThisIn;
	CClass*		mpcClass;
	int32		miDistToRoot;
	int32		miDistToStack;
	uint16		muiFlags;
	uint16		muiNumEmbedded;
	uint16		muiPreInits;
	uint16		muiPostInits;

public:
								CBaseObject();
								~CBaseObject();

protected:
	virtual	void				Class(void) =0;

public:
			bool				InitIdentifiers(const char* szName, OIndex oi);
			void				Kill(void) override;
	virtual void				Free(void) =0;
	virtual bool				Flush(void);

			bool				Save(CObjectWriter* pcFile) override;
			bool				Load(CObjectReader* pcFile) override;

			OIndex				GetIndex(void) override;
			void				ClearIdentifiers(void) override;

			bool				IsRoot(void);
	virtual bool				IsSubRoot(void);
			bool				IsHollow(void);
	virtual bool				IsCollection(void) =0;
	virtual bool				IsObject(void) =0;
			bool				IsNamed(void) override;
			bool				IsInvalidated(void);
			bool				IsDirty(void);
			bool				IsUpdateAttachedPointerTosDistToRoot(void);
			bool				IsInitialised(void);
			bool				HasClass(void);
			CClass*				GetClass(void);
			bool				IsKilled(void);
			bool				IsNamed(const char* szName);

			char*				GetName(void) override;
			int					SerialisedSize(void);
	virtual char*				GetIdentifier(CChars* psz);

			uint16				GetNumEmbedded(void);

	virtual void				SetPointerTosExpectedDistToRoot(int iDistToRoot) =0;
			void				SetDirty(bool bDirty);
			int					GetDistToRoot(void);
			int					GetDistToStack(void);
	virtual bool				SetDistToRoot(int iDistToRoot) override;
			bool				TestedForRoot(void);
			void				UpdateAttachedTosDistToRoot(CDistCalculatorParameters* pcParameters);
			void				CollectValidDistStartingObjectsAndSetClearedToRoot(CBaseObject* pcTo, CDistCalculatorParameters* pcParameters);
			void				CollectAndClearInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters);
	virtual bool				IsDistToRootValid(void);
			int					CollectDetachedAndSetDistToStackZero(CDistCalculatorParameters* pcParameters);
			int					CollectDetachedFroms(CDistCalculatorParameters* pcParameters);

			void				AddExpectedDistToRoot(CEmbeddedObject* pcPointedTo, int iExpectedDist, CDistCalculatorParameters* pcParameters);
			void				ClearDistTouchedFlags(void);
			bool				HasDistTouchedFlag(void);

			bool				TestedForSanity(void);
			CObjects*			GetObjectsThisIn(void);
			CClasses*			GetClasses(void);
			CStackPointers*		GetStackPointers(void);
	virtual void				SetDistToStack(int iDistToStack);
			CClass*				Class(CClasses* pcClasses);

	virtual bool				ContainsPointerTo(CEmbeddedObject* pcEmbedded);
			CEmbeddedObject* 	TestGetPointerTo(int iToIndex);
			int 				TestGetNumEmbeddedFromFlags(void);
			void				ClearFlagNumEmbedded(void);
	virtual void				SetFlag(int iFlag, int iFlagValue);
			int					GetFlags(void);
			bool				CanFindRoot(void);
			bool				CanFindRootThroughValidPath(void);

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
			void				ValidateKillCalled(void);
	
protected:
	virtual void				Allocate(CObjects* pcObjects);
			void				PreClass(void);
			void				SetClass(CClass* pcClass);

			void				PreInit(void);
			void				PostInit(void);
	virtual void				Initialised(void);
	virtual	void				EmbedFields(void) =0;

			void				FreeInternal(void) override;
			void				KillInternal(bool bHeapFromChanged) override;
			void				TryFree(bool bKillIfNoRoot, bool bHeapFromChanged);

	virtual void				RemoveAllPointerTosDontKill(void) =0;
	virtual void				RemoveAllPointerTos(void) =0;

			bool				SaveManaged(CObjectWriter* pcFile);
			bool				LoadManaged(CObjectReader* pcFile);

			bool				SaveHeapFroms(CObjectWriter* pcFile);
			bool				LoadHeapFroms(CObjectReader* pcFile);

	virtual void				FreeIdentifiers(void);
			void				FreePointers(void) override;
			int					RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew) =0;
			bool				RemoveToFrom(CEmbeddedObject* pcPointedTo);
			void				SetExpectedDistToRoot(int iExpectedDistToRoot);
			void				SetCalculatedDistToRoot(void);
			int					CalculateDistToRootFromPointedFroms(void);
	virtual int					CalculateDistToRootFromPointedFroms(int iDistToRoot);
			bool				IsBaseObject(void);
			uint16				GetNumEmbeddedFromFlags(void);
			void				SetFlagNumEmbedded(int iNumEmbedded);
			bool				IsMarkedUnreachable(void);
			void				ReplaceOneWithX(char* szDest, char* szMask);
			void				ContainerPreInit(void);
			void				ContainerPostInit(void);

			bool				SaveEmbeddedObjectsManaged(CObjectWriter* pcFile);
			bool				SavePointers(CObjectWriter* pcFile);
			bool				SavePrimitives(CObjectWriter* pcFile);
			bool				SaveUnmanaged(CObjectWriter* pcFile);
			bool				SaveEmbeddedObjectsHeapFroms(CObjectWriter* pcFile);
			bool				LoadEmbeddedObjectsManaged(CObjectReader* pcFile);
			bool				LoadPointers(CObjectReader* pcFile);
			bool				LoadPrimitives(CObjectReader* pcFile);
			bool				LoadUnmanaged(CObjectReader* pcFile);
			bool				LoadEmbeddedObjectsHeapFroms(CObjectReader* pcFile);

	virtual void				BaseValidatePointerTos(void) =0;

private:
			bool				ClipName(void);
};


#endif // !__BASE_OBJECT_H__

