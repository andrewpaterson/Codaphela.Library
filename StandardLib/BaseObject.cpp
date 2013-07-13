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
	mapFroms.Init();
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Kill(void)
{
	CArrayEmbeddedBaseObjectPtr		apcFromsChanged;
	int								iNumKilled;

	if (IsUnattached())
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
	mapFroms.Kill();
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
void CBaseObject::PrivateRemoveFrom(CBaseObject* pcFrom)
{
	mapFroms.Remove(&pcFrom, FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::RemoveFrom(CBaseObject* pcFrom)
{
	//Removing a 'from' kicks off memory reclamation.  This is the entry point for memory management.
	PrivateRemoveFrom(pcFrom);
	if (!CanFindRoot())
	{	
		KillThisGraph();
	}
	else
	{
		//ClearDistToSubRoot();
		FixDistToRoot();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::CopyFroms(CBaseObject* pcSource)
{
	mapFroms.ReInit();
	mapFroms.Copy(&pcSource->mapFroms);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::GetFroms(CArrayEmbeddedBaseObjectPtr* papcFroms)
{
	CBaseObject*	pcNotEmbedded;

	pcNotEmbedded = GetEmbeddingContainer();
	pcNotEmbedded->RecurseGetFroms(papcFroms);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::RecurseGetFroms(CArrayEmbeddedBaseObjectPtr* papcFroms)
{
	papcFroms->Copy((CArrayEmbeddedBaseObjectPtr*)&mapFroms);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::KillThisGraph(void)
{
	CArrayBaseObjectPtr		apcKilled;
	int						iNumKilled;

	if (CanFindRoot())
	{
		return 0;
	}

	apcKilled.Init(1024);

	if (mpcObjectsThisIn)
	{
		CollectThoseToBeKilled(&apcKilled);
		KillCollected(&apcKilled);
	}

	iNumKilled = apcKilled.NumElements();
	apcKilled.Kill();
	return iNumKilled;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::MarkForKilling(CArrayBaseObjectPtr* papcKilled)
{
	CBaseObject*		pcTemp;

	miDistToRoot = UNATTACHED_DIST_TO_ROOT;
	pcTemp = this;
	papcKilled->Add(&pcTemp);
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

	SetFlag(&miFlags, OBJECT_FLAGS_TESTED_FOR_ROOT, TRUE);

	apcFroms.Init();
	GetFroms(&apcFroms);
	iNumFroms = apcFroms.NumElements();

	if (iNumFroms == 0)
	{
		apcFroms.Kill();
		SetFlag(&miFlags, OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
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
		SetFlag(&miFlags, OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
		return FALSE;
	}

	if (pcNearestPointedFrom->IsRoot())
	{
		SetFlag(&miFlags, OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
		return TRUE;
	}

	bResult = pcNearestPointedFrom->CanFindRoot();
	SetFlag(&miFlags, OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);

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

	if (iNearestRoot+1 != miDistToRoot)
	{
		SetDistToRoot(iNearestRoot+1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::AddFrom(CBaseObject* pcFrom)
{
	if (pcFrom != NULL)
	{
		mapFroms.Add(&pcFrom);
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
int CBaseObject::NumFroms(void)
{
	return mapFroms.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CBaseObject::GetFrom(int iFrom)
{
	CBaseObject**	ppFrom;

	ppFrom = mapFroms.Get(iFrom);
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
CBaseObject* CBaseObject::TestGetFrom(int iFromIndex)
{
	return GetFrom(iFromIndex);
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
void CBaseObject::PotentiallySetDistToRoot(CBaseObject* pcTos, int iExpectedDistToRoot)
{
	int				iNumFroms;
	int				i;
	CBaseObject*	pcFrom;
	int				iBestDistToRoot;

	if ((pcTos->miDistToRoot == CLEARED_DIST_TO_ROOT) || (pcTos->miDistToRoot == UNATTACHED_DIST_TO_ROOT))
	{
		pcTos->SetDistToRoot(iExpectedDistToRoot);
		return;
	}

	iBestDistToRoot = iExpectedDistToRoot;
	iNumFroms = pcTos->mapFroms.NumElements();
	for (i = 0; i < iNumFroms; i++)
	{
		pcFrom = pcTos->GetFrom(i);
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
	pcTos->SetDistToRoot(iBestDistToRoot);
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
	int				iNumFroms;
	CBaseObject**	ppcPointedFrom;
	int				i;
	CBaseObject*	pcPointedFrom;

	iNumFroms = mapFroms.NumElements();
	ppcPointedFrom = mapFroms.GetData();
	for (i = 0; i < iNumFroms; i++)
	{
		pcPointedFrom = ppcPointedFrom[i];
		pcPointedFrom->RemoveTo(this);
	}

	mapFroms.ReInit();
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
BOOL CBaseObject::IsUnattached(void)
{
	if (mapFroms.IsEmpty() && IsNotEmbedded())
	{
		return TRUE;
	}
	return FALSE;
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
	SetNumEmbeddedFlag(1);
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
void CBaseObject::SetNumEmbeddedFlag(int iNumEmbedded)
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

