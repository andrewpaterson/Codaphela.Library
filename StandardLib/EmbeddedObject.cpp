#include "BaseLib/Log.h"
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
void CEmbeddedObject::KillInternal(BOOL bHeapFromChanged)
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
int CEmbeddedObject::RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* mpcObject)
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
BOOL CEmbeddedObject::IsNamed(void)
{
	return FALSE;
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
void CEmbeddedObject::ClearIdentifiers(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CEmbeddedObject::IsBaseObject(void)
{
	return FALSE;
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
BOOL CEmbeddedObject::IsNotEmbedded(void)
{
	return mpcEmbedded == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CEmbeddedObject::IsEmbedded(void)
{
	return mpcEmbedded != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CEmbeddedObject::GetEmbeddedIndex(CEmbeddedObject* pcEmbedded)
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
int CEmbeddedObject::BaseNumPointerTos(void)
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
	int					iNumFroms;
	CBaseObject**		ppcPointedFrom;
	int					i;
	CBaseObject*		pcPointedFrom;

	iNumFroms = mapHeapFroms.NumElements();
	if (iNumFroms > 0)
	{
		ppcPointedFrom = mapHeapFroms.GetData();
		for (i = 0; i < iNumFroms; i++)
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
void CEmbeddedObject::AddHeapFrom(CBaseObject* pcFromObject, BOOL bValidate)
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
void CEmbeddedObject::RemoveHeapFrom(CBaseObject* pcFromObject, BOOL bValidate)
{
	CBaseObject*	pcContainer;
	CObjects*		pcObjectsThisIn;

	pcObjectsThisIn = GetObjectsThisIn();
	//Removing a 'from' kicks off memory reclamation.  This is the entry point for memory management.
	PrivateRemoveHeapFrom(pcFromObject);

	pcContainer = GetEmbeddingContainer();
	pcContainer->TryFree(TRUE, TRUE);

#ifdef _DEBUG
	if (bValidate)
	{
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
BOOL CEmbeddedObject::PrivateRemoveHeapFrom(CBaseObject* pcFrom)
{
	return mapHeapFroms.Remove(&pcFrom, FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CEmbeddedObject::GetHeapFrom(int iFrom)
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
BOOL CEmbeddedObject::HasStackPointers(void)
{
	int		iNumStackPointers;

	iNumStackPointers = GetEmbeddingContainer()->NumStackFroms();
	return iNumStackPointers != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CEmbeddedObject::HasHeapFroms(void)
{
	int		iNumHeapPointers;

	iNumHeapPointers = GetEmbeddingContainer()->NumHeapFroms();
	return iNumHeapPointers != 0;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CEmbeddedObject::NumHeapFroms(void)
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
	int					i;
	int					iNumFroms;
	CEmbeddedObject*	pcFrom;
	int					iDistToRoot;

	iNearestRoot = MAX_DIST_TO_ROOT;
	pcNearestPointedFrom = NULL;
	iNumFroms = mapHeapFroms.NumElements();
	for (i = 0; i < iNumFroms; i++)
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
	int					i;
	int					iNumFroms;
	CEmbeddedObject*	pcFrom;
	int					iDistToRoot;

	iNearestRoot = MAX_DIST_TO_ROOT;
	pcNearestPointedFrom = NULL;
	iNumFroms = mapHeapFroms.NumElements();
	for (i = 0; i < iNumFroms; i++)
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
	int					i;
	int					iNumFroms;
	CEmbeddedObject*	pcFrom;
	int					iDistToStack;

	iNearestStack = MAX_DIST_TO_ROOT;
	pcNearestPointedFrom = NULL;
	iNumFroms = mapHeapFroms.NumElements();
	for (i = 0; i < iNumFroms; i++)
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
int CEmbeddedObject::NumStackFroms(void)
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
int CEmbeddedObject::NumTotalFroms(void)
{
	return NumStackFroms() + NumHeapFroms();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CEmbeddedObject::ContainsFrom(CEmbeddedObject* pcBaseObject)
{
	int					i;
	int					iNumFroms;
	CEmbeddedObject*	pcFrom;

	iNumFroms = mapHeapFroms.NumElements();
	for (i = 0; i < iNumFroms; i++)
	{
		pcFrom = *mapHeapFroms.Get(i);
		if (pcFrom == pcBaseObject)
		{
			return TRUE;
		}
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::AddStackFrom(CPointer* pcPointer)
{
	CStackPointers*	pcStackPointers;

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
BOOL CEmbeddedObject::IsInStack(void)
{
	return GetDistToStack() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::RemoveStackFromTryKill(CPointer* pcPointer, BOOL bFreeIfNoRoot)
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
			pcContainer->TryFree(bFreeIfNoRoot, FALSE);
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
BOOL CEmbeddedObject::IsAllocatedInObjects(void)
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
void CEmbeddedObject::PrintObject(CChars* psz, BOOL bEmbedded)
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidateFrom(CBaseObject* pcBaseObject)
{
	CChars	szObject;
	CChars	szFromObject;
	int		iThisDistToRoot;
	int		iOtherDistToRoot;
	BOOL	bFromPointsTo;

	iThisDistToRoot = GetDistToRoot();
	iOtherDistToRoot = pcBaseObject->GetDistToRoot();
	if ((iThisDistToRoot >= ROOT_DIST_TO_ROOT && iOtherDistToRoot >= ROOT_DIST_TO_ROOT) && (iOtherDistToRoot < iThisDistToRoot - 1))
	{
		szObject.Init();
		PrintObject(&szObject, IsEmbedded());
		szFromObject.Init();
		pcBaseObject->PrintObject(&szFromObject, pcBaseObject->IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", szObject.Text(), "} pointed to from object {", szFromObject.Text(), "} cannot have a DistToRoot that is different by more than 1.", NULL);
		szFromObject.Kill();
		szObject.Kill();
	}

	bFromPointsTo = pcBaseObject->ContainsPointerTo(this);
	if (!bFromPointsTo)
	{
		szObject.Init();
		PrintObject(&szObject, IsEmbedded());
		szFromObject.Init();
		pcBaseObject->PrintObject(&szFromObject, pcBaseObject->IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", szObject.Text(), "} pointed to from object {", szFromObject.Text(), "} does not have a from pointing to.", NULL);
		szFromObject.Kill();
		szObject.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidatePointerTo(CEmbeddedObject* pcPointedTo)
{
	CChars	szObject;
	CChars	szToObject;
	BOOL	bToPointsToFrom;

	bToPointsToFrom = pcPointedTo->ContainsFrom(this);
	if (!bToPointsToFrom)
	{
		szObject.Init();
		PrintObject(&szObject, IsEmbedded());
		szToObject.Init();
		pcPointedTo->PrintObject(&szToObject, pcPointedTo->IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", szObject.Text(), "} pointing to object {", szToObject.Text(), "} does not have a from pointing to.", NULL);
		szToObject.Kill();
		szObject.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidateFroms(void)
{
	int					i;
	CEmbeddedObject*	pcBaseObject;

	for (i = 0; i < mapHeapFroms.NumElements(); i++)
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
void CEmbeddedObject::GetStackFroms(CArrayTypedPointerPtr* papcFroms)
{
	CStackPointer*	pcStackPointer;
	CPointer*		pcPointer;

	pcStackPointer = mpcStackFroms;
	while (pcStackPointer)
	{
		pcPointer = pcStackPointer->GetPointer();
		papcFroms->Add(&pcPointer);

		pcStackPointer = pcStackPointer->GetNext();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CEmbeddedObject::TestRemoveHeapFrom(CBaseObject* pcFromObject)
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
	CBaseObject*					pcContainer;

	pcContainer = GetEmbeddingContainer();
	gcLogger.Error2(szMethod, " Cannot be called on embedded object of class [", ClassName(), "] with embedding index [", IndexToString(pcContainer->GetIndex()),"] and embedding class [", pcContainer->ClassName(), "].", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::LogExpectedToBeInitialised(char* szMethod)
{
	CBaseObject*					pcContainer;

	pcContainer = GetEmbeddingContainer();
	gcLogger.Error2(szMethod, " Cannot be called on un-initialised object of class [", ClassName(), "] with index [", IndexToString(GetIndex()),"].", NULL);
}

