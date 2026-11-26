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
#ifndef __EMBEDDED_OBJECT__
#define __EMBEDDED_OBJECT__
#include "BaseLib/Validation.h"
#include "BaseLib/IndexedGeneral.h"
#include "StackPointer.h"
#include "StackPointers.h"
#include "ObjectPointerArrays.h"
#include "Primitive.h"
#include "Unknown.h"
#include "PointerLocations.h"
#include "ObjectsLock.h"


#define ROOT_DIST_TO_ROOT			 0
#define UNATTACHED_DIST_TO_ROOT		-1
#define CLEARED_DIST_TO_ROOT		-2
#define EMBEDDED_DIST_TO_ROOT		-3  //An embedded object's own: its distance is its embedding container's.
#define HELD_FROM_STACK_DIST_TO_ROOT	-4  //An unattached object known to be held from the stack (see CBaseObject::IsKnownHeldFromStack).
#define MAX_DIST_TO_ROOT			 MAX_INT



enum EFreeResult
{
	FR_Error,
	FR_Freed,
	FR_NotFreed,
};


class CObject;
class CObjects;
class CObjectWriter;
class CObjectReader;
class CClass;
class CPointer;
class CEmbeddedObject;


struct SHeapFromPointer
{
	CBaseObject*		pcFrom;      //The Object containing the pointer.
	CEmbeddedObject*	pcTo;        //The Object pointed to.
	CPointer*			pcPointer;
};


typedef CArrayTemplate<SHeapFromPointer>	CArrayHeapFromPointer;


//An object with many heap froms (a hub, like a window's root component) keeps where each from is, so unlinking one
//isn't a search of them all, and a from that was closer to root, so asking whether there is one isn't either.
#define HEAP_FROM_HUB_MIN_FROMS		64

struct SHeapFromHub
{
	CPointerLocations	cLocations;		//Each from's index: an Object's index * 2, a Collection's index * 2 + 1.
	CBaseObject*		pcCloserFrom;	//A from that was closer to root than 'this' when last asked.  NULL if none is known.
	size				uiScanFrom;		//Where looking for a closer from starts: where the last one was found.
};


class CEmbeddedObject : public CUnknown
{
friend class CPointer;
friend class CCollection;
friend class CHollowObject;
CONSTRUCTABLE(CEmbeddedObject)
protected:
	CBaseObject*								mpcEmbedded;    //Object that 'this' is embedded in.
	CArrayTemplateEmbeddedPtr<CBaseObject, 4>		mapHeapFromObjects;		//Objects (not Collections) on the heap that 'this' is pointed to from.  This is a BaseObject not an EmbeddedObject because HollowEmbeddedObjects cannot point to anything.
																		//Heap froms are an in memory only concept with the exception that the closest from to the root is persistent.
	CArrayTemplateEmbeddedPtr<CBaseObject, 2>	mapHeapFromCollections;	//Collections on the heap that 'this' is pointed to from.  NumHeapFroms and GetHeapFrom see the Objects first, then the Collections.
	CStackPointer*								mpcStackFroms;  //Objects on the stack that 'this' is pointed to from.
	SHeapFromHub*								mpsHub;			//Once 'this' has HEAP_FROM_HUB_MIN_FROMS heap froms, NULL until then (and again below half that).

public:
								CEmbeddedObject();
								~CEmbeddedObject();

	static	int					CompareIndex(const void* ppvObject1, const void* ppvObject2);

			void				Kill(void) override;
	virtual void				KillInternal(bool bHeapFromChanged, bool bValidateNotEmbedded);

	virtual void				Evict(void);

	virtual void				FreeInternal(void) =0;
	virtual void				DoneKill(bool bAllocatedInObjects) =0;

	virtual void				FreePointers(void);
			void				PostRemapFroms(void);
	virtual bool				Save(CObjectWriter* pcFile) =0;
	virtual bool				Load(CObjectReader* pcFile) =0;
	virtual bool				IsHollow(void) const =0;
	virtual size				RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* mpcObject);
	virtual bool				SetDistToRoot(int iDistToRoot) =0;
	virtual int					GetDistToRoot(void) =0;
	virtual OIndex				GetIndex(void) const;
	virtual bool				IsNamed(void) const;
	virtual const char*			GetName(void) const;
	virtual void				FreeIdentifiers(void);
	virtual bool				IsDirty(void) =0;
	virtual void				SetDirty(bool bDirty) =0;
			void				LogicalDirty(void);
	virtual bool				IsBaseObject(void) const;
	virtual void				RemovePointerTo(CEmbeddedObject* pcTo);
			CBaseObject*		GetEmbeddingContainer(void);
			bool				IsEmbeddingContainerAllocatedInObjects(void);
			bool				IsEmbedded(void) const;
			bool				IsNotEmbedded(void) const;
	virtual bool				IsInitialised(void) =0;
	virtual bool				IsRoot(void) const =0;
	virtual size				GetEmbeddedIndex(CEmbeddedObject* pcEmbedded);
	virtual size				NumEmbedded(void) const =0;
			void				SetEmbedding(CBaseObject* pcEmbedded);
			CBaseObject*		GetEmbedding(void);  //The object this is embedded in directly (GetEmbeddingContainer is the outermost); NULL if not embedded.
	virtual CEmbeddedObject*	GetEmbeddedObject(size iIndex) =0;
	virtual CBaseObject*		Dehollow(void) =0;
			bool				IsAllocatedInObjects(void);
	virtual void				ValidateConsistency(void) =0;
			void				ValidateFrom(CBaseObject* pcBaseObject);
			void				ValidateFroms(void);
			void				ValidatePointerTo(CEmbeddedObject* pcPointedTo);
	virtual void				ValidatePointerTos(void);

	virtual size				NumPointerTos(void);
	virtual size				BaseNumPointerTos(void);
	virtual void				GetPointerTos(CArrayEmbeddedObjectPtr* papcTos);  //This object's pointer tos and those of the objects embedded in it.
	virtual void				BaseGetPointerTos(CArrayEmbeddedObjectPtr* papcTos);  //Only this object's own.  The one walk each kind of object writes.

			bool				HasHeapFroms(void);
			void				AddHeapFrom(CBaseObject* pcFromObject, bool bValidate);
			EFreeResult			RemoveHeapFromTryFree(CBaseObject* pcFromObject);  //Unlinks, then CObjects::FromRemoved.
	virtual size				NumHeapFroms(void);
			CBaseObject*		GetHeapFrom(size iFromIndex);
			void				AddHeapPointerForRemap(CBaseObject* pcFromObject);
			void				GetHeapFromPointers(CArrayHeapFromPointer* pacPointers);
	virtual void				GetPointerFieldsTo(CEmbeddedObject* pcTo, CArrayHeapFromPointer* pacPointers);

			bool				AddStackFrom(CPointer* pcPointer, bool bValidate);
			bool				AddStackFrom(CCollection* pcCollection, bool bValidate);
			bool				AddStackPointersForRemap(CStackPointer* pcStackPointer);
			bool				HasStackFroms(void);
			bool				UnlinkHeapFrom(CBaseObject* pcFrom);  //These only unlink; they never free.  See CObjects::FromRemoved.
			bool				HasOwnHeapFromCloserThan(int iDistToRoot);
			bool				IsBeingFreed(void);
			bool				UnlinkStackFrom(CPointer* pcPointer);
			bool				UnlinkStackFromOfThreads(CPointer* pcPointer);
			bool				IsAttachedToRoot(void);
			bool				IsThreadsOwn(void);
			CObjects*			GetWorkerObjects(void);
			CObjects*			GetWorkerObjectsLocking(void);
	static	bool				IsAcross(CObjects* pcWorker, CEmbeddedObject* pcTo);
			void				KeepWorkersOwnTos(CArrayEmbeddedObjectPtr* papcTos, size uiFirst);
			void				KeepWorkersOwnTosLocking(CArrayEmbeddedObjectPtr* papcTos, size uiFirst);
			void				ValidateWorkersOwn(CBaseObject* pcFromObject);
			void				AddStackFromReading(CPointer* pcPointer);
			bool				UnlinkStackFromReading(CPointer* pcPointer);
			bool				UnlinkStackFrom(CCollection* pcCollection);
			EFreeResult			RemoveStackFromTryFree(CPointer* pcPointer);
	virtual size				NumStackFroms(void);
			CStackPointer*		GetFirstStackFrom(void) const;

			size				NumTotalFroms(void);
			bool				ContainsFrom(CEmbeddedObject* pcBaseObject);
	virtual CClass*				GetClass(void) const =0;
	virtual bool				HasNes(void) const =0;
	virtual bool				HasNesPointerToThis(CEmbeddedObject* pcBaseObject) =0;
			CEmbeddedObject*	GetNesPointerFrom(size* piStart);

			CObjects*			GetObjects(void);
			CObjects*			GetObjectsOrDefault(void);
	virtual CObjects*			GetObjectsThisIn(void) =0;
			char*				PrintObject(CChars* psz);
			char*				PrintObject(CChars* psz, bool bEmbedded, bool bPointerMemory = true);
			bool				FailExpectedToBeInitialised(const char* szMethod);
			bool				FailExpectedToNotBeInitialised(const char* szMethod);
			bool				FailCantInitFreedObject(const char* szMethod);
			bool				FailNotExpectedToBeEmbedded(const char* szMethod);
			bool				FailExpectedToBeEmbedded(const char* szMethod);
			bool				TestRemoveHeapFrom(CBaseObject* pcFromObject);
			bool				TestRemoveStackFrom(CPointer* pcPointer);

			void				PrintHeapFroms(CChars* psz);
			void				DumpHeapFroms(void);
			void				PrintWeakFroms(CChars* psz);

public:
	virtual void				GetStackFroms(CArrayStackPointerE32* papcFroms);  //This object's stack froms and those of the objects embedded in it.
	virtual void				GetHeapFroms(CArrayBaseObjectPtrE32* papcFroms);  //This object's heap froms and those of the objects embedded in it.

protected:
			bool				LinkStackFrom(CStackPointer* pcStackPointer);
			bool				StackFromLinked(bool bAdded, bool bValidate);
	virtual void				Allocate(CObjects* pcObjects);
			void				AddHeapPointer(CBaseObject* pcFromObject);
	virtual void				RemoveAllHeapFroms(void);
	virtual void				RemoveAllStackFroms(void);
			CStackPointers*		GetStackPointers(void);
			void				ValidateObjectsConsistency(bool bValidate);

private:
	static	int					CompareIndexOtherwise(CEmbeddedObject* pc1, CEmbeddedObject* pc2);
			void				MakeHeapFromHub(void);
			void				KillHeapFromHub(void);
			bool				UnlinkHubHeapFrom(CBaseObject* pcFrom);
			void				ValidateHeapFromHub(void);
};


char* ObjectToString(CEmbeddedObject* pcObject);
char* ObjectToString(CEmbeddedObject* pcObject, bool bEmbedded);


//////////////////////////////////////////////////////////////////////////
//
// The worker's object system this is in, whose objects may point at main
// objects (ThreadSafetyPlan.txt, Level 3); NULL if it isn't in one, which
// with the locking off (one thread) is all that is asked.
//
//////////////////////////////////////////////////////////////////////////
inline CObjects* CEmbeddedObject::GetWorkerObjects(void)
{
#ifdef __LIBC65__
	return NULL;
#else
	return OBJECTS_SELDOM(gbObjectsLocking) ? GetWorkerObjectsLocking() : NULL;
#endif // __LIBC65__
}


//////////////////////////////////////////////////////////////////////////
//
// The objects BaseGetPointerTos added to papcTos (from uiFirst) less the
// main objects, when this is a worker's: its object system's walks don't
// go into main objects its objects point at (IsAcross).  With the locking
// off, nothing.
//
//////////////////////////////////////////////////////////////////////////
inline void CEmbeddedObject::KeepWorkersOwnTos(CArrayEmbeddedObjectPtr* papcTos, size uiFirst)
{
#ifndef __LIBC65__
	if (OBJECTS_SELDOM(gbObjectsLocking))
	{
		KeepWorkersOwnTosLocking(papcTos, uiFirst);
	}
#endif // !__LIBC65__
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcTo, which an object in pcWorker (GetWorkerObjects, or NULL)
// points at, is a main object: held as the worker's stack pointers hold
// one, not by a heap from, and not walked by the worker's object system.
//
//////////////////////////////////////////////////////////////////////////
inline bool CEmbeddedObject::IsAcross(CObjects* pcWorker, CEmbeddedObject* pcTo)
{
	return (pcWorker != NULL) && (pcTo != NULL) && (pcTo->GetObjectsThisIn() != pcWorker);
}


#endif // __EMBEDDED_OBJECT__

