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
	size				iNumHeapFroms;
	size				i;
	CBaseObject*		pvFrom;
	size				iCount;
	CStackPointer*		pcStackPointer;
	CStackPointer*		pcFirstStackPointer;
	SStackPointer*		psStackPointer;

	iCount = 0;

	iNumHeapFroms = pcOld->CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < iNumHeapFroms; i++)
	{
		pvFrom = pcOld->CEmbeddedObject::GetHeapFrom(i);
		iCount += pvFrom->RemapPointerTos(pcOld, pcNew);

		if (pcNew->IsInitialised())
		{
			pcNew->AddHeapFrom(pvFrom, false);
		}
		else
		{
			//If the object is not initialised it cannot point to any other objects
			//This means this dist to root calculation for tos can be skipped.
			pcNew->UnsafeAddHeapFrom(pvFrom);
			pcNew->SetDistToRoot(pcOld->GetDistToRoot());
		}
	}

	pcFirstStackPointer = pcOld->GetFirstStackFrom();
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
		pcNew->AddStackFroms(pcFirstStackPointer);
	}

	pcOld->PostRemapFroms();

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

