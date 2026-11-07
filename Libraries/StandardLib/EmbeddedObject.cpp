#include "BaseLib/Logger.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "Objects.h"
#include "BaseObject.h"
#include "Collection.h"
#include "DistCalculator.h"
#include "EmbeddedObject.h"


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
OIndex CEmbeddedObject::GetIndex(void)
{
	return INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::IsNamed(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CEmbeddedObject::GetName(void)
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
bool CEmbeddedObject::IsBaseObject(void)
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
bool CEmbeddedObject::IsNotEmbedded(void)
{
	return mpcEmbedded == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::IsEmbedded(void)
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
	int		iDistToRoot;

	if (pcFromObject != NULL)
	{
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
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::HasStackFroms(void)
{
	size	iNumStackPointers;

	iNumStackPointers = GetEmbeddingContainer()->NumStackFroms();
	return iNumStackPointers != 0;
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
	if (mpcStackFroms == NULL)
	{
		return 0;
	}
	else
	{
		return mpcStackFroms->NumPointers();
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
	bool	bAdded;

	bAdded = LinkStackFrom(GetStackPointers()->Add(pcPointer));
	if (bAdded)
	{
		GetEmbeddingContainer()->StackFromAdded();
	}

#ifdef _DEBUG
	if (bAdded)
	{
		ValidateObjectsConsistency(bValidate);
	}
#endif // _DEBUG

	return bAdded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::AddStackFrom(CCollection* pcCollection, bool bValidate)
{
	bool	bAdded;

	bAdded = LinkStackFrom(GetStackPointers()->Add(pcCollection));
	if (bAdded)
	{
		GetEmbeddingContainer()->StackFromAdded();
	}

#ifdef _DEBUG
	if (bAdded)
	{
		ValidateObjectsConsistency(bValidate);
	}
#endif // _DEBUG

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
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::LinkStackFrom(CStackPointer* pcStackPointer)
{
	//Adds pcStackPointer (and any stack pointers linked after it) to the end of this Object's stack froms.

	if (pcStackPointer == NULL)
	{
		return false;
	}

	if (mpcStackFroms)
	{
		return GetStackPointers()->SetLastNext(pcStackPointer, mpcStackFroms);
	}
	else
	{
		mpcStackFroms = pcStackPointer;
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFreeResult CEmbeddedObject::RemoveStackFromTryFree(CPointer* pcPointer)
{
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
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::UnlinkStackFrom(CPointer* pcPointer)
{
	bool	bRemoved;

	if (mpcStackFroms == NULL)
	{
		return false;
	}

	mpcStackFroms = GetStackPointers()->Remove(mpcStackFroms, pcPointer, &bRemoved);
	return bRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::UnlinkStackFrom(CCollection* pcCollection)
{
	bool	bRemoved;

	if (mpcStackFroms == NULL)
	{
		return false;
	}

	mpcStackFroms = GetStackPointers()->Remove(mpcStackFroms, pcCollection, &bRemoved);
	return bRemoved;
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
	return &gcObjects;
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

		pcObjects = &gcObjects;
	}
	return pcObjects;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes pcOther's froms (and their hub): pcOther, a copy of a fat hollow
// this has overwritten in place, is dropped without being killed.
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::CopyFields(CEmbeddedObject* pcOther)
{
	mapHeapFromObjects = pcOther->mapHeapFromObjects;
	mapHeapFromCollections = pcOther->mapHeapFromCollections;
	mpcStackFroms = pcOther->mpcStackFroms;
	mpsHub = pcOther->mpsHub;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::HasNes(void)
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
				*piStart = j+1;
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
CStackPointer* CEmbeddedObject::GetFirstStackFrom(void)
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
bool CEmbeddedObject::FailNotExpectedToBeEmbedded(char* szMethod)
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
bool CEmbeddedObject::FailExpectedToBeEmbedded(char* szMethod)
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
bool CEmbeddedObject::FailCantInitFreedObject(char* szMethod)
{

	return gcLogger.Error2(szMethod, " Cannot be called on a freed object of class [", ClassName(), "] with index [", IndexToString(GetIndex()), "].", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::FailExpectedToBeInitialised(char* szMethod)
{

	return gcLogger.Error2(szMethod, " Cannot be called on un-initialised object of class [", ClassName(), "] with index [", IndexToString(GetIndex()), "].", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::FailExpectedToNotBeInitialised(char* szMethod)
{

	return gcLogger.Error2(szMethod, " Cannot be called on already initialised object of class [", ClassName(), "] with index [", IndexToString(GetIndex()), "].", NULL);
}

