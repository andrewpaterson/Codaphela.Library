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
#include "BaseObject.h"


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
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has a from object {", ObjectToString(pcBaseObject), "}, which has no pointer to it.", NULL);
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
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} points to object {", ObjectToString(pcPointedTo), "}, which has no from back to it.", NULL);
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
	CStackPointer*		pcStackPointer;
	SStackPointer*		psStackPointer;

	uiHeapFroms = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < uiHeapFroms; i++)
	{
		pcBaseObject = CEmbeddedObject::GetHeapFrom(i);

		if (pcBaseObject->IsBaseObject())
		{
			ValidateFrom((CBaseObject*)pcBaseObject);
		}
	}

	//A stack from is kept by the address of its pointer (or collection), so one that has been moved (copied in a
	//CArrayTemplate that grew, memcpy) no longer points to this.  Pointers are only on the stack or fields of objects (CPointer).
	for (pcStackPointer = mpcStackFroms; pcStackPointer; pcStackPointer = pcStackPointer->GetNext())
	{
		psStackPointer = pcStackPointer->Get();
		if ((psStackPointer->eType == SPT_Pointer) && (psStackPointer->u.pcPointer->Object() != this))
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has a Stack-From Pointer [", PointerToString(psStackPointer->u.pvPointer), "] that no longer points to it.  Was the Pointer moved?", NULL);
		}
		else if ((psStackPointer->eType == SPT_Collection) && !((CBaseObject*)psStackPointer->u.pcCollection)->ContainsPointerTo(this))
		{
			gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} has a Stack-From Collection that no longer contains it.  Was the Collection moved?", NULL);
		}
	}

	ValidateHeapFromHub();
}


//////////////////////////////////////////////////////////////////////////
//
// A hub knows where every heap from is, and its closer from is one of them.
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidateHeapFromHub(void)
{
	size	i;
	size	uiNumFroms;
	size	uiMissing;

	if (mpsHub == NULL)
	{
		return;
	}

	if (mpsHub->cLocations.NumLocations() != CEmbeddedObject::NumHeapFroms())
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} keeps [", SizeToString(mpsHub->cLocations.NumLocations()), "] heap from locations but has [", SizeToString(CEmbeddedObject::NumHeapFroms()), "] heap froms.", NULL);
	}

	uiMissing = 0;
	uiNumFroms = mapHeapFromObjects.NumElements();
	for (i = 0; i < uiNumFroms; i++)
	{
		if (!mpsHub->cLocations.Contains(mapHeapFromObjects.GetPtr(i), (ptr)i * 2))
		{
			uiMissing++;
		}
	}
	uiNumFroms = mapHeapFromCollections.NumElements();
	for (i = 0; i < uiNumFroms; i++)
	{
		if (!mpsHub->cLocations.Contains(mapHeapFromCollections.GetPtr(i), (ptr)i * 2 + 1))
		{
			uiMissing++;
		}
	}
	if (uiMissing != 0)
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} keeps the wrong location for [", SizeToString(uiMissing), "] heap froms.", NULL);
	}

	if ((mpsHub->pcCloserFrom != NULL) && (mpsHub->cLocations.NumLocations(mpsHub->pcCloserFrom) == 0))
	{
		gcLogger.Error2(__METHOD__, " Object {", ObjectToString(this), "} remembers a closer from {", ObjectToString(mpsHub->pcCloserFrom), "} that isn't one of its heap froms.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each of this object's own pointers points to an object with a from back
// to this.  Embedded objects check their own (ValidateEmbeddedConsistency
// goes through them all).
//
//////////////////////////////////////////////////////////////////////////
void CEmbeddedObject::ValidatePointerTos(void)
{
	CArrayEmbeddedObjectPtr		apcTos;
	size						i;
	size						uiNumTos;

	apcTos.Init();
	BaseGetPointerTos(&apcTos);
	uiNumTos = apcTos.NumElements();
	for (i = 0; i < uiNumTos; i++)
	{
		ValidatePointerTo(apcTos.GetPtr(i));
	}
	apcTos.Kill();
}

