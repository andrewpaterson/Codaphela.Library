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
	//This destructor will only be called if the object was allocated on the stack.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CEmbeddedObject::RemapTos(CEmbeddedObject* pcOld, CEmbeddedObject* mpcObject)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CEmbeddedObject::GetOI(void)
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
BOOL CEmbeddedObject::IsDirty(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ClearIndex(void)
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
int CEmbeddedObject::UnsafeNumEmbeddedObjectTos(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::UnsafeGetEmbeddedObjectTos(CArrayEmbeddedObjectPtr* papcTos)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::SetEmbedded(CBaseObject* pcEmbedded)
{
	mpcEmbedded = pcEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::KillInternalData()
{
	mapHeapFroms.Kill();
	mpcStackFroms = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::KillFroms(void)
{
	mapHeapFroms.Kill();
	mpcStackFroms = NULL;  //This is wrong.  Your need to mark all the stack froms as unused.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::RemoveAllHeapFroms(void)
{
	int					iNumFroms;
	CEmbeddedObject**	ppcPointedFrom;
	int					i;
	CEmbeddedObject*	pcPointedFrom;

	iNumFroms = mapHeapFroms.NumElements();
	if (iNumFroms > 0)
	{
		ppcPointedFrom = (CEmbeddedObject**)mapHeapFroms.GetData();
		for (i = 0; i < iNumFroms; i++)
		{
			pcPointedFrom = ppcPointedFrom[i];
			pcPointedFrom->RemoveTo(this);
		}

		mapHeapFroms.ReInit();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::AddHeapFrom(CBaseObject* pcFromObject)
{
	if (pcFromObject != NULL)
	{
		mapHeapFroms.Add(&pcFromObject);
		if (pcFromObject->miDistToRoot >= ROOT_DIST_TO_ROOT)
		{
			GetEmbeddingContainer()->SetExpectedDistToRoot(pcFromObject->miDistToRoot+1);
		}

		GetObjects()->ValidateConsistency();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::AddHeapFrom(CBaseObject* pcFromObject, BOOL bValidate)
{
	if (pcFromObject != NULL)
	{
		mapHeapFroms.Add(&pcFromObject);
		if (pcFromObject->miDistToRoot >= ROOT_DIST_TO_ROOT)
		{
			GetEmbeddingContainer()->SetExpectedDistToRoot(pcFromObject->miDistToRoot+1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::RemoveHeapFrom(CBaseObject* pcFromObject)
{
	//Removing a 'from' kicks off memory reclamation.  This is the entry point for memory management.
	PrivateRemoveHeapFrom(pcFromObject);

	GetEmbeddingContainer()->TryKill(TRUE);

	GetObjects()->ValidateConsistency();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::RemoveHeapFrom(CBaseObject* pcFromObject, BOOL bValidate)
{
	//Removing a 'from' kicks off memory reclamation.  This is the entry point for memory management.
	PrivateRemoveHeapFrom(pcFromObject);

	GetEmbeddingContainer()->TryKill(TRUE);
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
void CEmbeddedObject::GetHeapFroms(CArrayEmbeddedBaseObjectPtr* papcFroms)
{
	papcFroms->Copy((CArrayEmbeddedBaseObjectPtr*)&mapHeapFroms);
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
BOOL CEmbeddedObject::HasHeapPointers(void)
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
CBaseObject* CEmbeddedObject::GetClosestFromToRoot(void)
{
	int				iNearestRoot;
	CBaseObject*	pcNearestPointedFrom;
	int				i;
	int				iNumFroms;
	CBaseObject*	pcFrom;

	iNearestRoot = MAX_DIST_TO_ROOT;
	pcNearestPointedFrom = NULL;
	iNumFroms = mapHeapFroms.NumElements();
	for (i = 0; i < iNumFroms; i++)
	{
		pcFrom = *mapHeapFroms.Get(i);
		if ((pcFrom->miDistToRoot >= ROOT_DIST_TO_ROOT) && (!pcFrom->TestedForRoot()))  //What the hell is !pcFrom->TestedForRoot() here for?
		{
			if (pcFrom->miDistToRoot < iNearestRoot)
			{
				iNearestRoot = pcFrom->miDistToRoot;
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
CBaseObject* CEmbeddedObject::GetClosestFromToStack(void)
{
	int				iNearestStack;
	CBaseObject*	pcNearestPointedFrom;
	int				i;
	int				iNumFroms;
	CBaseObject*	pcFrom;

	iNearestStack = MAX_DIST_TO_ROOT;
	pcNearestPointedFrom = NULL;
	iNumFroms = mapHeapFroms.NumElements();
	for (i = 0; i < iNumFroms; i++)
	{
		pcFrom = *mapHeapFroms.Get(i);
		if (pcFrom->miDistToStack >= 0)
		{
			if (pcFrom->miDistToStack < iNearestStack)
			{
				iNearestStack = pcFrom->miDistToStack;
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
	int				i;
	int				iNumFroms;
	CBaseObject*	pcFrom;

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
void CEmbeddedObject::RemoveStackFromTryKill(CPointer* pcPointer, BOOL bKillIfNoRoot)
{
	CStackPointers*	pcStackPointers;

	pcStackPointers = GetStackPointers();
	if (pcStackPointers)
	{
		if (mpcStackFroms)
		{
			mpcStackFroms = pcStackPointers->Remove(mpcStackFroms, pcPointer);
			GetEmbeddingContainer()->TryKill(bKillIfNoRoot);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::RemoveStackFrom(CPointer* pcPointer)
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

	pcObjects = GetObjects();
	return pcObjects != NULL;
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
	psz->Append(") ");
	psz->Append(GetOI());
	if (IsNamed())
	{
		psz->Append(" ");
		psz->Append(GetName());
	}
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
	BOOL	bFromPointsToTo;

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

	bFromPointsToTo = pcBaseObject->ContainsTo(this);
	if (!bFromPointsToTo)
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
void CEmbeddedObject::ValidateTo(CEmbeddedObject* pcPointedTo)
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
	int				i;
	CBaseObject*	pcBaseObject;

	for (i = 0; i < mapHeapFroms.NumElements(); i++)
	{
		pcBaseObject = *mapHeapFroms.Get(i);

		ValidateFrom(pcBaseObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidateTos(void)
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
void CEmbeddedObject::GetStackFroms(CArrayPointerPtr* papcFroms)
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
	gcLogger.Error2(szMethod, " Cannot be called on embedded object of class [", ClassName(), "] with embedding index [", IndexToString(pcContainer->GetOI()),"] and embedding class [", pcContainer->ClassName(), "].", NULL);
}

