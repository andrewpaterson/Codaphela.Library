#include "Objects.h"
#include "ObjectRemapFrom.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObjectRemapFrom::Remap(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew, BOOL bClearOldIndex)
{
	int					iCount;
	CEmbeddedObject*	pcEmbeddedOld;
	int					iNumEmbedded;
	CEmbeddedObject*	pcEmbeddedNew;
	int					i;
	BOOL				bHeapFromChanged;

	bHeapFromChanged = pcOld->HasHeapFroms();
	iNumEmbedded = CalculateNumEmbedded(pcOld, pcNew);
	iCount = 0;

	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbeddedOld = pcOld->GetEmbeddedObject(i);
		pcEmbeddedNew = pcNew->GetEmbeddedObject(i);

		iCount += RemapEmbedded(pcEmbeddedNew, pcEmbeddedOld);
	}

	if (bClearOldIndex)
	{
		pcOld->ClearIndex();
		pcOld->ClearName();
	}
	pcOld->Kill(bHeapFromChanged);

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObjectRemapFrom::RemapEmbedded(CEmbeddedObject* pcNew, CEmbeddedObject* pcOld)
{
	int				iNumHeapFroms;
	int				i;
	CBaseObject*	pvFrom;
	int				iCount;
	CStackPointer*	pcStackPointer;
	CStackPointer*	pcFirstStackPointer;
	int				iOldDistToRoot;
	BOOL			bDistChanged;;

	iCount = 0;

	iNumHeapFroms = pcOld->CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < iNumHeapFroms; i++)
	{
		pvFrom = pcOld->CEmbeddedObject::GetHeapFrom(i);
		iCount += pvFrom->RemapTos(pcOld, pcNew);
		pcNew->AddHeapFrom(pvFrom, FALSE);
	}

	pcFirstStackPointer = pcOld->GetFirstStackFrom();
	pcStackPointer = pcFirstStackPointer;
	if (pcStackPointer)
	{
		while (pcStackPointer)
		{
			pcStackPointer->GetPointer()->UnsafePointTo(pcNew);
			pcStackPointer = pcStackPointer->GetNext();
		}
		pcNew->AddStackFroms(pcFirstStackPointer);
	}

	iOldDistToRoot = pcOld->GetDistToRoot();
	bDistChanged = pcNew->SetDistToRoot(iOldDistToRoot);
	if (bDistChanged)
	{
		pcNew->SetPointedTosExpectedDistToRoot(iOldDistToRoot);
	}

	pcOld->PostRemapFroms();

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObjectRemapFrom::CalculateNumEmbedded(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	int					iNumEmbeddedOld;
	int					iNumEmbeddedNew;
	int					iNumEmbedded;

	iNumEmbeddedNew = pcNew->GetNumEmbedded();
	iNumEmbeddedOld = pcOld->GetNumEmbedded();

	iNumEmbedded = iNumEmbeddedNew;
	if (iNumEmbeddedOld < iNumEmbeddedNew)
	{
		iNumEmbedded = iNumEmbeddedOld;
	}

	return iNumEmbedded;
}

