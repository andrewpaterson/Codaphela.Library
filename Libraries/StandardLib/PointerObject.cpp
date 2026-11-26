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
#include "BaseLib/Atomic.h"
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "BaseLib/Numbers.h"
#include "Object.h"
#include "HollowObject.h"
#include "ObjectReader.h"
#include "ObjectRemapFrom.h"
#include "PointerObject.h"
#include "ObjectsLock.h"
#include "ObjectsThreads.h"


bool gbLogPointer = false;

//What a pointer that is let go of is pointed at.
static CEmbeddedObject* const	gpcPointerNoObject = NULL;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LogPointerDebug(CPointer* pvThis, const char* szMethod)
{
	CBaseObject*	pcPointedTo;
	CObject*		pcEmbedding;
	CChars			sz;
	size			uiLength;
	size			uiOriginalLength;

	if (gbLogPointer)
	{
		pcEmbedding = pvThis->Embedding();
		sz.Init();
		sz.Append(szMethod);
		uiOriginalLength = sz.Length();
		sz.RemoveEnd(62);
		uiLength = sz.Length();
		sz.Append(": ");
		if (uiLength < 62)
		{
			sz.Append(' ', 62 - uiLength);
		}
		else if (uiOriginalLength >= 62)
		{
			sz.SetChar(59, '.');
			sz.SetChar(60, '.');
			sz.SetChar(61, '.');
		}
		sz.AppendPointer(pvThis);
		sz.Append(' ');
		if (pcEmbedding != NULL)
		{
			sz.Append("Embedding {");
			pcEmbedding->PrintState(&sz);
			sz.Append("}.");
		}
		sz.Append("Object {");
		pcPointedTo = pvThis->BaseObject();
		if (pcPointedTo)
		{
			pcPointedTo->PrintState(&sz);
		}
		else
		{
			sz.Append("NULL");
		}
		sz.Append("}");
		gcLogger.Debug(sz.Text());
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer::CPointer()
{
	mpcObject = NULL;
	mpcEmbedding = NULL;

	LOG_POINTER_DEBUG();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer::CPointer(CPointer& cPointer)
{
	//Constructor only called if a Pointer is declared on the Stack.
	mpcEmbedding = NULL;
	mpcObject = NULL;

	PointToFromOutside(&cPointer.mpcObject);

	LOG_POINTER_DEBUG();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer::CPointer(const CPointer& cPointer)
{
	//Constructor only called if a Pointer is declared on the Stack.
	mpcEmbedding = NULL;
	mpcObject = NULL;

	PointToFromOutside(&cPointer.mpcObject);

	LOG_POINTER_DEBUG();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer::CPointer(CEmbeddedObject* pcObject)
{
	//Constructor only called if a Pointer is declared on the Stack.
	mpcEmbedding = NULL;
	mpcObject = NULL;

	PointToFromOutside(&pcObject);

	LOG_POINTER_DEBUG();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer::~CPointer()
{
	CEmbeddedObject*	pcObject;

	LOG_POINTER_DEBUG();

	//Read whole: a load on another thread may be rewriting it.  With the locking on it is read again inside, where
	//nothing can be.
	pcObject = LoadObject();
	if (pcObject != NULL)
	{
		if (!OBJECTS_LOCKING())
		{
			pcObject->RemoveStackFromTryFree(this);
		}
		else
		{
			PointToLocking(&gpcPointerNoObject);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::operator = (CEmbeddedObject* pcObject)
{
	PointToFromOutside(&pcObject);

	LOG_POINTER_DEBUG();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::operator = (CPointer& pPointer)
{
	PointToFromOutside(&pPointer.mpcObject);

	LOG_POINTER_DEBUG();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::operator = (const CPointer& pPointer)
{
	PointToFromOutside(&pPointer.mpcObject);

	LOG_POINTER_DEBUG();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CPointer::operator -> ()
{
	return DereferenceArrow();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CPointer::operator & ()
{
	return Dereference();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::operator ! ()
{
	return LoadObject() == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::operator == (const CPointer& pPointer)
{
	return LoadObject() == pPointer.Object();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::operator != (const CPointer& pPointer)
{
	return LoadObject() != pPointer.Object();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::operator == (void* pcPointer)
{
	return LoadObject() == pcPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::operator != (void* pcPointer)
{
	return LoadObject() != pcPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::UnsafeClearObject(void)
{
	OBJECTS_INSIDE();

	mpcObject = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointer::NumStackFroms(void) const
{
	OBJECTS_READ();

	if (mpcObject)
	{
		return mpcObject->NumStackFroms();
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointer::NumHeapFroms(void) const
{
	OBJECTS_READ();

	if (mpcObject)
	{
		return mpcObject->NumHeapFroms();
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointer::NumEmbedded(void) const
{
	OBJECTS_READ();

	if (mpcObject)
	{
		return mpcObject->NumEmbedded();
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointer::NumPointerTos(void) const
{
	OBJECTS_READ();

	if (mpcObject)
	{
		return mpcObject->NumPointerTos();
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::SetEmbedding(CObject* pcEmbedding)
{
	mpcEmbedding = pcEmbedding;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::UnsafePointTo(CEmbeddedObject* pcNewObject)
{
	OBJECTS_INSIDE();

	//Whole: a load remaps pointers that other threads may be reading (Dereference).
	AtomicStorePointer((void**)&mpcObject, pcNewObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::PointTo(CEmbeddedObject* pcNewObject)
{
	PointToFromOutside(&pcNewObject);
}


//////////////////////////////////////////////////////////////////////////
//
// pcObject, or if it is a hollow that was read before it was loaded on
// another thread (a collection's element, read and then pointed at) and
// forwards now, what it was loaded as: a pointer is never pointed at a
// forwarder.  Only a way in from outside that has taken the lock asks: a
// forwarder exists only while the locking is on, isn't attached to the
// root (so a reader never points at one), and the object system's own
// callers read what they point at under the lock, after any load has
// pointed it at the object.
//
//////////////////////////////////////////////////////////////////////////
static NO_INLINE CEmbeddedObject* PointerForwarded(CEmbeddedObject* pcObject)
{
	return (pcObject != NULL) ? pcObject->GetObjectsOrDefault()->Forwarded(pcObject) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static inline CEmbeddedObject* NotForwarding(CEmbeddedObject* pcObject)
{
	return OBJECTS_SELDOM(guiObjectsForwarders != 0) ? PointerForwarded(pcObject) : pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
// Points this at *ppcObject, from outside the object system: what the constructors and the assignments do.
// *ppcObject (another pointer's object, where this is to point at what that does) is read once nothing can be
// changing it.
//
// With the locking on it is first tried as a reader, without the lock (PointToLocking).
//
//////////////////////////////////////////////////////////////////////////
void CPointer::PointToFromOutside(CEmbeddedObject* const* ppcObject)
{
	if (!OBJECTS_LOCKING())
	{
		PointToInside(*ppcObject);
	}
	else
	{
		PointToLocking(ppcObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// PointToFromOutside with the locking on: in the running thread's own
// object system (a worker's) as with the locking off; as a reader if it
// can be (PointToReading); otherwise with the lock.  Apart, so that the
// ways with the locking off (the constructors, assignments and destructor)
// stay as short as they are.
//
//////////////////////////////////////////////////////////////////////////
NO_INLINE void CPointer::PointToLocking(CEmbeddedObject* const* ppcObject)
{
	CEmbeddedObject*	pcNewObject;

	//Only a worker asks.  What it points this at is its own, or a main object held by what it is read from, read
	//whole: a load may be rewriting it.
	if (ObjectsOwn() != NULL)
	{
		pcNewObject = (CEmbeddedObject*)AtomicLoadPointer((void**)ppcObject);
		if (IsAllThreadsOwn(pcNewObject))
		{
			ObjectsOwnEnters();
			PointToInside(pcNewObject);
			ObjectsOwnLeaves();
			return;
		}
	}

	if (!PointToReading(ppcObject))
	{
		ObjectsLock();
		PointToInside(NotForwarding(*ppcObject));
		ObjectsUnlock();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether this, pointed from its object at pcNewObject, changes nothing
// but the running thread's own object system: the object it points at,
// pcNewObject and the object it is a field of are each its own (or NULL).
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsAllThreadsOwn(CEmbeddedObject* pcNewObject)
{
	CObjects*			pcOwn;
	CEmbeddedObject*	pcOldObject;

	pcOwn = ObjectsOwn();
	if ((pcOwn == NULL) || ((char*)this < CORRUPT_MEMORY))
	{
		return false;
	}
	pcOldObject = LoadObject();
	return ((pcOldObject == NULL) || (pcOldObject->GetObjectsThisIn() == pcOwn)) &&
		   ((pcNewObject == NULL) || (pcNewObject->GetObjectsThisIn() == pcOwn)) &&
		   ((mpcEmbedding == NULL) || (mpcEmbedding->GetObjectsThisIn() == pcOwn));
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer on the stack pointed from one object attached to the root (or none) at another (or none) changes two
// stack froms and nothing else: nothing can be freed by it and no distance changes.  That is most of what is ever
// done with pointers, and a thread that is reading may do it (level 2; ObjectsThreads.h).  False, with nothing
// done, if it isn't that or can't be read for now: it is then done with the lock.
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::PointToReading(CEmbeddedObject* const* ppcObject)
{
	CEmbeddedObject*	pcNewObject;
	CEmbeddedObject*	pcOldObject;
	bool				bDone;

	//A field of an object, or a pointer reached through a NULL one (PointToInside says how).
	if (((char*)this < CORRUPT_MEMORY) || (mpcEmbedding != NULL) || !ObjectsReadBegins())
	{
		return false;
	}

	pcNewObject = *ppcObject;
	pcOldObject = mpcObject;
	bDone = ((pcNewObject == NULL) || pcNewObject->IsAttachedToRoot()) && ((pcOldObject == NULL) || pcOldObject->IsAttachedToRoot());

	//The home thread with objects waiting for it to free takes the lock instead, which frees them.
	bDone = bDone && !(ObjectsReaderIsHome() && ObjectsAreFreesWaiting());
	if (bDone && (pcNewObject != pcOldObject))
	{
		//The old from goes first: taking it off is what can fail, and with both objects held by the root the order
		//frees nothing.
		if (pcOldObject != NULL)
		{
			bDone = pcOldObject->UnlinkStackFromReading(this);
		}
		if (bDone)
		{
			mpcObject = pcNewObject;
			if (pcNewObject != NULL)
			{
				pcNewObject->AddStackFromReading(this);
			}
		}
	}
	ObjectsReadEnds(bDone);
	return bDone;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether this field, of a worker's object, was or is now pointed at a
// main object (ThreadSafetyPlan.txt, Level 3).  Never with the locking
// off.
//
//////////////////////////////////////////////////////////////////////////
inline bool CPointer::IsFieldAcross(CEmbeddedObject* pcOldObject)
{
	CObjects*	pcWorker;

	pcWorker = mpcEmbedding->GetWorkerObjects();
	return CEmbeddedObject::IsAcross(pcWorker, pcOldObject) || CEmbeddedObject::IsAcross(pcWorker, mpcObject);
}


//////////////////////////////////////////////////////////////////////////
//
// PointTo for what holds the object system's lock already: the constructors and assignments, which read what they
// are to point at under it.
//
//////////////////////////////////////////////////////////////////////////
void CPointer::PointToInside(CEmbeddedObject* pcNewObject)
{
	CEmbeddedObject*	pcOldObject;
	CPointer			cHoldEmbedding;

	//A Ptr field reached through a NULL Ptr (pNull->mpField = p) is near address 0.  DereferenceArrow has already
	//logged the NULL dereference, so the assignment is dropped rather than crashing.  See
	//TestObjectAllocatorAssignmentToNullObject.
	if ((char*)this < CORRUPT_MEMORY)
	{
		return;
	}

	if (mpcObject != pcNewObject)
	{
		pcOldObject = mpcObject;
		mpcObject = pcNewObject;

		if (IsEmbeddingAllocatedInObjects())
		{
			if (OBJECTS_SELDOM(IsFieldAcross(pcOldObject)))
			{
				PointFieldAcross(pcOldObject);
			}
			else if (pcOldObject)
			{
				//Removing the heap from on the old object can free the embedding (and this pointer) if nothing else holds the embedding.
				//Hold it until this method is done; it is freed when cHoldEmbedding is destroyed.
				if (!IsEmbeddingHeld())
				{
					cHoldEmbedding.PointToInside(mpcEmbedding);
				}

				if (mpcObject)
				{
					mpcObject->AddHeapFrom(mpcEmbedding, false);
					mpcObject->SetDirty(true);
				}
				pcOldObject->SetDirty(true);
				pcOldObject->RemoveHeapFromTryFree(mpcEmbedding);
			}
			else if (mpcObject)
			{
				mpcObject->AddHeapFrom(mpcEmbedding, true);
				mpcObject->SetDirty(true);
			}
			mpcEmbedding->SetDirty(true);
		}
		else
		{
			//Add the new stack from before removing the old one.  The new object may only be reachable through the old
			//object (p = p->mpNext) and would otherwise be freed with it.
			if (mpcObject)
			{
				mpcObject->AddStackFrom(this, false);
			}

			if (pcOldObject)
			{
				pcOldObject->RemoveStackFromTryFree(this);
			}
		}
	}

	if (mpcObject)
	{
		mpcObject->LogicalDirty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// PointToInside for a field of a worker's object pointed from pcOldObject
// at mpcObject, either of them a main object.  An object of the worker's
// own has a heap from of it, as any field's object does; a main object is
// held as the worker's stack pointers hold one (its from in the worker's
// table, or on it), under the main lock: the new before the old goes, and
// a main object let go of is the main object system's to free.
//
//////////////////////////////////////////////////////////////////////////
NO_INLINE void CPointer::PointFieldAcross(CEmbeddedObject* pcOldObject)
{
	CPointer	cHoldEmbedding;
	CObjects*	pcWorker;

	if ((pcOldObject != NULL) && !IsEmbeddingHeld())
	{
		cHoldEmbedding.PointToInside(mpcEmbedding);
	}

	pcWorker = mpcEmbedding->GetWorkerObjects();
	ObjectsLock();
	if (mpcObject != NULL)
	{
		if (CEmbeddedObject::IsAcross(pcWorker, mpcObject))
		{
			mpcObject->AddStackFrom(this, false);
		}
		else
		{
			mpcObject->AddHeapFrom(mpcEmbedding, pcOldObject == NULL);
			mpcObject->SetDirty(true);
		}
	}
	if (pcOldObject != NULL)
	{
		if (CEmbeddedObject::IsAcross(pcWorker, pcOldObject))
		{
			pcOldObject->RemoveStackFromTryFree(this);
		}
		else
		{
			pcOldObject->SetDirty(true);
			pcOldObject->RemoveHeapFromTryFree(mpcEmbedding);
		}
	}
	ObjectsUnlock();
}


//////////////////////////////////////////////////////////////////////////
//
// This field, of a worker's object that is being freed, lets go of the
// main object it points at (PointFieldAcross), and is NULL.
//
//////////////////////////////////////////////////////////////////////////
void CPointer::ReleaseAcross(void)
{
	CEmbeddedObject*	pcObject;

	//Read and cleared under the lock: a load of the main object rewrites it.
	ObjectsLock();
	pcObject = mpcObject;
	UnsafePointTo(NULL);
	pcObject->RemoveStackFromTryFree(this);
	ObjectsUnlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsEmbeddingHeld(void) const
{
	//An object attached to the root, or pointed to from the stack, cannot be freed by changing one of its own pointers.
	//Anything else (e.g. after Return()) is only alive because nothing has tried to free it yet.

	CBaseObject*	pcContainer;

	pcContainer = mpcEmbedding->GetEmbeddingContainer();
	if (pcContainer->GetDistToRoot() >= ROOT_DIST_TO_ROOT)
	{
		return true;
	}
	return pcContainer->HasStackFroms();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::PointToNull(void)
{
	PointTo(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CPointer::Return(void)
{
	OBJECTS_WRITE();

	CEmbeddedObject*	pOldObject;

	//Call this method when you need to return a pointers object but no other
	//pointers point to the object and this pointers destructor must not destroy the object.

	pOldObject = mpcObject;
	if (pOldObject)
	{
		pOldObject->UnlinkStackFrom(this);
		mpcObject = NULL;
	}
	return pOldObject;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsNotNull(void) const
{
	return LoadObject() != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsNull(void) const
{
	return LoadObject() == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsNotEqual(CPointer& pPointer)
{
	return LoadObject() != pPointer.Object();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsEqual(CPointer& pPointer)
{
	return LoadObject() == pPointer.Object();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsEqual(CEmbeddedObject* pcObject)
{
	return LoadObject() == pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer* CPointer::This(void)
{
	//The pointer's own address.  operator& gives the address of the object pointed to, so M_Pointer in a Class() and
	//anything else that needs the CPointer itself uses this.
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CPointer::Object(void) const
{
	return LoadObject();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject** CPointer::ObjectPtr(void)
{
	return &mpcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CPointer::BaseObject(void)
{
	CEmbeddedObject*	pcObject;

	pcObject = LoadObject();
	if (pcObject && pcObject->IsBaseObject())
	{
		return (CBaseObject*)pcObject;
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
CObject* CPointer::Embedding(void)
{
	return mpcEmbedding;
}


//////////////////////////////////////////////////////////////////////////
//
// The object, loaded if it is a hollow.  NULL for a NULL pointer, or a
// hollow that can't be loaded (the pointer is left pointing to it, to try
// again).  Nothing is written: a load points every pointer to the hollow
// at the object, this one too.  The pointer is read whole, as a load on
// another thread may be rewriting it (ThreadSafetyPlan.txt, Forwarding
// hollows).
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CPointer::Dereference(void)
{
	CEmbeddedObject*	pcObject;

	pcObject = LoadObject();
	if (pcObject == NULL)
	{
		return NULL;
	}
	return Dehollow(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
// Dereference for operator->: dereferencing a NULL pointer is an error.
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CPointer::DereferenceArrow(void)
{
	CEmbeddedObject*	pcObject;

	pcObject = LoadObject();
	if (pcObject == NULL)
	{
		gcLogger.Error2(__METHOD__, " Attempted to dereference NULL Pointer.", NULL);
		return NULL;
	}
	return Dehollow(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
// pcObject, what this pointed to when it was read, or if that was a
// hollow what it is loaded as (which this points to now).
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CPointer::Dehollow(CEmbeddedObject* pcObject)
{
	CEmbeddedObject*	pcLoaded;

	pcLoaded = pcObject->Dehollow();
#ifdef _DEBUG
	if ((pcLoaded != NULL) && (pcLoaded != pcObject) && (LoadObject() != pcLoaded))
	{
		gcLogger.Error2(__METHOD__, " A pointer ", (mpcEmbedding != NULL) ? "field" : "on the stack", " to hollow {", ObjectToString(pcObject), "} wasn't pointed at what the hollow was loaded as, {", ObjectToString(pcLoaded), "}: it points at {", ObjectToString(LoadObject()), "}.", NULL);
	}
#endif // _DEBUG
	return pcLoaded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointer::MorphInto(CEmbeddedObject* pcNew)
{
	OBJECTS_WRITE();

	CObjectRemapFrom	cRemapper;
	size				iCount;

	iCount = cRemapper.Remap(mpcObject, pcNew);

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsHollow(void) const
{
	CEmbeddedObject*	pcObject;

	pcObject = LoadObject();
	if (pcObject)
	{
		return pcObject->IsHollow();
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::Load(CObjectReader* pcFile)
{
	if (mpcObject)
	{
		if (!mpcObject->IsHollow())
		{
			return mpcObject->Load(pcFile);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPointer::GetDistToRoot(void) const
{
	OBJECTS_READ();

	return mpcObject ? mpcObject->GetDistToRoot() : UNATTACHED_DIST_TO_ROOT;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CPointer::GetIndex(void) const
{
	CEmbeddedObject*	pcObject;

	pcObject = LoadObject();
	if (pcObject)
	{
		return pcObject->GetIndex();
	}
	else
	{
		return NULL_O_INDEX;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CPointer::GetName(void) const
{
	CEmbeddedObject*	pcObject;

	pcObject = LoadObject();
	if (pcObject)
	{
		return pcObject->GetName();
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
bool CPointer::IsNamed(void) const
{
	CEmbeddedObject*	pcObject;

	pcObject = LoadObject();
	if (pcObject)
	{
		return pcObject->IsNamed();
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CPointer::ClassName(void)
{
	CEmbeddedObject*	pcObject;

	pcObject = LoadObject();
	if (pcObject)
	{
		return pcObject->ClassName();
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
CClass* CPointer::GetClass(void) const
{
	CEmbeddedObject*	pcObject;

	pcObject = LoadObject();
	if (pcObject)
	{
		return pcObject->GetClass();
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
bool CPointer::IsDirty(void) const
{
	OBJECTS_READ();

	if (mpcObject)
	{
		return mpcObject->IsDirty();
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::LogicalDirty(void)
{
	CEmbeddedObject*	pcObject;

	pcObject = LoadObject();
	if (pcObject)
	{
		pcObject->LogicalDirty();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointer::IsEmbeddingAllocatedInObjects(void) const
{
	if (mpcEmbedding)
	{
		return mpcEmbedding->IsAllocatedInObjects();
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::Kill(void)
{
	CEmbeddedObject*	pcObject;

	//This method exists so that its object can be killed without invoking -> and potentially loading it first.
	pcObject = LoadObject();
	if (pcObject)
	{
		pcObject->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::AssignObject(CEmbeddedObject* pcObject)
{
	if (!OBJECTS_LOCKING())
	{
		AssignObjectInside(pcObject);
	}
	else
	{
		AssignObjectLocking(pcObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// AssignObject with the locking on.  A pointer that points at nothing yet
// is pointed at pcObject as any is, which a reader may do.  Apart, as
// PointToLocking is.
//
//////////////////////////////////////////////////////////////////////////
NO_INLINE void CPointer::AssignObjectLocking(CEmbeddedObject* pcObject)
{
	if (IsAllThreadsOwn(pcObject))
	{
		ObjectsOwnEnters();
		AssignObjectInside(pcObject);
		ObjectsOwnLeaves();
	}
	else if ((LoadObject() != NULL) || !PointToReading(&pcObject))
	{
		ObjectsLock();
		AssignObjectInside(NotForwarding(pcObject));
		ObjectsUnlock();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// AssignObject for what holds the object system's lock already: what reads a pointer out of the object system's own
// tables (the memory index, a weak reference) and must hold the object before the lock is let go.
//
//////////////////////////////////////////////////////////////////////////
void CPointer::AssignObjectInside(CEmbeddedObject* pcObject)
{
	OBJECTS_INSIDE();

	mpcObject = pcObject;
	if (mpcObject)
	{
		mpcObject->AddStackFrom(this, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::AddHeapFrom(CBaseObject* pcFrom)
{
	OBJECTS_INSIDE();

	if (mpcObject)
	{
		mpcObject->AddHeapFrom(pcFrom, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::DumpFroms(void)
{
	CBaseObject* pvBaseObject = BaseObject();

	if (pvBaseObject)
	{
		pvBaseObject->DumpFroms();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::DumpPointerTos(void)
{
	CBaseObject* pvBaseObject = BaseObject();

	if (pvBaseObject)
	{
		pvBaseObject->DumpPointerTos();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointer::Dump(void)
{
	CChars			sz;
	CBaseObject*	pvBaseObject = BaseObject();

	if (pvBaseObject)
	{
		pvBaseObject->Dump();
	}
	else
	{
		sz.Init("NULL\n");
		sz.Dump();
		sz.Kill();
	}
}

