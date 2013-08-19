#include "BaseObject.h"
#include "ObjectRemapFrom.h"


int CObjectRemapFrom::RemapFrom(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
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

		iCount += RemapEmbeddedFrom(pcEmbeddedNew, pcEmbeddedOld);
	}

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObjectRemapFrom::RemapEmbeddedFrom(CEmbeddedObject* pcNew, CEmbeddedObject* pcOld)
{
	int				iNumFroms;
	int				i;
	CBaseObject*	pvFrom;
	int				iCount;

	iCount = 0;

	iNumFroms = pcOld->CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < iNumFroms; i++)
	{
		pvFrom = pcOld->PrivateGetFrom(i);
		iCount += pvFrom->RemapTos(pcOld, pcNew);
		pcNew->AddFrom(pvFrom);
	}

	pcNew->CopyFroms(pcOld);
	pcNew->SetDistToRoot(pcOld->DistToRoot());

	return iCount;
}

