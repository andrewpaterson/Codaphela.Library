/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "BaseLib/Log.h"
#include "ObjectSerialiser.h"
#include "ObjectDeserialiser.h"
#include "BaseObject.h"
#include "Objects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject::CBaseObject()
{
	mpcObjectsThisIn = NULL;
	miDistToRoot = UNATTACHED_DIST_TO_ROOT;
	moi = INVALID_O_INDEX;
	miFlags = OBJECT_FLAGS_DIRTY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PreInit(CObjects* pcObjects)
{
	mpcObjectsThisIn = pcObjects;
	Class();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PreInit(void)
{
	//Call this if you need to allocate objects on the stack.
	PreInit(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Class(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Kill(void)
{
	CArrayEmbeddedBaseObjectPtr		apcFromsChanged;
	int								iNumKilled;
	int								iNumFroms;

	//This method is for the user to forcibly kill an object.
	//It is not called internally.  KillThisGraph is method used to free objects that are unattached.

	//This does NOT call CUnknown::Kill.  That is handled elsewhere.

	ValidateNotEmbedded(__METHOD__);

	ClearStackPointersTo();

	iNumFroms = NumHeapFroms();
	if (iNumFroms == 0)
	{
		apcFromsChanged.Init();
		RemoveAllTos(&apcFromsChanged);
		mpcObjectsThisIn->UpdateDistToRootFromSubRoot(&apcFromsChanged);
		apcFromsChanged.Kill();
		iNumKilled = KillThisGraph();
	}
	else
	{
		RemoveAllFroms();
		iNumKilled = KillThisGraph();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::KillDontFree(void)
{
	LOG_OBJECT_DESTRUCTION(this);

	KillData();
	CEmbeddedObject::KillFroms();
	KillToPointers();

	miFlags |= OBJECT_FLAGS_KILLED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Free(void)
{
	if (IsNotEmbedded())
	{
		if (mpcObjectsThisIn)
		{
			mpcObjectsThisIn->RemoveInKill(this);
		}
		moi = INVALID_O_INDEX;
		CUnknown::Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::RemoveHeapFrom(CBaseObject* pcFrom)
{
	//Removing a 'from' kicks off memory reclamation.  This is the entry point for memory management.
	PrivateRemoveFrom(pcFrom);

	GetEmbeddingContainer()->TryKill(FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::TryKill(BOOL bDontTryFindRoot)
{
	ValidateNotEmbedded(__METHOD__);

	BOOL			bHasStackPointers;
	BOOL			bHasHeapPointers;
	BOOL			bCanFindRoot;
	BOOL			bMustKill;

	if (!bDontTryFindRoot)
	{
		bHasStackPointers = HasStackPointers();
		bCanFindRoot = CanFindRoot();

		//If we removed a heap pointer and have no stack pointers and cannot find the root
		bMustKill = !bCanFindRoot && !bHasStackPointers;
		if (bMustKill)
		{
			//then we can kill this object.
			KillThisGraph();
		}
		else if (bCanFindRoot)
		{
			UpdateDistToRootFromPointedFroms();
		}
		else if (bHasStackPointers)
		{
			ClearDistToSubRoot();
			UnattachDistToRoot();
		}
	}
	else
	{
		bHasHeapPointers = HasHeapPointers();
		bHasStackPointers = HasStackPointers();
		bCanFindRoot = IsRoot();

		//If we removed a stack pointer and have no more stack pointers and have no heap pointers (regardless of whether or not they can find the root)
		bMustKill = !bHasHeapPointers && !bHasStackPointers && !bCanFindRoot;
		if (bMustKill)
		{
			//then we can kill this object.
			KillThisGraph();
		}

		//If we still have heap pointers but no stack pointers and we can't find the root then this object is still being initialised 
		//and should not be killed.
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::KillThisGraph(void)
{
	CArrayBaseObjectPtr		apcKilled;
	int						iNumKilled;

	//This method will never be called on an embedded object.
	//ie: Only the containing object can KillThisGraph.

	//This method will only be called on an object that cannot find the root.
	//If an object is being forcibly killed then it's tos and froms will have already been removed.

	if (mpcObjectsThisIn)
	{
		apcKilled.Init(1024);

		CollectThoseToBeKilled(&apcKilled);
		KillCollected(&apcKilled);

		iNumKilled = apcKilled.NumElements();
		apcKilled.Kill();
		return iNumKilled;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::CollectPointedToToBeKilled(CArrayBaseObjectPtr* papcKilled, CBaseObject* pcPointedTo)
{
	BOOL			bHasStackPointers;
	BOOL			bCanFindRoot;
	BOOL			bMustKill;
	CBaseObject*	pcContainer;

	if (pcPointedTo)
	{
		if (!pcPointedTo->IsMarkedUnreachable())
		{
			pcContainer = pcPointedTo->GetEmbeddingContainer();

			bHasStackPointers = pcContainer->HasStackPointers();
			bCanFindRoot = pcContainer->CanFindRoot();

			bMustKill = !bCanFindRoot && !bHasStackPointers;
			if (bMustKill)
			{
				pcContainer->CollectThoseToBeKilled(papcKilled);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::MarkThisForKilling(CArrayBaseObjectPtr* papcKilled)
{
	CBaseObject*		pcTemp;

	//These both assume we are the embedding container.
	ClearDistToRoot();
	SetFlag(OBJECT_FLAGS_UNREACHABLE, TRUE);

	pcTemp = this;
	papcKilled->Add(&pcTemp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::CollectThoseToBeKilled(CArrayBaseObjectPtr* papcKilled)
{
	ValidateNotEmbedded(__METHOD__);

	MarkThisForKilling(papcKilled);
	CollectPointedToToBeKilled(papcKilled);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::KillCollected(CArrayBaseObjectPtr* papcKilled)
{
	mpcObjectsThisIn->Remove(papcKilled);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ClearDistToRoot(void)
{
	miDistToRoot = CLEARED_DIST_TO_ROOT;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CBaseObject::ClearDistToSubRoot(void)
{
	//The object returned here is always the object immediately below the sub-root, not the sub-root itself.
	ValidateNotEmbedded(__METHOD__);

	int								i;
	int								iNumFroms;
	CBaseObject**					ppcPointedFrom;
	CBaseObject*					pcPointedFrom;
	CBaseObject*					pcRootSet;
	CBaseObject*					pcTemp;
	CArrayEmbeddedBaseObjectPtr		apcFroms;
	CBaseObject*					pcContainer;

	ClearDistToRoot();
	
	pcRootSet = NULL;

	apcFroms.Init();
	GetHeapFroms(&apcFroms);
	iNumFroms = apcFroms.NumElements();
	ppcPointedFrom = apcFroms.GetData();
	for (i = 0; i < iNumFroms; i++)
	{
		pcPointedFrom = ppcPointedFrom[i];
		if (pcPointedFrom->miDistToRoot >= ROOT_DIST_TO_ROOT)
		{
			if (pcPointedFrom->IsSubRoot())
			{
				//No point in stepping back to the sub root.  It's root distance is always correct.
				//Stop at the object immediately pointed from the sub root.
				apcFroms.Kill();
				return this;
			}

			pcContainer = pcPointedFrom->GetEmbeddingContainer();
			pcTemp = pcContainer->ClearDistToSubRoot();
			if (pcTemp != NULL)
			{
				pcRootSet = pcTemp;
			}
		}
	}

	apcFroms.Kill();
	return pcRootSet;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ClearStackPointersTo(void)
{
	CStackPointer*	pcStackPointer;

	pcStackPointer = mpcStackFroms;
	while (pcStackPointer)
	{
		pcStackPointer = pcStackPointer->ClearPointerGetNext();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetFlag(int iFlag, int iFlagValue)
{
	::SetFlag(&miFlags, iFlag, iFlagValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::CanFindRoot(void)
{
	ValidateNotEmbedded(__METHOD__);

	int								iNumFroms;
	int								i;
	CBaseObject**					ppcPointedFrom;
	int								iNearestRoot;
	CBaseObject*					pcNearestPointedFrom;
	int								iPointedFromDist;
	BOOL							bResult;
	CArrayEmbeddedBaseObjectPtr		apcFroms;

	if (this->IsRoot())
	{
		return TRUE;
	}

	SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, TRUE);

	apcFroms.Init();
	GetHeapFroms(&apcFroms);
	iNumFroms = apcFroms.NumElements();

	if (iNumFroms == 0)
	{
		apcFroms.Kill();
		SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
		return FALSE;
	}

	iNearestRoot = MAX_INT;
	pcNearestPointedFrom = NULL;
	ppcPointedFrom = apcFroms.GetData();
	for (i = 0; i < iNumFroms; i++)
	{
		iPointedFromDist = ppcPointedFrom[i]->miDistToRoot;
		if ((iPointedFromDist >= ROOT_DIST_TO_ROOT) && (!ppcPointedFrom[i]->TestedForRoot()))
		{
			if (iPointedFromDist < iNearestRoot)
			{
				iNearestRoot = iPointedFromDist;
				pcNearestPointedFrom = ppcPointedFrom[i];
			}
		}
	}

	apcFroms.Kill();

	if (pcNearestPointedFrom == NULL)
	{
		apcFroms.Kill();
		SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
		return FALSE;
	}

	if (pcNearestPointedFrom->IsRoot())
	{
		SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
		return TRUE;
	}

	bResult = pcNearestPointedFrom->GetEmbeddingContainer()->CanFindRoot();
	SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetExpectedDistToRoot(int iExpectedDistToRoot)
{
	ValidateNotEmbedded(__METHOD__);

	int	iBestDistToRoot;

	iBestDistToRoot = CalculateDistToRootFromPointedFroms(iExpectedDistToRoot);
	SetDistToRootAndSetPointedTosExpectedDistToRoot(iBestDistToRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetCalculatedDistToRoot(void)
{
	ValidateNotEmbedded(__METHOD__);

	int	iBestDistToRoot;

	iBestDistToRoot = CalculateDistToRootFromPointedFroms();
	SetDistToRootAndSetPointedTosExpectedDistToRoot(iBestDistToRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::CalculateDistToRootFromPointedFroms(void)
{
	int iDistToRoot;

	iDistToRoot = CalculateDistToRootFromPointedFroms(MAX_DIST_TO_ROOT);
	if (iDistToRoot != MAX_DIST_TO_ROOT)
	{
		return iDistToRoot;
	}
	else
	{
		return UNATTACHED_DIST_TO_ROOT;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::CalculateDistToRootFromPointedFroms(int iDistToRoot)
{
	int				iNumFroms;
	int				i;
	CBaseObject*	pcFrom;
	int				iBestDistToRoot;

	iBestDistToRoot = iDistToRoot;
	iNumFroms = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < iNumFroms; i++)
	{
		pcFrom = CEmbeddedObject::PrivateGetHeapFrom(i);
		if (pcFrom)
		{
			if (pcFrom->miDistToRoot < iBestDistToRoot)
			{
				if (pcFrom->miDistToRoot >= ROOT_DIST_TO_ROOT)
				{
					iBestDistToRoot = pcFrom->miDistToRoot+1;
				}
			}
		}
	}
	return iBestDistToRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::UpdateDistToRootFromPointedFroms(void)
{
	ValidateNotEmbedded(__METHOD__);

	int	iNearestRoot;

	iNearestRoot = CalculateDistToRootFromPointedFroms();
	if (iNearestRoot != UNATTACHED_DIST_TO_ROOT)
	{
		SetDistToRootAndSetPointedTosExpectedDistToRoot(iNearestRoot);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::UnattachDistToRoot(void)
{
	ValidateNotEmbedded(__METHOD__);

	SetDistToRootAndSetPointedTosExpectedDistToRoot(UNATTACHED_DIST_TO_ROOT);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::RemoveToFrom(CEmbeddedObject* pcPointedTo, CArrayEmbeddedBaseObjectPtr* papcFromsChanged)
{
	CBaseObject*	pcBaseObject;

	if (pcPointedTo)
	{
		if (pcPointedTo->IsBaseObject())
		{
			pcBaseObject = (CBaseObject*)pcPointedTo;
			if (pcBaseObject->miDistToRoot >= ROOT_DIST_TO_ROOT)
			{
				pcBaseObject->PrivateRemoveFrom(this);
				papcFromsChanged->Add(&pcBaseObject);
			}
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::SerialisedSize(void)
{
	CObjectSerialiser	cSerialiser;
	int					iLength;

	cSerialiser.Init(NULL, this);
	cSerialiser.Save();
	iLength = cSerialiser.GetLength();
	cSerialiser.Kill();
	return iLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsRoot(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsNamed(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CBaseObject::GetName(void)
{
	return "";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsSubRoot(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsInvalidated(void)
{
	return miFlags & OBJECT_FLAGS_INVALIDATED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsDirty(void)
{
	return miFlags & OBJECT_FLAGS_DIRTY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::TestedForRoot(void)
{
	return miFlags & OBJECT_FLAGS_TESTED_FOR_ROOT;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetDirty(void)
{
	miFlags |= OBJECT_FLAGS_DIRTY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::GetDistToRoot(void)
{
	return miDistToRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetObjectID(OIndex oi)
{
	moi = oi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ClearIndex(void)
{
	SetObjectID(INVALID_O_INDEX);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsUnknown(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::AddHeapFrom(CBaseObject* pcFromObject)
{
	if (pcFromObject != NULL)
	{
		mapHeapFroms.Add(&pcFromObject);
		if (pcFromObject->miDistToRoot >= ROOT_DIST_TO_ROOT)
		{
			SetExpectedDistToRoot(pcFromObject->miDistToRoot+1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsHollow(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CBaseObject::TestGetTo(int iToIndex)
{
	CEmbeddedObject**			ppTo;
	CEmbeddedObject*			pTo;
	CArrayEmbeddedObjectPtr		apcTos;

	apcTos.Init(32);
	GetTos(&apcTos);
	ppTo = apcTos.SafeGet(iToIndex);
	if (ppTo)
	{
		pTo = *ppTo;
	}
	else
	{
		pTo = NULL;
	}
	apcTos.Kill();
	return pTo;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CBaseObject::GetOI(void)
{
	return moi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetName(char* szName)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsBaseObject(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::GetNumEmbedded(void)
{
	SetFlagNumEmbedded(1);
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::GetNumEmbeddedFromFlags(void)
{
	int	iNumEmbedded;

	iNumEmbedded = (miFlags & OBJECT_FLAGS_NUM_EMBEDDED) >> 8;
	return iNumEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetFlagNumEmbedded(int iNumEmbedded)
{
	iNumEmbedded = iNumEmbedded << 8;
	miFlags &= ~OBJECT_FLAGS_NUM_EMBEDDED;
	miFlags |= iNumEmbedded & OBJECT_FLAGS_NUM_EMBEDDED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::TestGetNumEmbeddedFromFlags(void)
{
	return GetNumEmbeddedFromFlags();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObjects* CBaseObject::GetObjects(void)
{
	CBaseObject*	pcBaseObject;

	pcBaseObject = GetEmbeddingContainer();
	return pcBaseObject->mpcObjectsThisIn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsMarkedUnreachable(void)
{
	return miFlags & OBJECT_FLAGS_UNREACHABLE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::TestCanFindRoot(void)
{
	return CanFindRoot();
}

