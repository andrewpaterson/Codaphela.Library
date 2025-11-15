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
	mapHeapFroms.Init();
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
void CEmbeddedObject::KillInternal(bool bHeapFromChanged)
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
void CEmbeddedObject::BaseGetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos)
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
void CEmbeddedObject::FreePointers()
{
	mapHeapFroms.Kill();

	RemoveAllStackFroms();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::PostRemapFroms(void)
{
	mapHeapFroms.Kill();
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

	uiNumFroms = mapHeapFroms.NumElements();
	if (uiNumFroms > 0)
	{
		ppcPointedFrom = mapHeapFroms.GetData();
		for (i = 0; i < uiNumFroms; i++)
		{
			pcPointedFrom = ppcPointedFrom[i];
			pcPointedFrom->RemovePointerTo(this);
		}

		mapHeapFroms.ReInit();
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
		mapHeapFroms.Add(&pcFromObject);
		if (iDistToRoot >= ROOT_DIST_TO_ROOT)
		{
			pcEmbedding = GetEmbeddingContainer();
			pcEmbedding->SetExpectedDistToRoot(iDistToRoot + 1);
		}

#ifdef _DEBUG
		if (bValidate)
		{
			if (IsAllocatedInObjects())
			{
				GetObjectsThisIn()->ValidateObjectsConsistency();
			}
		}
#endif // _DEBUG
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::UnsafeAddHeapFrom(CBaseObject* pcFromObject)
{
	mapHeapFroms.Add(&pcFromObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidateInitialised(char* szMethod)
{
	if (!IsInitialised())
	{
		LogExpectedToBeInitialised(szMethod);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::RemoveHeapFrom(CBaseObject* pcFromObject, bool bValidate)
{
	CBaseObject*	pcContainer;
	CObjects*		pcObjectsThisIn;
	bool			bRemoved;

	//Removing a 'from' kicks off memory reclamation.  This is the entry point for memory management.
	bRemoved = PrivateRemoveHeapFrom(pcFromObject);
	if (!bRemoved)
	{
		gcLogger.Error2(__METHOD__, " Could not remove Object {", ObjectToString(pcFromObject), "} heap from from Object {", ObjectToString(this), "}.", NULL);
	}

	pcContainer = GetEmbeddingContainer();
	pcContainer->TryFree(true, true);

#ifdef _DEBUG
	if (bValidate)
	{
		pcObjectsThisIn = GetObjectsThisIn();
		if (pcObjectsThisIn)
		{
			pcObjectsThisIn->ValidateObjectsConsistency();
		}
	}
#endif
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::PrivateRemoveHeapFrom(CBaseObject* pcFrom)
{
	return mapHeapFroms.Remove(&pcFrom, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CEmbeddedObject::GetHeapFrom(size iFrom)
{
	CBaseObject**	ppFrom;

	ppFrom = mapHeapFroms.Get(iFrom);
	if (ppFrom)
	{
		return *ppFrom;
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
void CEmbeddedObject::GetHeapFroms(CArrayTemplateEmbeddedBaseObjectPtr* papcFroms)
{
	papcFroms->Copy((CArrayTemplateEmbeddedBaseObjectPtr*)&mapHeapFroms);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEmbeddedObject::HasStackPointers(void)
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
	return mapHeapFroms.NumElements();
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
	uiNumFroms = mapHeapFroms.NumElements();
	for (i = 0; i < uiNumFroms; i++)
	{
		pcFrom = *mapHeapFroms.Get(i);
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
	uiNumFroms = mapHeapFroms.NumElements();
	for (i = 0; i < uiNumFroms; i++)
	{
		pcFrom = *mapHeapFroms.Get(i);
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
CEmbeddedObject* CEmbeddedObject::GetClosestFromToStack(void)
{
	int					iNearestStack;
	CEmbeddedObject*	pcNearestPointedFrom;
	size				i;
	size				uiNumFroms;
	CEmbeddedObject*	pcFrom;
	int					iDistToStack;

	iNearestStack = MAX_DIST_TO_ROOT;
	pcNearestPointedFrom = NULL;
	uiNumFroms = mapHeapFroms.NumElements();
	for (i = 0; i < uiNumFroms; i++)
	{
		pcFrom = *mapHeapFroms.Get(i);
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

	uiNumFroms = mapHeapFroms.NumElements();
	for (i = 0; i < uiNumFroms; i++)
	{
		pcFrom = *mapHeapFroms.Get(i);
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
void CEmbeddedObject::AddStackFrom(CPointer* pcPointer)
{
	CStackPointers*		pcStackPointers;

	pcStackPointers = GetStackPointers();
	if (pcStackPointers)
	{
		if (mpcStackFroms)
		{
			pcStackPointers->Add(pcPointer, mpcStackFroms);
		}
		else
		{
			mpcStackFroms = pcStackPointers->Add(pcPointer);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::AddStackFrom(CCollection* pcCollection)
{
	CStackPointers* pcStackPointers;

	pcStackPointers = GetStackPointers();
	if (pcStackPointers)
	{
		if (mpcStackFroms)
		{
			pcStackPointers->Add(pcCollection, mpcStackFroms);
		}
		else
		{
			mpcStackFroms = pcStackPointers->Add(pcCollection);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::AddStackFroms(CStackPointer* pcStackPointer)
{
	CStackPointers*	pcStackPointers;

	pcStackPointers = GetStackPointers();
	if (pcStackPointers)
	{
		if (mpcStackFroms)
		{
			pcStackPointers->Add(pcStackPointer, mpcStackFroms);
		}
		else
		{
			mpcStackFroms = pcStackPointer;
		}
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
void CEmbeddedObject::RemoveStackFromTryKill(CPointer* pcPointer, bool bFreeIfNoRoot)
{
	CStackPointers*	pcStackPointers;
	CBaseObject*	pcContainer;

	if (mpcStackFroms)
	{
		pcStackPointers = GetStackPointers();
		if (pcStackPointers)
		{
			mpcStackFroms = pcStackPointers->Remove(mpcStackFroms, pcPointer);
			pcContainer = GetEmbeddingContainer();
			pcContainer->TryFree(bFreeIfNoRoot, false);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::PrivateRemoveStackFrom(CPointer* pcPointer)
{
	CStackPointers*	pcStackPointers;

	pcStackPointers = GetStackPointers();
	if (pcStackPointers)
	{
		if (mpcStackFroms)
		{
			mpcStackFroms = pcStackPointers->Remove(mpcStackFroms, pcPointer);
		}
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
				pcStackPointer = pcStackPointer->ClearPointerGetNext();
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
	CObjects*	pcObjects;

	pcObjects = GetObjectsThisIn();
	return pcObjects != NULL;
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
char* CEmbeddedObject::PrintObject(CChars* psz)
{
	return PrintObject(psz, IsEmbedded());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CEmbeddedObject::PrintObject(CChars* psz, bool bEmbedded)
{
	int		iDistToRoot;
	int		iDistToStack;

	psz->Append(PointerToString(this));
	psz->Append(" [");
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

	uiHeapFroms = mapHeapFroms.NumElements();
	for (i = 0; i < uiHeapFroms; i++)
	{
		pcBaseObject = *mapHeapFroms.Get(i);

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
void CEmbeddedObject::GetStackFroms(CArrayStackPointer* papcFroms)
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
void CEmbeddedObject::TestRemoveStackFrom(CPointer* pcPointer)
{
	PrivateRemoveStackFrom(pcPointer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidateNotEmbedded(char* szMethod)
{
	if (IsEmbedded())
	{
		LogNotExpectedToBeEmbedded(szMethod);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::LogNotExpectedToBeEmbedded(char* szMethod)
{
	CBaseObject* pcContainer;

	pcContainer = GetEmbeddingContainer();
	gcLogger.Error2(szMethod, " Cannot be called on embedded object of class [", ClassName(), "] with embedding index [", IndexToString(pcContainer->GetIndex()), "] and embedding class [", pcContainer->ClassName(), "].", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::LogCantInitFreedObject(char* szMethod)
{
	CBaseObject* pcContainer;

	pcContainer = GetEmbeddingContainer();
	gcLogger.Error2(szMethod, " Cannot be called on a freed object of class [", ClassName(), "] with index [", IndexToString(GetIndex()), "].", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::LogExpectedToBeInitialised(char* szMethod)
{
	CBaseObject*					pcContainer;

	pcContainer = GetEmbeddingContainer();
	gcLogger.Error2(szMethod, " Cannot be called on un-initialised object of class [", ClassName(), "] with index [", IndexToString(GetIndex()), "].", NULL);
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

