#include "Objects.h"
#include "ObjectRemapFrom.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObjectRemapFrom::Remap(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	size				iCount;
	CEmbeddedObject*	pcEmbeddedOld;
	size				iNumEmbedded;
	CEmbeddedObject*	pcEmbeddedNew;
	size				i;
	bool				bHeapFromChanged;

	bHeapFromChanged = pcOld->HasHeapFroms();
	iNumEmbedded = CalculateNumEmbedded(pcOld, pcNew);
	iCount = 0;

	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbeddedOld = pcOld->GetEmbeddedObject(i);
		pcEmbeddedNew = pcNew->GetEmbeddedObject(i);

		iCount += RemapEmbedded(pcEmbeddedNew, pcEmbeddedOld);
	}

	pcOld->KillInternal(bHeapFromChanged, true);

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObjectRemapFrom::RemapEmbedded(CEmbeddedObject* pcNew, CEmbeddedObject* pcOld)
{
	return RemapEmbedded(pcNew, pcOld, pcOld);
}


//////////////////////////////////////////////////////////////////////////
//
// pcOld is the address the froms point to.  pcOldFroms holds the froms.  They differ when
// pcOld has been overwritten (a fat hollow converted in place) and pcOldFroms is a copy of it.
//
//////////////////////////////////////////////////////////////////////////
size CObjectRemapFrom::RemapEmbedded(CEmbeddedObject* pcNew, CEmbeddedObject* pcOld, CEmbeddedObject* pcOldFroms)
{
	size				iNumHeapFroms;
	size				i;
	CBaseObject*		pvFrom;
	size				iCount;
	CStackPointer*		pcStackPointer;
	CStackPointer*		pcFirstStackPointer;
	SStackPointer*		psStackPointer;

	iCount = 0;

	iNumHeapFroms = pcOldFroms->CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < iNumHeapFroms; i++)
	{
		pvFrom = pcOldFroms->CEmbeddedObject::GetHeapFrom(i);
		if (pvFrom->GetEmbeddingContainer() == pcOldFroms->GetEmbeddingContainer())
		{
			//The old object points to itself.  That pointer is killed with the old object so it must not be moved to the new object.
			continue;
		}
		iCount += pvFrom->RemapPointerTos(pcOld, pcNew);

		if (pcNew->IsInitialised())
		{
			pcNew->AddHeapFrom(pvFrom, false);
		}
		else
		{
			//If the object is not initialised it cannot point to any other objects
			//This means this dist to root calculation for tos can be skipped.
			pcNew->AddHeapPointerForRemap(pvFrom);
			pcNew->SetDistToRoot(pcOldFroms->GetDistToRoot());
		}
	}

	pcFirstStackPointer = pcOldFroms->GetFirstStackFrom();
	pcStackPointer = pcFirstStackPointer;
	if (pcStackPointer)
	{
		while (pcStackPointer)
		{
			psStackPointer = pcStackPointer->Get();
			if (psStackPointer->meType == SPT_Pointer)
			{
				psStackPointer->u.pcPointer->UnsafePointTo(pcNew);
				iCount++;
			}
			else if (psStackPointer->meType == SPT_Collection)
			{
				iCount += psStackPointer->u.pcCollection->RemapPointerTos(pcOld, pcNew);
			}
			else
			{
				gcLogger.Error2(__METHOD__, " Unknown Stack Pointer Type.", NULL);
			}
			pcStackPointer = pcStackPointer->GetNext();
		}
		pcNew->AddStackPointersForRemap(pcFirstStackPointer);
	}

	pcOldFroms->PostRemapFroms();

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObjectRemapFrom::CalculateNumEmbedded(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	size	iNumEmbeddedOld;
	size	iNumEmbeddedNew;
	size	iNumEmbedded;

	iNumEmbeddedNew = pcNew->NumEmbedded();
	iNumEmbeddedOld = pcOld->NumEmbedded();

	iNumEmbedded = iNumEmbeddedNew;
	if (iNumEmbeddedOld < iNumEmbeddedNew)
	{
		iNumEmbedded = iNumEmbeddedOld;
	}

	return iNumEmbedded;
}

