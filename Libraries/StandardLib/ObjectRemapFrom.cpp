/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
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

	//An embedded object pcNew doesn't have (it is of a class with fewer) keeps its froms until pcOld is killed, which
	//clears the pointers to it.
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
			if (psStackPointer->eType == SPT_Pointer)
			{
				psStackPointer->u.pcPointer->UnsafePointTo(pcNew);
				iCount++;
			}
			else if (psStackPointer->eType == SPT_Collection)
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

