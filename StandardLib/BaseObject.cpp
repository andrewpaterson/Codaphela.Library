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
#include "BaseLib/Logger.h"
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
	CBaseObject*					pcContainer;
	int								iNumFroms;

	//This method is for the user to forcibly kill an object.
	//It is not called internally.  KillThisGraph is method used to free objects that are unattached.

	//Note this does NOT call CUnknown::Kill.  That is handled elsewhere.

	if (IsEmbedded())
	{
		//Kill should not be called on an embedded object.
		pcContainer = GetEmbeddingContainer();
		gcLogger.Error2("CBaseObject::Kill object of class [", ClassName(), "] called Kill but is embedded in object with index [", IndexToString(pcContainer->GetOI()),"] of class [", pcContainer->ClassName(), "].", NULL);

		return;
	}

	iNumFroms = NumHeapFroms();
	if (iNumFroms == 0)
	{
		apcFromsChanged.Init();
		RemoveAllTos(&apcFromsChanged);
		mpcObjectsThisIn->FixDistToRoot(&apcFromsChanged);
		apcFromsChanged.Kill();
		iNumKilled = KillThisGraph();
	}
	else
	{
		iNumKilled = RemoveAllFroms();
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::KillDontFree(void)
{
	KillData();
	KillFroms();
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
void CBaseObject::RemoveFrom(CBaseObject* pcFrom)
{
	//Removing a 'from' kicks off memory reclamation.  This is the entry point for memory management.
	PrivateRemoveFrom(pcFrom);

	TryKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::TryKill(void)
{
	CBaseObject*	pcContainer;

	pcContainer = GetEmbeddingContainer();
	if (!CanFindRoot() && !HasStackPointers())
	{	
		pcContainer->KillThisGraph();
	}
	else
	{
		//ClearDistToSubRoot();
		pcContainer->FixDistToRoot();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::RecurseSetDistToRoot(int iDistToRoot)
{
	miDistToRoot = iDistToRoot;
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

	if (CanFindRoot())
	{
		return 0;
	}

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
	if (pcPointedTo)
	{
		if (pcPointedTo->miDistToRoot != UNATTACHED_DIST_TO_ROOT)
		{
			if (!pcPointedTo->CanFindRoot())
			{
				pcPointedTo->CollectThoseToBeKilled(papcKilled);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::MarkForKilling(CArrayBaseObjectPtr* papcKilled)
{
	CBaseObject*		pcTemp;

	RecurseSetDistToRoot(UNATTACHED_DIST_TO_ROOT);
	pcTemp = this;
	papcKilled->Add(&pcTemp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::CollectThoseToBeKilled(CArrayBaseObjectPtr* papcKilled)
{
	CBaseObject*		pcContainer;

	pcContainer = GetEmbeddingContainer();
	pcContainer->ContainerCollectThoseToBeKilled(papcKilled);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ContainerCollectThoseToBeKilled(CArrayBaseObjectPtr* papcKilled)
{
	//This method will never be called on an embedded object.
	//Only the containing object can ContainerCollectThoseToBeKilled.

	MarkForKilling(papcKilled);
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
CBaseObject* CBaseObject::ClearDistToSubRoot(void)
{
	int								i;
	int								iNumFroms;
	CBaseObject**					ppcPointedFrom;
	CBaseObject*					pcPointedFrom;
	CBaseObject*					pcRootSet;
	CBaseObject*					pcTemp;
	CArrayEmbeddedBaseObjectPtr		apcFroms;

	miDistToRoot = UNATTACHED_DIST_TO_ROOT;
	pcRootSet = NULL;

	apcFroms.Init();
	GetFroms(&apcFroms);
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

			pcTemp = pcPointedFrom->ClearDistToSubRoot();
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
void CBaseObject::SetFlagEmbedded(int iFlag, int iFlagValue)
{
	CBaseObject*	pcContainer;

	pcContainer = GetEmbeddingContainer();
	pcContainer->RecurseSetFlagEmbedded(iFlag, iFlagValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::RecurseSetFlagEmbedded(int iFlag, int iFlagValue)
{
	SetFlag(&miFlags, iFlag, iFlagValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::CanFindRoot(void)
{
	int								iNumFroms;
	int								i;
	CBaseObject**					ppcPointedFrom;
	int								iNearestRoot;
	CBaseObject*					pcNearestPointedFrom;
	int								iPointedFromDist;
	BOOL							bResult;
	CArrayEmbeddedBaseObjectPtr		apcFroms;

	SetFlagEmbedded(OBJECT_FLAGS_TESTED_FOR_ROOT, TRUE);

	apcFroms.Init();
	GetFroms(&apcFroms);
	iNumFroms = apcFroms.NumElements();

	if (iNumFroms == 0)
	{
		apcFroms.Kill();
		SetFlagEmbedded(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
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
		SetFlagEmbedded(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
		return FALSE;
	}

	if (pcNearestPointedFrom->IsRoot())
	{
		SetFlagEmbedded(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
		return TRUE;
	}

	bResult = pcNearestPointedFrom->CanFindRoot();
	SetFlagEmbedded(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::FixDistToRoot(void)
{
	int								i;
	CBaseObject**					ppcPointedFrom;
	int								iNearestRoot;
	int								iNumFroms;
	int								iDistToRoot;
	CArrayEmbeddedBaseObjectPtr		apcFroms;

	iNearestRoot = MAX_INT;

	apcFroms.Init();
	GetFroms(&apcFroms);

	iNumFroms = apcFroms.NumElements();
	ppcPointedFrom = apcFroms.GetData();

	for (i = 0; i < iNumFroms; i++)
	{
		iDistToRoot = ppcPointedFrom[i]->miDistToRoot;
		if (iDistToRoot != UNATTACHED_DIST_TO_ROOT)
		{
			if (iDistToRoot < iNearestRoot)
			{
				iNearestRoot = iDistToRoot;
			}
		}
	}

	apcFroms.Kill();

	if (iNumFroms > 0)
	{
		if (iNearestRoot+1 != miDistToRoot)
		{
			SetDistToRoot(iNearestRoot+1);
		}
	}
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
	CBaseObject*	pcContainer;
	
	pcContainer = GetEmbeddingContainer();
	return pcContainer->IsEmbeddedDirty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsEmbeddedDirty(void)
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
int CBaseObject::DistToRoot(void)
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
void CBaseObject::AddFrom(CBaseObject* pcFrom)
{
	if (pcFrom != NULL)
	{
		mapHeapFroms.Add(&pcFrom);
		if (pcFrom->miDistToRoot >= ROOT_DIST_TO_ROOT)
		{
			PotentiallySetDistToRoot(this, pcFrom->miDistToRoot+1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PotentiallySetDistToRoot(CBaseObject* pcTo, int iExpectedDistToRoot)
{
	int				iNumFroms;
	int				i;
	CBaseObject*	pcFrom;
	int				iBestDistToRoot;

	if ((pcTo->miDistToRoot == CLEARED_DIST_TO_ROOT) || (pcTo->miDistToRoot == UNATTACHED_DIST_TO_ROOT))
	{
		pcTo->SetDistToRoot(iExpectedDistToRoot);
		return;
	}

	iBestDistToRoot = iExpectedDistToRoot;
	iNumFroms = pcTo->CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < iNumFroms; i++)
	{
		pcFrom = pcTo->PrivateGetFrom(i);
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
	pcTo->SetDistToRoot(iBestDistToRoot);
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
int CBaseObject::RemoveAllFroms(void)
{
	CEmbeddedObject::RemoveAllFroms();
	return KillThisGraph();
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
int CBaseObject::RecurseNumHeapFroms(void)
{
	return CEmbeddedObject::NumHeapFroms();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::RecurseNumStackFroms(void)
{
	return CEmbeddedObject::NumStackFroms();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::RecurseNumTotalFroms(void)
{
	return CEmbeddedObject::NumTotalFroms();
}

