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
#ifndef __OBJECTS_H__
#define __OBJECTS_H__
#include "BaseLib/DataConnection.h"
#include "BaseLib/IndexedGeneral.h"
#include "BaseLib/Logger.h"
#include "BaseLib/SequenceConnection.h"
#include "NamedIndexedObjects.h"
#include "Unknowns.h"
#include "Null.h"
#include "ObjectsSource.h"
#include "Root.h"
#include "Pointer.h"
#include "Classes.h"
#include "FreeObjects.h"
#include "DistCalculator.h"
#include "ObjectsRemovalContext.h"
#include "WeakFroms.h"
#include "ObjectsLock.h"


#define ROOT_NAME					"GraphRoot"
#define FREE_OBJECTS_NAME			"GraphFreeObjects"


Ptr<CRoot> ORoot(void);

//The arguments go to Init as they are, not copied: Init's own parameters say what is copied.
template <class SpecificClass, typename ... Args>
Ptr<SpecificClass> OMalloc(Args&& ... args);

template <class SpecificClass, typename ... Args>
Ptr<SpecificClass> ONMalloc(const char* szObjectName, Args&& ... args);


#define  LOG_OBJECT_ALLOCATION(pcObject) LogObjectAllocation(pcObject, __ENGINE_PRETTY_FUNCTION__)
#define  LOG_OBJECT_ALLOCATION_FAILURE(Class, Index, Name) LogObjectAllocationFailure(Class, Index, Name, __ENGINE_PRETTY_FUNCTION__)
#define  LOG_OBJECT_DESTRUCTION(pcObject) LogObjectDestruction(pcObject, __ENGINE_PRETTY_FUNCTION__)

typedef void (*ObjectCallback)(CBaseObject*);


class CHollowObject;
class CFreeObjects;


//A hollow loaded while another thread may have read a pointer to it and not yet asked it for its object, and what it
//was loaded as (ThreadSafetyPlan.txt, Forwarding hollows).
//A from of an object a worker adopts (CObjects::Adopt) that is taken off while the object moves, and put back as a stack
//from once it is main's: a stack pointer or collection of the worker's, or a field or collection of a worker's object
//outside the graph (which then holds a main object, IsAcross).
struct SAdoptFrom
{
	CEmbeddedObject*	pcTo;
	CPointer*			pcPointer;
	CCollection*		pcCollection;
};


typedef CArrayTemplate<SAdoptFrom>	CArrayAdoptFrom;


struct SForwardingHollow
{
	CHollowObject*	pcHollow;
	CBaseObject*	pcObject;
};
typedef CArrayTemplate<SForwardingHollow>	CArrayForwardingHollow;


//Design page: Docs/libraries/standardlib/objects.md
//Design page: Docs/libraries/standardlib/threads.md
class CObjects
{
friend class CBaseObject;
friend class CInternalObjectDeserialiser;
friend class CRoot;
protected:
	bool					mbInitialised;
	CUnknowns*				mpcUnknownsAllocatingFrom;

	CNamedIndexedObjects	mcMemory;			//Objects (BaseObject*) allocated in Unknowns referenced by name and OIndex.
	CStackPointers*			mpcStackPointers;	//The pointers on the stack to this's objects.
	CObjectsSource			mcSource;			//An object found on disk will be deserialised and then placed in memory.
	CClasses				mcClasses;

	CDataConnection*		mpcDataConnection;
	CSequenceConnection*	mpcSequenceConnection;
	CSpinLock				mcIndexLock;  //Over the sequence while the locking is on (ReserveIndexes), not the object system's lock.

	ObjectCallback			mAllocationCallback;
	ObjectCallback			mHollowAllocationCallback;
	ObjectCallback			mDestructionCallback;

	bool					mbValidateConsistency;
	CObjectRemovalContext	mcRemovalContext;
	CWeakFroms				mcWeakFroms;  //Every weakly held object's weak references (WeakReferencesPlan.txt).
	CArrayBaseObjectPtr		mapcWaitingToBeFreed;  //Objects other threads found dead, for the home thread to free (Collect).
	CArrayForwardingHollow	masForwarders;  //Hollows loaded while other threads were registered, until none can still ask one (FreeForwarders).
	size					muiForwardersMarked;  //The first of masForwarders, forwarding when the threads' ways in were last marked.
	uint32					muiWalkStamp;  //The base of the last walk started.  See WALK_STAMP_STEP.
	bool					mbWorker;  //A worker thread's own object system (ObjectsWorkerInit): that thread is its home.

public:
												CObjects();
						void					Init(CUnknowns* pcUnknownsAllocatingFrom, CStackPointers* pcStackPointers, CDataConnection* pcDataConnection, CSequenceConnection* pcSequenceConnection);
						void					InitWorker(CObjects* pcMain, CUnknowns* pcUnknowns, CStackPointers* pcStackPointers, CSequenceConnection* pcSequenceConnection);
						void					Kill(void);

	template<class M>	void					AddConstructor(void);
	template<class M>	CObjectSource*			AddSource(CAbstractFile* pcFile, const char* szFilename);

						bool					Flush(void);
						void					Discard(void);
						bool					Destroy(CBaseObject* pcObject);
						bool					Destroy(CArrayOIndex* paoiDestroyed);
						bool					LoadForward(CBaseObject* pcStart, CArrayOIndex* paoiLoaded, size uiMaxObjects);
						bool					GetStoredHeapFroms(CBaseObject* pcObject, CArrayOIndex* paoiHolders);  //The holders its record in the database was saved with.
						bool					ValidateNoDirty(void);
						bool					ForceSave(CBaseObject* pcObject);

						CPointer				Get(OIndex oi);
						CPointer				Get(const char* szObjectName);
						CPointer				GetPossiblyHollow(OIndex oi);
						CPointer				GetPossiblyHollow(const char* szObjectName);
	template<class M> 	Ptr<M>					Get(OIndex oi);
	template<class M>	Ptr<M>					Get(const char* szObjectName);
						bool					Contains(const char* szObjectName);
						bool					Contains(OIndex oi);

	template<class M> 	Ptr<M>					Malloc(void);
	template<class M>	Ptr<M>					Malloc(const char* szObjectName);

						Ptr<CRoot>				Root(void);
						void					LogicalDirty(CBaseObject* pcObject);

						void					Collect(void);  //The home thread's: frees what other threads found dead.
						void					FreeForwarders(void);  //The home thread's: frees the forwarding hollows no thread can still ask.
						bool					IsHomeThread(void);  //The running thread is the one that frees this's objects.
						bool					IsWorker(void);
						OIndex					ReserveIndexes(size uiCount);  //uiCount indexes in a row, the first: a worker's block, without waiting for writers.
						bool					Adopt(CPointer& pTop);  //The main object system's, by a worker: pTop's graph becomes main's (ObjectsAdopt).
						bool					IsWriteThrough(void);
						size					NumWaitingToBeFreed(void);
						size					NumForwarders(void);

						bool					Remove(CArrayBaseObjectPtr* papcKilled);
						bool					Remove(CArrayBaseObjectPtr* papcKilled, CArrayBaseObjectPtr* papcKilledAfter);
						EFreeResult				FromRemoved(CEmbeddedObject* pcTo, bool bHeapFrom);
						void					RemoveInKillOrder(CArrayBaseObjectPtr* papcKilled);

						CPointer				Null(void);
	template<class M>	Ptr<M>					Null(void);

						size					NumMemoryIndexes(void) const;
						size					NumMemoryNames(void) const;
						int64					NumDatabaseIndexes(void) const;
						int64					NumDatabaseNames(void) const;

						CSequenceConnection*	GetIndexGenerator(void) const;
						CNamedIndexedObjects*	GetMemory(void);
						bool					EvictInMemory(void);

						CBaseObject*			Dehollow(OIndex oi);
						CBaseObject*			Dehollow(const char* szObjectName);
						CBaseObject*			Dehollow(CHollowObject* pcHollow);

						bool					ReplaceBaseObject(CBaseObject* pcExisting, CBaseObject* pcObject);
						bool					TestRemoveMemoryIdentifiers(CBaseObject* pvObject);

						uint32					StartWalk(void);
						void					TestSetWalkStamp(uint32 uiWalkStamp);

						CStackPointers*			GetStackPointers(void) const;
						CClasses*				GetClasses(void);
						CConstructors*			GetConstructors(void) const;
						CUnknowns*				GetUnknowns(void) const;

						OIndex					StartMemoryIteration(SIndexesIterator* psIter);
						OIndex					IterateMemory(SIndexesIterator* psIter);
						CPointer				TestGetFromMemory(OIndex oi);
						CPointer				TestGetFromMemory(const char* szName);

						Ptr<CFreeObjects>		FreeObjects(void);
	template<class M>	M*						AddFreeObject(Ptr<M>& p);
						CBaseObject*			AddFreeObject(CPointer &p);
						bool					RemoveFreeObject(CBaseObject* pcObject);
						size					NumFreeObjects(void);

						void					PrintMemoryNames(CChars* psz);
						void					PrintMemoryUseIteration(CChars* psz);
						void					PrintMemoryPages(CChars* psz);
						void					PrintStackPointers(CChars* psz, bool bPointerMemory = true);
						void					PrintGraph(CChars* psz, bool bPointerMemory = true);
						void					DumpMemoryUseIteration(void);
						void					DumpMemoryPages(void);
						void					DumpGraph(void);
						void					DumpMemoryNames(void);
						void					DumpStackPointers(void);

						void					ValidateEmpty(void);
						void					ValidateObjectsConsistency(void);

						void					SetAllocationCallback(ObjectCallback fCallback);
						void					SetHollowAllocationCallback(ObjectCallback fCallback);
						void					SetDestructionCallback(ObjectCallback fCallback);

						void					EnableValidation(void);
						void					DisableValidation(void);

protected:
						Ptr<CRoot>				GetRoot(void);
						bool					HasRoot(void);

						bool					AddIntoMemory(CBaseObject* pcObject);

	template<class M> 	Ptr<M>					PointTo(M* pcObject);
	template<class M> 	Ptr<M>					PointToSetDirty(M* pcObject);
	template<class M>	M*						AllocateUninitialisedByTemplate(const char* szObjectName);
	template<class M>	M*						AllocateUninitialisedByTemplate(const char* szObjectName, OIndex oi, size uiAdditionalBytes);
						CBaseObject*			AllocateUninitialisedByClassName(const char* szClassName, const char* szObjectName, OIndex oi);
						bool					ValidateCanAllocate(const char* szClassName);
						bool					ValidateCanAllocate(void);

						CBaseObject*			GetFromMemory(OIndex oi);
						CBaseObject*			GetFromMemory(const char* szObjectName);
						CBaseObject*			GetFromDatabase(OIndex oi);
						CBaseObject*			GetFromDatabase(const char* szObjectName);
						CBaseObject*			GetFromSources(const char* szObjectName);
						OIndex					GetNextIndex(void);
						OIndex					NextIndex(void);

						void					FreeInternal(CArrayBaseObjectPtr* papcObjectPts);
						void					RemoveAllPointerTos(CArrayBaseObjectPtr* papcObjectPts);
						void					RemoveAllHeapFroms(CArrayBaseObjectPtr* papcObjectPts);
						void					DoneKill(CArrayBaseObjectPtr* papcObjectPts);
						void					ClearWeakFroms(CArrayBaseObjectPtr* papcObjectPts);
						void					KillObjects(CArrayBaseObjectPtr* papcObjectPts);
						void					RemoveBatch(CArrayBaseObjectPtr* papcObjectPts);
						void					SaveStoredDirty(CArrayBaseObjectPtr* papcObjectPts);
						void					WaitToBeFreed(CArrayBaseObjectPtr* papcObjectPts);
						void					ClearWalkStamps(void);
						bool					ValidateNoOtherThreadHolds(const char* szMethod);
						uint32					AdoptGraph(CObjects* pcWorker, CBaseObject* pcTop, CArrayBaseObjectPtr* papcGraph);
						bool					CanAdoptNames(CArrayBaseObjectPtr* papcGraph);
						void					TakeAdoptedFroms(CBaseObject* pcObject, uint32 uiStamp, CArrayAdoptFrom* pasFroms);
						void					RecordAdoptedFields(CEmbeddedObject* pcEmbedded, CArrayBaseObjectPtr* papcOutside, CArrayAdoptFrom* pasFroms, CArrayHeapFromPointer* pasFields);
						void					AdoptWeakFroms(CObjects* pcWorker, CArrayBaseObjectPtr* papcGraph, uint32 uiStamp);
						void					MoveAdopted(CObjects* pcWorker, CBaseObject* pcObject);
						void					HoldAdoptedTos(CBaseObject* pcObject, uint32 uiStamp);
						bool					ValidateNoOtherThreadHolds(const char* szMethod, CBaseObject* pcObject);
						bool					LoadForDestroy(OIndex oi);
						bool					SaveDirtyInMemory(void);
						void					FreeAllInKillOrder(bool bDeallocate);
						bool					ValidateNoEmbedded(CArrayBaseObjectPtr* papcObjectPts);

						void					RecurseDumpGraph(CChars* psz, CEmbeddedObject* pcObject, int iLevel, bool bEmbedded, bool bPointerMemory);
						void					GetObjectsInMemory(CArrayBaseObjectPtr* papcObjects);
						void					ValidateSceneGraph(void);
						void					ValidateSceneGraphObject(CBaseObject* pcBaseObject, CArrayBaseObjectPtr* papcStack);
						void					ValidateIndexedObjects(CArrayBaseObjectPtr* papcObjects);
						void					ClearValidationFlags(CArrayBaseObjectPtr* papcObjects);
						void					ValidateObjectInternals(CArrayBaseObjectPtr* papcObjects);
						void					GetStackPointerAddresses(CArrayVoidPtr* papvAddresses);
						CHollowObject*			AllocateHollow(size uiNumEmbedded, const char* szObjectName, OIndex oi);
						void					AppendHollowEmbeddedObjects(CBaseObject* pcHollow, size uiNumEmbedded, void* pvEmbedded) ;
						void					UnsetDumpFlags(void);
						bool					MustForward(void);
						void					Forward(CHollowObject* pcHollow, CBaseObject* pcObject);
						CBaseObject*			GetForwarded(CHollowObject* pcHollow);
						void					FreeForwardersInside(bool bAll);

public:
						CBaseObject*			AllocateUninitialisedByClassNameAndAddIntoMemory(const char* szClassName, const char* szObjectName);

						CHollowObject*			AllocateInternalHollow(const char* szObjectName, OIndex oi, size uiNumEmbedded);

						CBaseObject*			AllocateReplacingNamed(const char* szClassName, const char* szObjectName);
						CBaseObject*			GetNamedOrAllocateHollow(const char* szObjectName, size uiNumEmbedded);
						void					ReplaceWithHollow(CBaseObject* pcObject);
						CEmbeddedObject*		Forwarded(CEmbeddedObject* pcObject);

						void					AddWeakFrom(CEmbeddedObject* pcObject, CWeakHolder* pcHolder, size uiSlot);
						bool					RemoveWeakFrom(CEmbeddedObject* pcObject, CWeakHolder* pcHolder, size uiSlot);
						void					ClearWeakFroms(CBaseObject* pcObject);  //Each weak reference to it (and to those embedded in it) NULLed.
						void					MoveWeakFroms(CEmbeddedObject* pcFrom, CEmbeddedObject* pcTo);  //pcFrom's weak references point at pcTo.
						CWeakFroms*				GetWeakFroms(void);

protected:
						CBaseObject*			AllocateForInternalDeserialisation(const char* szClassName, const char* szObjectName, OIndex oi);
};


extern CObjects	gcObjects;
extern bool		gbObjects;
extern size		guiObjectsForwarders;  //Forwarding hollows in every object system (with the lock): pointing a pointer looks for one only while there are some.


//The object system OMalloc, ORoot and the rest make and find objects in: the running thread's own (a worker's), or
//gcObjects (ThreadSafetyPlan.txt, Level 3 as built).
#ifdef __LIBC65__
inline CObjects* ObjectsOwn(void) { return NULL; }
inline CObjects* ObjectsCurrent(void) { return &gcObjects; }
#else
extern OBJECTS_PER_THREAD CObjects*	gpcObjectsOwn;  //The running thread's own object system, NULL if it has none.
inline CObjects* ObjectsOwn(void) { return gpcObjectsOwn; }
inline CObjects* ObjectsCurrent(void)
{
	CObjects*	pcOwn;

	pcOwn = gpcObjectsOwn;
	return (pcOwn != NULL) ? pcOwn : &gcObjects;
}
#endif // __LIBC65__

void ObjectsInit(void);
void ObjectsInit(CDataConnection* pcDataConnection, CSequenceConnection* pcSequenceConnection);
void ObjectsInit(CUnknowns* pcUnknowns, CDataConnection* pcDataConnection, CSequenceConnection* pcSequenceConnection);
void ObjectsKill(bool bValidate = true);
bool ObjectsAdopt(CPointer& pTop);  //A worker's: pTop and every object of its it reaches become the main object system's (ThreadSafetyPlan.txt, Level 3).
bool ObjectsValidate(void);
bool ObjectsFlush(void);
void ObjectsCollect(void);  //The home thread's, from its tick: frees the objects other threads found dead.
void ObjectsDiscard(void);  //Marks every object in memory clean, throwing its changes away.  ObjectsFlush or ObjectsDiscard comes before ObjectsKill.
void ObjectsDump(void);

void LogObjectAllocation(CBaseObject* pcObject, const char* szMethod);
void LogObjectDestruction(CBaseObject* pcObject, const char* szMethod);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
SpecificClass* CObjects::AllocateUninitialisedByTemplate(const char* szObjectName, OIndex oi, size uiAdditionalBytes)
{
	SpecificClass*	pcObject;
	bool			bResult;

	bResult = ValidateCanAllocate();
	if (!bResult)
	{
		return NULL;
	}

	pcObject = mpcUnknownsAllocatingFrom->Add<SpecificClass>(uiAdditionalBytes);
	if (pcObject)
	{
		pcObject->Allocate(this);
		pcObject->InitIdentifiers(szObjectName, oi);
	}
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
SpecificClass* CObjects::AllocateUninitialisedByTemplate(const char* szObjectName)
{
	OIndex	oi;

	oi = GetNextIndex();
	return AllocateUninitialisedByTemplate<SpecificClass>(szObjectName, oi, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::Null(void)
{
	Ptr<SpecificClass> pObject;
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::PointTo(SpecificClass* pcObject)
{
	Ptr<SpecificClass>	pObject;

	if (pcObject)
	{
		pObject.AssignObject(pcObject);
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::PointToSetDirty(SpecificClass* pcObject)
{
	Ptr<SpecificClass>	pObject;

	if (pcObject)
	{
		pObject.AssignObject(pcObject);
		pcObject->SetDirty(true);
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::Malloc(void)
{
	return Malloc<SpecificClass>("");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::Malloc(const char* szObjectName)
{
	OBJECTS_WRITE();

	bool			bResult;
	SpecificClass*	pcObject = AllocateUninitialisedByTemplate<SpecificClass>(szObjectName);
	if (pcObject)
	{
		bResult = AddIntoMemory(pcObject);
		if (bResult)
		{
			return PointToSetDirty(pcObject);
		}
		else
		{
			pcObject->KillInternal(false, true);
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::Get(OIndex oi)
{
	return Get(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
Ptr<SpecificClass> CObjects::Get(const char* szObjectName)
{
	return Get(szObjectName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
void CObjects::AddConstructor(void)
{
	if (mpcUnknownsAllocatingFrom == NULL)
	{
		gcLogger.Error2(__METHOD__, " UnknownsAllocatingFrom is NULL.  Probably Objects has not been initialised.", NULL);
		return;
	}

	if (!mpcUnknownsAllocatingFrom->GetConstructor<SpecificClass>())
	{
		mpcUnknownsAllocatingFrom->AddConstructor<SpecificClass>();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SpecificClass>
CObjectSource* CObjects::AddSource(CAbstractFile* pcFile, const char* szFilename)
{
	return mcSource.AddSource<SpecificClass>(pcFile, szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CObjects::AddFreeObject(Ptr<M>& p)
{
	return (M*)AddFreeObject((CPointer&)p);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class SpecificClass, typename ... Args>
Ptr<SpecificClass> OMalloc(Args&& ... args)
{
	//Calls .Init on the allocated instance to ensure it's properly wound into the Objects machinery.
	Ptr<SpecificClass> pObject = ObjectsCurrent()->Malloc<SpecificClass>();
	if (pObject.IsNotNull())
	{
		pObject->Init(args...);
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class SpecificClass, typename ... Args>
Ptr<SpecificClass> ONMalloc(const char* szObjectName, Args&& ... args)
{
	//Calls .Init on the allocated instance to ensure it's properly wound into the Objects machinery.
	Ptr<SpecificClass> pObject = ObjectsCurrent()->Malloc<SpecificClass>(szObjectName);
	if (pObject.IsNotNull())
	{
		pObject->Init(args...);
	}
	return pObject;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class SpecificClass>
Ptr<SpecificClass> OMallocNoI()
{
	//Does not call .Init on the allocated instance.
	return ObjectsCurrent()->Malloc<SpecificClass>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class SpecificClass>
Ptr<SpecificClass> ONMallocNoI(const char* szObjectName)
{
	//Does not call .Init on the allocated instance.
	return ObjectsCurrent()->Malloc<SpecificClass>(szObjectName);
}


#endif // __OBJECTS_H__

