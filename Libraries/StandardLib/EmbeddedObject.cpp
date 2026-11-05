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
		return -1;
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

#ifdef _DEBUG
		ValidateObjectsConsistency(bValidate);
#endif // _DEBUG
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidateObjectsConsistency(bool bValidate)
{
	if (bValidate)
	{
		if (IsAllocatedInObjects())
		{
			GetObjectsThisIn()->ValidateObjectsConsistency();
		}
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
	uiAdded = gcObjects.StartWalk() | WALK_STATE_TOUCHED;
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

	eResult = gcObjects.FromRemoved(this, true);

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
// remembers the one it found.
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::HasOwnHeapFromCloserThan(int iDistToRoot)
{
	size			i;
	size			uiNumFroms;
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
	for (i = 0; i < uiNumFroms; i++)
	{
		pcFrom = CEmbeddedObject::GetHeapFrom(i);
		iDist = pcFrom->GetDistToRoot();
		if ((iDist >= 0) && (iDist < iDistToRoot))
		{
			if (mpsHub != NULL)
			{
				mpsHub->pcCloserFrom = pcFrom;
			}
			return true;
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
	return &gcStackPointers;
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
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::ContainsFrom(CEmbeddedObject* pcBaseObject)
{
	size				i;
	size				uiNumFroms;
	CEmbeddedObject*	pcFrom;

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

	return gcObjects.FromRemoved(this, false);
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
char* CEmbeddedObject::PrintObject(CChars* psz)
{
	return PrintObject(psz, IsEmbedded());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CEmbeddedObject::PrintObject(CChars* psz, bool bEmbedded, bool bPointerMemory)
{
	int		iDistToRoot;

	if (bPointerMemory)
	{
		psz->Append(PointerToString(this));
		psz->Append(" ");
	}
	psz->Append("[");
	iDistToRoot = GetDistToRoot();
	if (iDistToRoot >= 0 && iDistToRoot <= 9)
	{
		psz->Append(" ");
	}
	psz->Append(iDistToRoot);
	if (!IsAllocatedInObjects())
	{
		psz->Append(",0");
	}
	psz->Append("]:");

	if (bEmbedded)
	{
		psz->Append("(");
	}
	psz->Append(ClassName());
	psz->Append("(");
	psz->Append(ClassSize());
	psz->Append(") Index:");
	psz->Append(GetIndex());
	if (IsNamed())
	{
		psz->Append(" Name:");
		psz->Append(GetName());
	}
	psz->Append(" Froms:");
	psz->Append(CEmbeddedObject::NumHeapFroms());
	psz->Append(",");
	psz->Append(CEmbeddedObject::NumStackFroms());
	if (bEmbedded)
	{
		psz->Append(")");
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidateFrom(CBaseObject* pcBaseObject)
{
	int		iThisDistToRoot;
	int		iOtherDistToRoot;
	bool	bFromPointsTo;

	iThisDistToRoot = GetDistToRoot();
	iOtherDistToRoot = pcBaseObject->GetDistToRoot();
	if ((iThisDistToRoot >= ROOT_DIST_TO_ROOT && iOtherDistToRoot >= ROOT_DIST_TO_ROOT) && (iOtherDistToRoot < iThisDistToRoot - 1))
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} pointed to from object {", ObjectToString(pcBaseObject), "} cannot have a DistToRoot that is different by more than 1.", NULL);
	}

	bFromPointsTo = pcBaseObject->ContainsPointerTo(this);
	if (!bFromPointsTo)
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} pointed to from object {", ObjectToString(pcBaseObject), "} does not have a from pointing to.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidatePointerTo(CEmbeddedObject* pcPointedTo)
{
	bool	bToPointsToFrom;

	bToPointsToFrom = pcPointedTo->ContainsFrom(this);
	if (!bToPointsToFrom)
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} pointing to Object {", ObjectToString(pcPointedTo), "} does not have a from pointing to.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidateFroms(void)
{
	size				i;
	CEmbeddedObject*	pcBaseObject;
	size				uiHeapFroms;
	CStackPointer*		pcStackPointer;
	SStackPointer*		psStackPointer;

	uiHeapFroms = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < uiHeapFroms; i++)
	{
		pcBaseObject = CEmbeddedObject::GetHeapFrom(i);

		if (pcBaseObject->IsBaseObject())
		{
			ValidateFrom((CBaseObject*)pcBaseObject);
		}
	}

	//A stack from is kept by the address of its pointer (or collection), so one that has been moved (copied in a
	//CArrayTemplate that grew, memcpy) no longer points to this.  Pointers are only on the stack or fields of objects (CPointer).
	for (pcStackPointer = mpcStackFroms; pcStackPointer; pcStackPointer = pcStackPointer->GetNext())
	{
		psStackPointer = pcStackPointer->Get();
		if ((psStackPointer->eType == SPT_Pointer) && (psStackPointer->u.pcPointer->Object() != this))
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has a Stack-From Pointer [", PointerToString(psStackPointer->u.pvPointer), "] that no longer points to it.  Was the Pointer moved?", NULL);
		}
		else if ((psStackPointer->eType == SPT_Collection) && !((CBaseObject*)psStackPointer->u.pcCollection)->ContainsPointerTo(this))
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has a Stack-From Collection that no longer contains it.  Was the Collection moved?", NULL);
		}
	}

	ValidateHeapFromHub();
}


//////////////////////////////////////////////////////////////////////////
//
// A hub knows where every heap from is, and its closer from is one of them.
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidateHeapFromHub(void)
{
	size	i;
	size	uiNumFroms;
	size	uiMissing;

	if (mpsHub == NULL)
	{
		return;
	}

	if (mpsHub->cLocations.NumLocations() != CEmbeddedObject::NumHeapFroms())
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} keeps [", SizeToString(mpsHub->cLocations.NumLocations()), "] heap from locations but has [", SizeToString(CEmbeddedObject::NumHeapFroms()), "] heap froms.", NULL);
	}

	uiMissing = 0;
	uiNumFroms = mapHeapFromObjects.NumElements();
	for (i = 0; i < uiNumFroms; i++)
	{
		if (!mpsHub->cLocations.Contains(mapHeapFromObjects.GetPtr(i), (ptr)i * 2))
		{
			uiMissing++;
		}
	}
	uiNumFroms = mapHeapFromCollections.NumElements();
	for (i = 0; i < uiNumFroms; i++)
	{
		if (!mpsHub->cLocations.Contains(mapHeapFromCollections.GetPtr(i), (ptr)i * 2 + 1))
		{
			uiMissing++;
		}
	}
	if (uiMissing != 0)
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} keeps the wrong location for [", SizeToString(uiMissing), "] heap froms.", NULL);
	}

	if ((mpsHub->pcCloserFrom != NULL) && (mpsHub->cLocations.NumLocations(mpsHub->pcCloserFrom) == 0))
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} remembers a closer from {", ObjectToString(mpsHub->pcCloserFrom), "} that isn't one of its heap froms.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each of this object's own pointers points to an object with a from back
// to this.  Embedded objects check their own (ValidateEmbeddedConsistency
// goes through them all).
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidatePointerTos(void)
{
	CArrayEmbeddedObjectPtr		apcTos;
	size						i;
	size						uiNumTos;

	apcTos.Init();
	BaseGetPointerTos(&apcTos);
	uiNumTos = apcTos.NumElements();
	for (i = 0; i < uiNumTos; i++)
	{
		ValidatePointerTo(apcTos.GetPtr(i));
	}
	apcTos.Kill();
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

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::DumpHeapFroms(void)
{
	size			uiNumElements;
	size			i;
	CBaseObject*	pcFromObject;
	CChars			sz;

	sz.Init();
	uiNumElements = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < uiNumElements; i++)
	{
		pcFromObject = CEmbeddedObject::GetHeapFrom(i);
		sz.Append(i);
		sz.Append(": ");
		pcFromObject->PrintState(&sz);
		sz.AppendNewLine();
	}
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* ObjectToString(CEmbeddedObject* pcObject)
{
	CChars	szChars;
	size	uiCount;
	char*	sz;

	szChars.Init();
	if (pcObject)
	{
		pcObject->PrintObject(&szChars);
	}
	else
	{
		szChars.Append("NULL");
	}

	uiCount = IncrementLogToStringCount();
	sz = gaszLogToStringScratchPad[uiCount];
	StrCpySafe(sz, szChars.Text(), LOG_TO_STRING_MAX_LENGTH);
	szChars.Kill();
	return sz;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* ObjectToString(CEmbeddedObject* pcObject, bool bEmbedded)
{
	CChars	szChars;
	size	uiCount;
	char* sz;

	szChars.Init();
	if (pcObject)
	{
		//This should be PrintState?
		pcObject->PrintObject(&szChars, bEmbedded);
	}
	else
	{
		szChars.Append("NULL");
	}

	uiCount = IncrementLogToStringCount();
	sz = gaszLogToStringScratchPad[uiCount];
	StrCpySafe(sz, szChars.Text(), LOG_TO_STRING_MAX_LENGTH);
	szChars.Kill();
	return sz;
}

