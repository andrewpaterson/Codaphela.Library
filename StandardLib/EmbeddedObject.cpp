#include "BaseLib/Logger.h"
#include "Objects.h"
#include "BaseObject.h"
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
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::BaseGetPointerTos(CArrayEmbeddedObjectPtr* papcTos)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters)
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
	mapHeapFromObjects.Kill();
	mapHeapFromCollections.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::PostRemapFroms(void)
{
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::AddHeapFrom(CBaseObject* pcFromObject, bool bValidate)
{
	CBaseObject*	pcEmbedding;
	int				iDistToRoot;

	if (pcFromObject != NULL)
	{
		iDistToRoot = pcFromObject->GetDistToRoot();
		AddHeapPointer(pcFromObject);
		if (iDistToRoot >= ROOT_DIST_TO_ROOT)
		{
			pcEmbedding = GetEmbeddingContainer();
			pcEmbedding->SetExpectedDistToRoot(iDistToRoot + 1);
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
	if (pcFromObject->IsCollection())
	{
		mapHeapFromCollections.Add(pcFromObject);
	}
	else
	{
		mapHeapFromObjects.Add(pcFromObject);
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
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::GetHeapFromPointers(CArrayHeapFromPointer* pacPointers)
{
	//Adds every pointer field, in every Object on the heap, that points to this.

	size			i;
	size			j;
	size			uiNumFroms;
	CBaseObject*	pcFrom;
	bool			bAlreadyAdded;

	uiNumFroms = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < uiNumFroms; i++)
	{
		pcFrom = CEmbeddedObject::GetHeapFrom(i);

		//An Object with more than one pointer to this is in the heap froms more than once.
		bAlreadyAdded = false;
		for (j = 0; j < i; j++)
		{
			if (CEmbeddedObject::GetHeapFrom(j) == pcFrom)
			{
				bAlreadyAdded = true;
				break;
			}
		}

		if (!bAlreadyAdded)
		{
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
EFreeResult CEmbeddedObject::RemoveHeapFromTryFree(CBaseObject* pcFromObject, bool bValidate)
{
	CBaseObject*	pcContainer;
	CObjects*		pcObjectsThisIn;
	bool			bRemoved;
	EFreeResult		eResult;

	pcObjectsThisIn = GetObjectsThisIn();

	//Removing a 'from' kicks off memory reclamation.  This is the entry point for memory management.
	bRemoved = PrivateRemoveHeapFrom(pcFromObject);
	if (!bRemoved)
	{
		gcLogger.Error2(__METHOD__, " Could not remove Object {", ObjectToString(pcFromObject), "} Heap-From from Object {", ObjectToString(this), "}.", NULL);
		return FR_Error;
	}

	pcContainer = GetEmbeddingContainer();
	eResult = pcContainer->TryFree(true);

#ifdef _DEBUG
	if (bValidate)
	{
		if (pcObjectsThisIn)
		{
			pcObjectsThisIn->ValidateObjectsConsistency();
		}
	}
#endif

	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::PrivateRemoveHeapFrom(CBaseObject* pcFrom)
{
	if (pcFrom->IsCollection())
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
CEmbeddedObject* CEmbeddedObject::GetClosestFromToRoot(void)
{
	int					iNearestRoot;
	CEmbeddedObject*	pcNearestPointedFrom;
	size				i;
	size				uiNumFroms;
	CEmbeddedObject*	pcFrom;
	int					iDistToRoot;

	iNearestRoot = MAX_DIST_TO_ROOT;
	pcNearestPointedFrom = NULL;
	uiNumFroms = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < uiNumFroms; i++)
	{
		pcFrom = CEmbeddedObject::GetHeapFrom(i);
		iDistToRoot = pcFrom->GetDistToRoot();
		if (iDistToRoot >= ROOT_DIST_TO_ROOT)
		{
			if (iDistToRoot < iNearestRoot)
			{
				iNearestRoot = iDistToRoot;
				pcNearestPointedFrom = pcFrom;
			}
		}
	}

	return pcNearestPointedFrom;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CEmbeddedObject::GetClosestFromForCanFindRoot(void)
{
	int					iNearestRoot;
	CEmbeddedObject*	pcNearestPointedFrom;
	size				i;
	size				uiNumFroms;
	CEmbeddedObject*	pcFrom;
	int					iDistToRoot;

	iNearestRoot = MAX_DIST_TO_ROOT;
	pcNearestPointedFrom = NULL;
	uiNumFroms = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < uiNumFroms; i++)
	{
		pcFrom = CEmbeddedObject::GetHeapFrom(i);
		iDistToRoot = pcFrom->GetDistToRoot();
		if ((iDistToRoot >= ROOT_DIST_TO_ROOT) && (!pcFrom->TestedForRoot()))
		{
			if (iDistToRoot < iNearestRoot)
			{
				iNearestRoot = iDistToRoot;
				pcNearestPointedFrom = pcFrom;
			}
		}
	}

	return pcNearestPointedFrom;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CEmbeddedObject::GetClosestHeapFromToStack(void)
{
	int					iNearestStack;
	CEmbeddedObject*	pcNearestPointedFrom;
	size				i;
	size				uiNumFroms;
	CEmbeddedObject*	pcFrom;
	int					iDistToStack;
	//CEmbeddedObject*	pcExpectedNearest;

	iNearestStack = MAX_DIST_TO_ROOT;
	pcNearestPointedFrom = NULL;
	uiNumFroms = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < uiNumFroms; i++)
	{
		pcFrom = CEmbeddedObject::GetHeapFrom(i);
		iDistToStack = pcFrom->GetDistToStack();
		if (iDistToStack >= 0)
		{
			if (iDistToStack < iNearestStack)
			{
				iNearestStack = iDistToStack;
				pcNearestPointedFrom = pcFrom;
			}
		}
	}

	return pcNearestPointedFrom;
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
bool CEmbeddedObject::IsInStack(void)
{
	return GetDistToStack() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFreeResult CEmbeddedObject::RemoveStackFromTryFree(CPointer* pcPointer)
{
	bool	bRemoved;

	bRemoved = PrivateRemoveStackFrom(pcPointer);
	if (!bRemoved)
	{
		gcLogger.Error2(__METHOD__, " Could not remove Pointer Stack-From from Object {", ObjectToString(this), "}.", NULL);
		return FR_Error;
	}

	return GetEmbeddingContainer()->TryFree(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFreeResult CEmbeddedObject::RemoveStackFromTryFree(CCollection* pcCollection)
{
	bool	bRemoved;

	bRemoved = PrivateRemoveStackFrom(pcCollection);
	if (!bRemoved)
	{
		gcLogger.Error2(__METHOD__, " Could not remove Collection Stack-From from Object {", ObjectToString(this), "}.", NULL);
		return FR_Error;
	}

	return GetEmbeddingContainer()->TryFree(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::PrivateRemoveStackFrom(CPointer* pcPointer)
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
bool CEmbeddedObject::PrivateRemoveStackFrom(CCollection* pcCollection)
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
	int		iDistToStack;

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
	iDistToStack = GetDistToStack();
	if (iDistToStack != UNKNOWN_DIST_TO_STACK)
	{
		psz->Append(",");
		psz->Append(iDistToStack);
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

	uiHeapFroms = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < uiHeapFroms; i++)
	{
		pcBaseObject = CEmbeddedObject::GetHeapFrom(i);

		if (pcBaseObject->IsBaseObject())
		{
			ValidateFrom((CBaseObject*)pcBaseObject);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidatePointerTos(void)
{
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
	return PrivateRemoveHeapFrom(pcFromObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::TestRemoveStackFrom(CPointer* pcPointer)
{
	return PrivateRemoveStackFrom(pcPointer);
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

