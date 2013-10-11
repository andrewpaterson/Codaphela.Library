#include "BaseObject.h"
#include "PointerObject.h"
#include "ObjectRemapFrom.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObjectRemapFrom::Remap(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	int					iCount;
	int					iNumEmbeddedOld;
	CEmbeddedObject*	pcEmbeddedOld;
	int					iNumEmbeddedNew;
	int					iNumEmbedded;
	CEmbeddedObject*	pcEmbeddedNew;
	int					i;

	iNumEmbeddedNew = pcNew->GetNumEmbedded();
	iNumEmbeddedOld = pcOld->GetNumEmbedded();

	iNumEmbedded = iNumEmbeddedNew;
	if (iNumEmbeddedOld < iNumEmbeddedNew)
	{
		iNumEmbedded = iNumEmbeddedOld;
	}

	iCount = 0;
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbeddedOld = pcOld->GetEmbeddedObject(i);
		pcEmbeddedNew = pcNew->GetEmbeddedObject(i);

		iCount += RemapEmbedded(pcEmbeddedNew, pcEmbeddedOld);
	}
	pcOld->KillFroms();

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

	iCount = 0;

	iNumHeapFroms = pcOld->CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < iNumHeapFroms; i++)
	{
		pvFrom = pcOld->PrivateGetHeapFrom(i);
		iCount += pvFrom->RemapTos(pcOld, pcNew);
		pcNew->AddHeapFrom(pvFrom);
	}

	pcStackPointer = pcOld->GetFirstStackFrom();
	while (pcStackPointer)
	{
		pcStackPointer->GetPointer()->UnsafePointTo(pcNew);
		pcStackPointer = pcStackPointer->GetNext();
	}

	pcNew->SetDistToRoot(pcOld->DistToRoot());

	return iCount;
}

