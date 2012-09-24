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
	//This should only be called once all the 'from s' and 'to s' are gone.

	mapFroms.Kill();
	if (mpcObjectsThisIn)
	{
		mpcObjectsThisIn->RemoveInKill(this);
	}
	moi = INVALID_O_INDEX;
	CUnknown::Kill();
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
	CArrayEmbeddedBaseObjectPtr		apcFromsChanged;
	CBaseObject*					pcTemp;

	PrivateRemoveFrom(pcFrom);
	if (!CanFindRoot())
	{
		KillThisGraph();
	}
	else
	{
		apcFromsChanged.Init();
		pcTemp = this;
		apcFromsChanged.Add(&pcTemp);
		FixDistToRoot(&apcFromsChanged);
		apcFromsChanged.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::CopyFroms(CBaseObject* pcSource)
{
	RemoveAllFroms();
	mapFroms.Copy(&pcSource->mapFroms);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::KillThisGraph(void)
{
	CArrayBaseObjectPtr		apcKilled;
	int						iNumKilled;
	
	apcKilled.Init(1024);
	CollectedThoseToBeKilled(&apcKilled);

	iNumKilled = KillCollected(&apcKilled);

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
int CBaseObject::KillCollected(CArrayBaseObjectPtr* papcKilled)
{
	int								i;
	int								iNumElements;
	CBaseObject*					pcKilled;
	CArrayEmbeddedBaseObjectPtr		apcFromsChanged;

	iNumElements = papcKilled->NumElements();
	apcFromsChanged.Init();

	for (i = 0; i < iNumElements; i++)
	{
		pcKilled = *papcKilled->Get(i);
		pcKilled->RemoveAllTos(&apcFromsChanged);
	}

	for (i = 0; i < iNumElements; i++)
	{
		pcKilled = *papcKilled->Get(i);
		pcKilled->Kill();
	}

	FixDistToRoot(&apcFromsChanged);

	apcFromsChanged.Kill();
	return iNumElements;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CBaseObject::ClearDistToSubRoot(void)
{
	int				i;
	int				iNumFroms;
	CBaseObject**	ppcPointedFrom;
	CBaseObject*	pcPointedFrom;
	CBaseObject*	pcRootSet;
	CBaseObject*	pcTemp;

	miDistToRoot = UNATTACHED_DIST_TO_ROOT;
	pcRootSet = NULL;

	iNumFroms = mapFroms.NumElements();
	ppcPointedFrom = mapFroms.GetData();
	for (i = 0; i < iNumFroms; i++)
	{
		pcPointedFrom = ppcPointedFrom[i];
		if (pcPointedFrom->miDistToRoot >= ROOT_DIST_TO_ROOT)
		{
			if (pcPointedFrom->IsSubRoot())
			{
				//No point in stepping back to the sub root.  It's root distance is always correct.
				//Stop at the object immediately pointed from the sub root.
				return this;
			}

			pcTemp = pcPointedFrom->ClearDistToSubRoot();
			if (pcTemp != NULL)
			{
				pcRootSet = pcTemp;
			}
		}
	}
	return pcRootSet;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::CanFindRoot(void)
{
	int				iNumFroms;
	int				i;
	CBaseObject**	ppcPointedFrom;
	int				iNearestRoot;
	CBaseObject*	pcNearestPointedFrom;
	int				iPointedFromDist;
	BOOL			bResult;

	SetFlag(&miFlags, OBJECT_FLAGS_TESTED_FOR_ROOT, TRUE);

	iNumFroms = mapFroms.NumElements();
	if (iNumFroms == 0)
	{
		SetFlag(&miFlags, OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
		return FALSE;
	}

	iNearestRoot = MAX_INT;
	pcNearestPointedFrom = NULL;
	ppcPointedFrom = mapFroms.GetData();
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

	if (pcNearestPointedFrom == NULL)
	{
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
void CBaseObject::FixDistToRoot(CArrayEmbeddedBaseObjectPtr* papcFromsChanged)
{
	int				i;
	int				iNumElements;
	CBaseObject*	pcSubRoot;
	CBaseObject*	pcTemp;
	CBaseObject*	pcFromsChanged;

	//You could optimize this to stop one before the RootSet.
	pcSubRoot = NULL;
	iNumElements = papcFromsChanged->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcFromsChanged = *papcFromsChanged->Get(i);
		pcTemp = pcFromsChanged->ClearDistToSubRoot();
		if (!pcSubRoot)
		{
			//Theres a bug here, you need to collect all the SubRoots and call FixDistToRoot on all of them.
			pcSubRoot = pcTemp;
		}
	}

	if (pcSubRoot)
	{
		pcSubRoot->FixDistToRoot();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::FixDistToRoot(void)
{
	int				i;
	CBaseObject**	ppcPointedFrom;
	int				iNearestRoot;
	int				iNumElements;

	iNearestRoot = MAX_INT;
	ppcPointedFrom = mapFroms.GetData();
	iNumElements = mapFroms.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		if (ppcPointedFrom[i]->miDistToRoot < iNearestRoot)
		{
			iNearestRoot = ppcPointedFrom[i]->miDistToRoot;
		}
	}

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
BOOL CBaseObject::HasFroms(void)
{
	return mapFroms.IsNotEmpty();
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
BOOL CBaseObject::RemoveToFrom(CBaseObject* pcPointedTo, CArrayEmbeddedBaseObjectPtr* papcFromsChanged)
{
	if (pcPointedTo)
	{
		if (pcPointedTo->miDistToRoot >= ROOT_DIST_TO_ROOT)
		{
			pcPointedTo->PrivateRemoveFrom(this);
			papcFromsChanged->Add(&pcPointedTo);
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
	}
	else
	{
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
		if (pcTos->miDistToRoot != iBestDistToRoot)
		{
			pcTos->SetDistToRoot(iBestDistToRoot);
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
CBaseObject* CBaseObject::TestGetTo(int iToIndex)
{
	CBaseObject**		ppTo;
	CBaseObject*		pTo;
	CArrayBaseObjectPtr	apcTos;

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
void CBaseObject::RemoveAllFroms(void)
{
	mapFroms.Reinit();
}

