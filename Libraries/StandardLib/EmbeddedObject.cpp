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
#include "BaseLib/Logger.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "Objects.h"
#include "BaseObject.h"
#include "Collection.h"
#include "DistCalculator.h"
#include "EmbeddedObject.h"
#include "BaseLib/NativeThread.h"
#include "ObjectsLock.h"
#include "ObjectsThreads.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject::CEmbeddedObject()
{
	mapHeapFromObjects.Init();
	mapHeapFromCollections.Init();
	mpcStackFroms = NULL;
	mpcEmbedded = NULL;
	mpsHub = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject::~CEmbeddedObject()
{
	//Never put code here.  Use ~BaseObject instead.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::Kill(void)
{
	NotImplemented(__METHOD__);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::KillInternal(bool bHeapFromChanged, bool bValidateNotEmbedded)
{
	NotImplemented(__METHOD__);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::Evict(void)
{
	NotImplemented(__METHOD__);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::Allocate(CObjects* pcObjects)
{
	mapHeapFromObjects.Init();
	mapHeapFromCollections.Init();
	mpsHub = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CEmbeddedObject::RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* mpcObject)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CEmbeddedObject::GetIndex(void) const
{
	return INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::IsNamed(void) const
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CEmbeddedObject::GetName(void) const
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::FreeIdentifiers(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::IsBaseObject(void) const
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CEmbeddedObject::GetEmbeddingContainer(void)
{
	CEmbeddedObject*	pcContainer;

	pcContainer = this;
	while (pcContainer->IsEmbedded())
	{
		pcContainer = (CEmbeddedObject*)pcContainer->mpcEmbedded;
	}

	return (CBaseObject*)pcContainer;
}


//////////////////////////////////////////////////////////////////////////
//
// The order of a set's members and a map's keys (ppvObject1 and 2 point to
// pointers to objects): by index, which a hollow shares with the object
// loaded for it, so the two are the same as far as order goes and loading
// moves nothing.  An embedded object goes by its container's index, then
// its place in the container (as it is written: CObjectWriter::
// WriteDependent).  Objects with no index (not allocated in Objects) go by
// address.  NULL is first.
//
//////////////////////////////////////////////////////////////////////////
int CEmbeddedObject::CompareIndex(const void* ppvObject1, const void* ppvObject2)
{
	CEmbeddedObject*	pc1;
	CEmbeddedObject*	pc2;
	OIndex				oi1;
	OIndex				oi2;

	//Read whole: a load may be rewriting a set's element.  Nearly always two objects, not embedded, with indexes of
	//their own: no call.
	pc1 = (CEmbeddedObject*)AtomicLoadPointer((void**)ppvObject1);
	pc2 = (CEmbeddedObject*)AtomicLoadPointer((void**)ppvObject2);
	if ((pc1 != NULL) && (pc2 != NULL) && (pc1->mpcEmbedded == NULL) && (pc2->mpcEmbedded == NULL))
	{
		oi1 = ((CBaseObject*)pc1)->moi;
		oi2 = ((CBaseObject*)pc2)->moi;
		if (oi1 != oi2)
		{
			return (oi1 < oi2) ? -1 : 1;
		}
	}
	return CompareIndexOtherwise(pc1, pc2);
}


//////////////////////////////////////////////////////////////////////////
//
// CompareIndex's other cases: NULL, embedded objects, an object compared
// with itself, a hollow and the object loaded for it, objects with no
// index.
//
//////////////////////////////////////////////////////////////////////////
NO_INLINE int CEmbeddedObject::CompareIndexOtherwise(CEmbeddedObject* pc1, CEmbeddedObject* pc2)
{
	CBaseObject*		pcContainer1;
	CBaseObject*		pcContainer2;
	size				uiPlace1;
	size				uiPlace2;

	if ((pc1 == pc2) || (pc1 == NULL) || (pc2 == NULL))
	{
		return ComparePtr(pc1, pc2);
	}

	pcContainer1 = (pc1->mpcEmbedded == NULL) ? (CBaseObject*)pc1 : pc1->GetEmbeddingContainer();
	pcContainer2 = (pc2->mpcEmbedded == NULL) ? (CBaseObject*)pc2 : pc2->GetEmbeddingContainer();
	if (pcContainer1->moi != pcContainer2->moi)
	{
		return (pcContainer1->moi < pcContainer2->moi) ? -1 : 1;
	}
	if ((pcContainer1 != pcContainer2) && (pcContainer1->moi == INVALID_O_INDEX))
	{
		return ComparePtr(pcContainer1, pcContainer2);
	}

	uiPlace1 = pcContainer1->GetEmbeddedIndex(pc1);
	uiPlace2 = pcContainer2->GetEmbeddedIndex(pc2);
	return (uiPlace1 < uiPlace2) ? -1 : ((uiPlace1 > uiPlace2) ? 1 : 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::LogicalDirty(void)
{
	//This needs to be called when a object is done being dirtied and can be flushed in the case where the Codabase is write-through

	CObjects*		pcObjects;
	CBaseObject*	pcObject;

	pcObject = GetEmbeddingContainer();
	pcObjects = pcObject->GetObjectsThisIn();
	if (pcObjects)
	{
		pcObjects->LogicalDirty(pcObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::IsNotEmbedded(void) const
{
	return mpcEmbedded == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::IsEmbedded(void) const
{
	return mpcEmbedded != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CEmbeddedObject::GetEmbeddedIndex(CEmbeddedObject* pcEmbedded)
{
	if (pcEmbedded == this)
	{
		return 0;
	}
	else
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CEmbeddedObject::BaseNumPointerTos(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Objects with embedded objects add theirs (CObject).
//
//////////////////////////////////////////////////////////////////////////
size CEmbeddedObject::NumPointerTos(void)
{
	return BaseNumPointerTos();
}


//////////////////////////////////////////////////////////////////////////
//
// Objects with no pointers (hollow ones) have none to remove.
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::RemovePointerTo(CEmbeddedObject* pcTo)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Objects with embedded objects add theirs (CObject).
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::GetPointerTos(CArrayEmbeddedObjectPtr* papcTos)
{
	BaseGetPointerTos(papcTos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::BaseGetPointerTos(CArrayEmbeddedObjectPtr* papcTos)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::SetEmbedding(CBaseObject* pcEmbedded)
{
	mpcEmbedded = pcEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CEmbeddedObject::GetEmbedding(void)
{
	return mpcEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::FreePointers(void)
{
	KillHeapFromHub();
	mapHeapFromObjects.Kill();
	mapHeapFromCollections.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::PostRemapFroms(void)
{
	KillHeapFromHub();
	mapHeapFromObjects.ReInit();
	mapHeapFromCollections.ReInit();
	mpcStackFroms = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::RemoveAllHeapFroms(void)
{
	size			uiNumFroms;
	CBaseObject**	ppcPointedFrom;
	size			i;
	CBaseObject*	pcPointedFrom;

	uiNumFroms = mapHeapFromObjects.NumElements();
	if (uiNumFroms > 0)
	{
		ppcPointedFrom = mapHeapFromObjects.GetData();
		for (i = 0; i < uiNumFroms; i++)
		{
			pcPointedFrom = ppcPointedFrom[i];
			pcPointedFrom->RemovePointerTo(this);
		}

		mapHeapFromObjects.ReInit();
	}

	uiNumFroms = mapHeapFromCollections.NumElements();
	if (uiNumFroms > 0)
	{
		ppcPointedFrom = mapHeapFromCollections.GetData();
		for (i = 0; i < uiNumFroms; i++)
		{
			pcPointedFrom = ppcPointedFrom[i];
			pcPointedFrom->RemovePointerTo(this);
		}

		mapHeapFromCollections.ReInit();
	}
	KillHeapFromHub();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::AddHeapFrom(CBaseObject* pcFromObject, bool bValidate)
{
	OBJECTS_INSIDE();

	int		iDistToRoot;

	if (pcFromObject != NULL)
	{
#ifdef _DEBUG
		ValidateWorkersOwn(pcFromObject);
#endif // _DEBUG
		iDistToRoot = pcFromObject->GetDistToRoot();
		AddHeapPointer(pcFromObject);
		if (iDistToRoot >= ROOT_DIST_TO_ROOT)
		{
			CDistCalculator::FromAdded(GetEmbeddingContainer(), iDistToRoot + 1);
		}
		else
		{
			GetEmbeddingContainer()->HeapFromAdded(pcFromObject);
		}

#ifdef _DEBUG
		ValidateObjectsConsistency(bValidate);
#endif // _DEBUG
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A from goes in the Collection or the Object array depending on what it is.
// Froms are only added for real Objects; HollowObjects cannot point to anything.
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::AddHeapPointer(CBaseObject* pcFromObject)
{
	ptr		uiLocation;

	if (pcFromObject->IsCollection())
	{
		mapHeapFromCollections.Add(pcFromObject);
		uiLocation = (ptr)(mapHeapFromCollections.NumElements() - 1) * 2 + 1;
	}
	else
	{
		mapHeapFromObjects.Add(pcFromObject);
		uiLocation = (ptr)(mapHeapFromObjects.NumElements() - 1) * 2;
	}

	if (mpsHub != NULL)
	{
		mpsHub->cLocations.Add(pcFromObject, uiLocation);
	}
	else if (CEmbeddedObject::NumHeapFroms() >= HEAP_FROM_HUB_MIN_FROMS)
	{
		MakeHeapFromHub();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::AddHeapPointerForRemap(CBaseObject* pcFromObject)
{
	AddHeapPointer(pcFromObject);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether this (or the object it is embedded in) is in the batch being
// freed: its froms are about to be dropped with it.
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::IsBeingFreed(void)
{
	return FixBool(GetEmbeddingContainer()->GetFlags() & OBJECT_FLAGS_FREED);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::GetHeapFromPointers(CArrayHeapFromPointer* pacPointers)
{
	//Adds every pointer field, in every Object on the heap, that points to this.

	size			i;
	size			uiNumFroms;
	CBaseObject*	pcFrom;
	uint32			uiAdded;

	//An Object with more than one pointer to this is in the heap froms more than once; it is stamped the first time.
	uiAdded = GetObjectsOrDefault()->StartWalk() | WALK_STATE_TOUCHED;
	uiNumFroms = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < uiNumFroms; i++)
	{
		pcFrom = CEmbeddedObject::GetHeapFrom(i);
		if (pcFrom->GetWalkStamp() != uiAdded)
		{
			pcFrom->SetWalkStamp(uiAdded);
			pcFrom->GetPointerFieldsTo(this, pacPointers);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::GetPointerFieldsTo(CEmbeddedObject* pcTo, CArrayHeapFromPointer* pacPointers)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFreeResult CEmbeddedObject::RemoveHeapFromTryFree(CBaseObject* pcFromObject)
{
	OBJECTS_INSIDE();

	bool			bRemoved;
	EFreeResult		eResult;
#ifdef _DEBUG
	CObjects*		pcObjectsThisIn;

	pcObjectsThisIn = GetObjectsThisIn();
#endif

	//Removing a 'from' kicks off memory reclamation.  This is the entry point for memory management.
	bRemoved = UnlinkHeapFrom(pcFromObject);
	if (!bRemoved)
	{
		gcLogger.Error2(__METHOD__, " Could not remove Object {", ObjectToString(pcFromObject), "} Heap-From from Object {", ObjectToString(this), "}.", NULL);
		return FR_Error;
	}

	eResult = GetObjectsOrDefault()->FromRemoved(this, true);

#ifdef _DEBUG
	if (pcObjectsThisIn)
	{
		pcObjectsThisIn->ValidateObjectsConsistency();
	}
#endif

	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::UnlinkHeapFrom(CBaseObject* pcFrom)
{
	OBJECTS_INSIDE();

	bool	bRemoved;

	if (mpsHub != NULL)
	{
		bRemoved = UnlinkHubHeapFrom(pcFrom);
		if (CEmbeddedObject::NumHeapFroms() < HEAP_FROM_HUB_MIN_FROMS / 2)
		{
			KillHeapFromHub();
		}
		return bRemoved;
	}
	else if (pcFrom->IsCollection())
	{
		return mapHeapFromCollections.Remove(&pcFrom, false);
	}
	else
	{
		return mapHeapFromObjects.Remove(&pcFrom, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The hub knows where pcFrom is.  The last from of its kind takes its
// place, so nothing else moves.
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::UnlinkHubHeapFrom(CBaseObject* pcFrom)
{
	SPointerLocationsIterator	sIter;
	ptr							uiLocation;
	ptr							uiLastLocation;
	size						uiIndex;
	size						uiLast;
	CBaseObject**				ppcFroms;
	CBaseObject*				pcLast;
	bool						bCollection;

	if (!mpsHub->cLocations.StartIteration(&sIter, pcFrom, &uiLocation))
	{
		return false;
	}
	mpsHub->cLocations.Remove(pcFrom, uiLocation);
	if (mpsHub->pcCloserFrom == pcFrom)
	{
		mpsHub->pcCloserFrom = NULL;
	}

	uiIndex = (size)(uiLocation / 2);
	bCollection = (uiLocation % 2) == 1;
	if (bCollection)
	{
		ppcFroms = mapHeapFromCollections.GetData();
		uiLast = mapHeapFromCollections.NumElements() - 1;
	}
	else
	{
		ppcFroms = mapHeapFromObjects.GetData();
		uiLast = mapHeapFromObjects.NumElements() - 1;
	}

	if (uiIndex != uiLast)
	{
		pcLast = ppcFroms[uiLast];
		uiLastLocation = (ptr)uiLast * 2 + (bCollection ? 1 : 0);
		mpsHub->cLocations.Remove(pcLast, uiLastLocation);
		mpsHub->cLocations.Add(pcLast, uiLocation);
		ppcFroms[uiIndex] = pcLast;
	}

	if (bCollection)
	{
		mapHeapFromCollections.RemoveAt(uiLast, false);
	}
	else
	{
		mapHeapFromObjects.RemoveAt(uiLast, false);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether one of this object's own heap froms (not those of the objects
// embedded in it) is attached and closer to root than iDistToRoot.  A hub
// remembers the one it found, and the next look starts where it was found,
// going round to the start: clearing a hub's froms one after another
// (taking an array of them off the root) asks again after each, and would
// otherwise pass all those already cleared every time.
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::HasOwnHeapFromCloserThan(int iDistToRoot)
{
	size			i;
	size			uiNumFroms;
	size			uiStep;
	CBaseObject*	pcFrom;
	int				iDist;

	if ((mpsHub != NULL) && (mpsHub->pcCloserFrom != NULL))
	{
		iDist = mpsHub->pcCloserFrom->GetDistToRoot();
		if ((iDist >= 0) && (iDist < iDistToRoot))
		{
			return true;
		}
	}

	uiNumFroms = CEmbeddedObject::NumHeapFroms();
	i = 0;
	if ((mpsHub != NULL) && (mpsHub->uiScanFrom < uiNumFroms))
	{
		i = mpsHub->uiScanFrom;
	}
	for (uiStep = 0; uiStep < uiNumFroms; uiStep++)
	{
		pcFrom = CEmbeddedObject::GetHeapFrom(i);
		iDist = pcFrom->GetDistToRoot();
		if ((iDist >= 0) && (iDist < iDistToRoot))
		{
			if (mpsHub != NULL)
			{
				mpsHub->pcCloserFrom = pcFrom;
				mpsHub->uiScanFrom = i;
			}
			return true;
		}
		i++;
		if (i == uiNumFroms)
		{
			i = 0;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::MakeHeapFromHub(void)
{
	size	i;
	size	uiNumFroms;

	mpsHub = NewMalloc<SHeapFromHub>();
	mpsHub->cLocations.Init();
	mpsHub->pcCloserFrom = NULL;
	mpsHub->uiScanFrom = 0;

	uiNumFroms = mapHeapFromObjects.NumElements();
	for (i = 0; i < uiNumFroms; i++)
	{
		mpsHub->cLocations.Add(mapHeapFromObjects.GetPtr(i), (ptr)i * 2);
	}
	uiNumFroms = mapHeapFromCollections.NumElements();
	for (i = 0; i < uiNumFroms; i++)
	{
		mpsHub->cLocations.Add(mapHeapFromCollections.GetPtr(i), (ptr)i * 2 + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::KillHeapFromHub(void)
{
	if (mpsHub != NULL)
	{
		mpsHub->cLocations.Kill();
		SafeFree(mpsHub);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CEmbeddedObject::GetHeapFrom(size iFrom)
{
	CBaseObject*	pFrom;
	size			uiNumObjects;

	uiNumObjects = mapHeapFromObjects.NumElements();
	if (iFrom < uiNumObjects)
	{
		pFrom = mapHeapFromObjects.GetPtr(iFrom);
	}
	else
	{
		pFrom = mapHeapFromCollections.GetPtr(iFrom - uiNumObjects);
	}
	return pFrom;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::GetHeapFroms(CArrayBaseObjectPtrE32* papcFroms)
{
	size	i;
	size	uiNumCollections;

	papcFroms->Copy(&mapHeapFromObjects);
	uiNumCollections = mapHeapFromCollections.NumElements();
	for (i = 0; i < uiNumCollections; i++)
	{
		papcFroms->Add(mapHeapFromCollections.GetPtr(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointers* CEmbeddedObject::GetStackPointers(void)
{
	return GetObjectsOrDefault()->GetStackPointers();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether anything on the stack points to the object this is embedded in
// (or to any object embedded in that).  A test of each one's list, not a
// count of them.
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::HasStackFroms(void)
{
	CBaseObject*		pcContainer;
	CEmbeddedObject*	pcEmbedded;
	size				i;
	size				uiNumEmbedded;

	pcContainer = GetEmbeddingContainer();
	uiNumEmbedded = pcContainer->NumEmbedded();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = pcContainer->GetEmbeddedObject(i);
		if ((pcEmbedded->mpcStackFroms != NULL) || ThreadStackFromsHas(pcEmbedded))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::HasHeapFroms(void)
{
	size	iNumHeapPointers;

	iNumHeapPointers = GetEmbeddingContainer()->NumHeapFroms();
	return iNumHeapPointers != 0;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CEmbeddedObject::NumHeapFroms(void)
{
	return mapHeapFromObjects.NumElements() + mapHeapFromCollections.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CEmbeddedObject::NumStackFroms(void)
{
	size	uiNumOnThreads;

	uiNumOnThreads = ThreadStackFromsNum(this);
	if (mpcStackFroms == NULL)
	{
		return uiNumOnThreads;
	}
	else
	{
		return mpcStackFroms->NumPointers() + uiNumOnThreads;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CEmbeddedObject::NumTotalFroms(void)
{
	return NumStackFroms() + NumHeapFroms();
}


//////////////////////////////////////////////////////////////////////////
//
// A hub looks the from up where it keeps them (ValidateHeapFromHub checks
// those against the froms), so validating every pointer to a hub isn't a
// search of all its froms each time.
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::ContainsFrom(CEmbeddedObject* pcBaseObject)
{
	size						i;
	size						uiNumFroms;
	CEmbeddedObject*			pcFrom;
	SPointerLocationsIterator	sIter;
	ptr							uiLocation;

	if (mpsHub != NULL)
	{
		return mpsHub->cLocations.StartIteration(&sIter, pcBaseObject, &uiLocation);
	}

	uiNumFroms = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < uiNumFroms; i++)
	{
		pcFrom = CEmbeddedObject::GetHeapFrom(i);
		if (pcFrom == pcBaseObject)
		{
			return true;
		}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::AddStackFrom(CPointer* pcPointer, bool bValidate)
{
	OBJECTS_INSIDE();

#ifdef _DEBUG
	ValidateWorkersOwn(NULL);
#endif // _DEBUG

	//A registered thread keeps its own stack froms of objects attached to the root (ObjectsThreads.h).
	if (ThreadStackFromAdd(this, pcPointer))
	{
		return true;
	}
	return StackFromLinked(LinkStackFrom(GetStackPointers()->Add(pcPointer)), bValidate);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::AddStackFrom(CCollection* pcCollection, bool bValidate)
{
	OBJECTS_INSIDE();

	return StackFromLinked(LinkStackFrom(GetStackPointers()->Add(pcCollection)), bValidate);
}


//////////////////////////////////////////////////////////////////////////
//
// After a stack from was (bAdded) linked to this: the container is told,
// and (in a debug build) the objects validated.
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::StackFromLinked(bool bAdded, bool bValidate)
{
	if (bAdded)
	{
		GetEmbeddingContainer()->StackFromAdded();
#ifdef _DEBUG
		ValidateObjectsConsistency(bValidate);
#endif // _DEBUG
	}
	return bAdded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::AddStackPointersForRemap(CStackPointer* pcStackPointer)
{
	return LinkStackFrom(pcStackPointer);
}


//////////////////////////////////////////////////////////////////////////
//
// Puts pcStackPointer (and any stack pointers linked after it) in front of
// this object's stack froms: the newest is first.  Adding one is constant
// time, and as stack pointers usually go in the reverse of the order they
// came, so is removing it.
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::LinkStackFrom(CStackPointer* pcStackPointer)
{
	if (pcStackPointer == NULL)
	{
		return false;
	}

	pcStackPointer->LinkBefore(mpcStackFroms);
	mpcStackFroms = pcStackPointer;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFreeResult CEmbeddedObject::RemoveStackFromTryFree(CPointer* pcPointer)
{
	OBJECTS_INSIDE();

	bool	bRemoved;

	bRemoved = UnlinkStackFrom(pcPointer);
	if (!bRemoved)
	{
		gcLogger.Error2(__METHOD__, " Could not remove Pointer Stack-From from Object {", ObjectToString(this), "}.", NULL);
		return FR_Error;
	}

	return GetObjectsOrDefault()->FromRemoved(this, false);
}


//////////////////////////////////////////////////////////////////////////
//
// Takes pcFrom (a CPointer or a CCollection) out of the stack froms that
// start at *ppcStackFroms.  False if it isn't one of them.
//
//////////////////////////////////////////////////////////////////////////
template<class TFrom>
static bool UnlinkStackFromList(CStackPointers* pcStackPointers, CStackPointer** ppcStackFroms, TFrom* pcFrom)
{
	bool	bRemoved;

	if (*ppcStackFroms == NULL)
	{
		return false;
	}
	*ppcStackFroms = pcStackPointers->Remove(*ppcStackFroms, pcFrom, &bRemoved);
	return bRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::UnlinkStackFrom(CPointer* pcPointer)
{
	OBJECTS_INSIDE();

	if (ObjectsThreadsRegistered())
	{
		return UnlinkStackFromOfThreads(pcPointer);
	}
	return UnlinkStackFromList(GetStackPointers(), &mpcStackFroms, pcPointer);
}


//////////////////////////////////////////////////////////////////////////
//
// While threads are registered a pointer's from is one the running thread keeps (ObjectsThreads.h), or is on this,
// or (a pointer let go of by a thread that didn't take it) is one some other thread keeps.  Apart, so that with one
// thread UnlinkStackFrom is as short as it was.
//
//////////////////////////////////////////////////////////////////////////
NO_INLINE bool CEmbeddedObject::UnlinkStackFromOfThreads(CPointer* pcPointer)
{
	return ThreadStackFromRemove(this, pcPointer) || UnlinkStackFromList(GetStackPointers(), &mpcStackFroms, pcPointer) || ThreadStackFromRemoveAnywhere(this, pcPointer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::UnlinkStackFrom(CCollection* pcCollection)
{
	OBJECTS_INSIDE();

	return UnlinkStackFromList(GetStackPointers(), &mpcStackFroms, pcCollection);
}


//////////////////////////////////////////////////////////////////////////
//
// Held by the root (or the root itself): a stack from of it taken or let go of frees nothing and changes no
// distance.
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::IsAttachedToRoot(void)
{
	return GetDistToRoot() >= ROOT_DIST_TO_ROOT;
}


//////////////////////////////////////////////////////////////////////////
//
// The stack from of a pointer that has been pointed at this, added by a thread that is reading (ObjectsThreads.h)
// and has seen this attached to the root.  A registered thread keeps the from of a main object itself; the home
// thread's goes on this, where only it and writers look, and a worker's of its own object on this, where only it does.
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::AddStackFromReading(CPointer* pcPointer)
{
	OBJECTS_INSIDE();

#ifdef _DEBUG
	ValidateWorkersOwn(NULL);
#endif // _DEBUG

	if (ObjectsReaderIsHome() || !ThreadStackFromAdd(this, pcPointer))
	{
		LinkStackFrom(GetStackPointers()->Add(pcPointer));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// And taken off by one.  False if it isn't where a reader may look: a registered thread's from of an object that
// wasn't attached when it was taken is on the object, and a pointer another thread took is in that thread's table.
// A writer finds those.
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::UnlinkStackFromReading(CPointer* pcPointer)
{
	OBJECTS_INSIDE();

	if (ObjectsReaderIsHome() || IsThreadsOwn())
	{
		return UnlinkStackFromList(GetStackPointers(), &mpcStackFroms, pcPointer);
	}
	return ThreadStackFromRemove(this, pcPointer);
}


//////////////////////////////////////////////////////////////////////////
//
// In the running thread's own object system (a worker's): only it uses
// this, so this keeps its froms itself, whatever the locking.
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::IsThreadsOwn(void)
{
	CObjects*	pcOwn;

	pcOwn = ObjectsOwn();
	return (pcOwn != NULL) && (GetObjectsThisIn() == pcOwn);
}


//////////////////////////////////////////////////////////////////////////
//
// KeepWorkersOwnTos with the locking on.
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::KeepWorkersOwnTosLocking(CArrayEmbeddedObjectPtr* papcTos, size uiFirst)
{
	CObjects*	pcWorker;
	size		i;

	pcWorker = GetWorkerObjectsLocking();
	if (pcWorker != NULL)
	{
		i = papcTos->NumElements();
		while (i > uiFirst)
		{
			i--;
			if (IsAcross(pcWorker, papcTos->GetPtr(i)))
			{
				papcTos->RemoveAt(i, true);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// GetWorkerObjects with the locking on.
//
//////////////////////////////////////////////////////////////////////////
CObjects* CEmbeddedObject::GetWorkerObjectsLocking(void)
{
	CObjects*	pcObjects;

	pcObjects = GetObjectsThisIn();
	return ((pcObjects != NULL) && pcObjects->IsWorker()) ? pcObjects : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A worker's objects are its own until it adopts them (ThreadSafetyPlan.txt,
// Level 3): no object of another object system points at one (pcFromObject
// is about to), and no other thread holds one (pcFromObject NULL: the
// running thread is about to).
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidateWorkersOwn(CBaseObject* pcFromObject)
{
	CObjects*	pcWorker;
	CObjects*	pcFromObjects;

	pcWorker = GetWorkerObjects();
	if (pcWorker == NULL)
	{
		return;
	}

	if (pcFromObject != NULL)
	{
		pcFromObjects = pcFromObject->GetObjectsThisIn();
		if ((pcFromObjects != NULL) && (pcFromObjects != pcWorker))
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(pcFromObject), "} of another object system points at {", ObjectToString(this), "}, of a worker's: a worker's objects are its own until it adopts them.", NULL);
		}
	}
	else if (ObjectsOwn() != pcWorker)
	{
		gcLogger.Error2(__METHOD__, " A thread that isn't its worker holds {", ObjectToString(this), "}: a worker's objects are its own until it adopts them.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::RemoveAllStackFroms(void)
{
	CStackPointers*		pcStackPointers;
	CStackPointer*		pcStackPointer;

	pcStackPointers = GetStackPointers();
	if (pcStackPointers)
	{
		if (mpcStackFroms)
		{
			pcStackPointer = mpcStackFroms;
			while (pcStackPointer)
			{
				pcStackPointer = pcStackPointer->ClearPointerGetNext(this);
			}

			pcStackPointers->RemoveAll(mpcStackFroms);
			mpcStackFroms = NULL;
		}
	}
	ThreadStackFromsClear(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::IsAllocatedInObjects(void)
{
	return GetObjectsThisIn() != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The objects this is in, or gcObjects for an object on the stack (which
// is in none).  Unlike GetObjects it doesn't check gcObjects is
// initialised: an object on the stack can outlive it.
//
//////////////////////////////////////////////////////////////////////////
CObjects* CEmbeddedObject::GetObjectsOrDefault(void)
{
	CObjects* pcObjects;

	pcObjects = GetObjectsThisIn();
	if (pcObjects)
	{
		return pcObjects;
	}
	return ObjectsCurrent();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObjects* CEmbeddedObject::GetObjects(void)
{
	CObjects* pcObjects;

	pcObjects = GetObjectsThisIn();
	if (!pcObjects)
	{
		if (!ObjectsValidate())
		{
			return NULL;
		}

		pcObjects = ObjectsCurrent();
	}
	return pcObjects;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::HasNes(void) const
{
	CClass*		pcClass;

	pcClass = GetClass();
	if (pcClass)
	{
		return pcClass->HasNes();
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CEmbeddedObject::GetNesPointerFrom(size* piStart)
{
	size				j;
	size				iNumHeapFroms;
	CEmbeddedObject*	pcFromObject;
	bool				bHasNesToThis;

	iNumHeapFroms = CEmbeddedObject::NumHeapFroms();
	for (j = *piStart; j < iNumHeapFroms; j++)
	{
		pcFromObject = CEmbeddedObject::GetHeapFrom(j);
		if (pcFromObject->HasNes())
		{
			bHasNesToThis = pcFromObject->HasNesPointerToThis(this);
			if (bHasNesToThis)
			{
				*piStart = j + 1;
				return pcFromObject;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointer* CEmbeddedObject::GetFirstStackFrom(void) const
{
	return mpcStackFroms;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::GetStackFroms(CArrayStackPointerE32* papcFroms)
{
	CStackPointer*	pcStackPointer;
	SStackPointer*	psPointer;

	pcStackPointer = mpcStackFroms;
	while (pcStackPointer)
	{
		psPointer = pcStackPointer->Get();
		papcFroms->Add(psPointer);

		pcStackPointer = pcStackPointer->GetNext();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::TestRemoveHeapFrom(CBaseObject* pcFromObject)
{
	return UnlinkHeapFrom(pcFromObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::TestRemoveStackFrom(CPointer* pcPointer)
{
	return UnlinkStackFrom(pcPointer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::IsEmbeddingContainerAllocatedInObjects(void)
{
	CEmbeddedObject*	pcEmbedded;

	pcEmbedded = GetEmbeddingContainer();
	if (pcEmbedded)
	{
		return pcEmbedded->IsAllocatedInObjects();
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
bool CEmbeddedObject::FailNotExpectedToBeEmbedded(const char* szMethod)
{
	// Ensure that an Object embedded in another Object as a field of that object is not being addressed directly.  (nothing to do with pointers on the stack)

	CBaseObject*	pcContainer;
	const char*		szContainerClass;

	pcContainer = GetEmbeddingContainer();
	szContainerClass = pcContainer->ClassName();

	//If DESTRUCTABLE(x) is not present at the top of the class after CONSTRUCTABLE(x) then wrong destructor on BaseObject will be called.
	return gcLogger.Error2(szMethod, " Cannot be called on embedded object of class [", ClassName(), "] with embedding index [", IndexToString(pcContainer->GetIndex()), "] and embedding class [", szContainerClass, "].  Ensure DESTRUCTABLE(", szContainerClass, "); is present.", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::FailExpectedToBeEmbedded(const char* szMethod)
{
	CChars	sz;

	sz.Init();
	PrintObject(&sz);
	gcLogger.Error2(szMethod, " Must be called on an object embedded in another, not on {", sz.Text(), "}.", NULL);
	sz.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::FailCantInitFreedObject(const char* szMethod)
{

	return gcLogger.Error2(szMethod, " Cannot be called on a freed object of class [", ClassName(), "] with index [", IndexToString(GetIndex()), "].", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::FailExpectedToBeInitialised(const char* szMethod)
{

	return gcLogger.Error2(szMethod, " Cannot be called on un-initialised object of class [", ClassName(), "] with index [", IndexToString(GetIndex()), "].", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::FailExpectedToNotBeInitialised(const char* szMethod)
{

	return gcLogger.Error2(szMethod, " Cannot be called on already initialised object of class [", ClassName(), "] with index [", IndexToString(GetIndex()), "].", NULL);
}

