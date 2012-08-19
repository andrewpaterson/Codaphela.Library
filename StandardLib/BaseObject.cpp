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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject::CBaseObject()
{
	mapFroms.Init();
	miDistToRoot = -1;
	moi = INVALID_O_INDEX;
	miFlags = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Kill(void)
{
	//This should only be called once all the 'froms' and 'tos' are gone.

	mapFroms.Kill();
	moi = INVALID_O_INDEX;
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::RemoveFrom(CBaseObject* pcFrom)
{
	mapFroms.Remove(&pcFrom, FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::RemoveEmbeddedFrom(CBaseObject* pcFrom)
{
	CArrayEmbeddedBaseObjectPtr		apcFromsChanged;
	CBaseObject*					pcTemp;

	mapFroms.Remove(&pcFrom, FALSE);
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
void CBaseObject::KillThisGraph(void)
{
	CArrayBaseObjectPtr	apcKilled;
	
	apcKilled.Init(1024);
	CollectedThoseToBeKilled(&apcKilled);

	KillCollected(&apcKilled);

	apcKilled.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::MarkForKilling(CArrayBaseObjectPtr* papcKilled)
{
	CBaseObject*		pcTemp;

	miDistToRoot = -1;
	pcTemp = this;
	papcKilled->Add(&pcTemp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::KillCollected(CArrayBaseObjectPtr* papcKilled)
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

	miDistToRoot = -1;
	pcRootSet = NULL;

	iNumFroms = mapFroms.NumElements();
	ppcPointedFrom = mapFroms.GetData();
	for (i = 0; i < iNumFroms; i++)
	{
		pcPointedFrom = ppcPointedFrom[i];
		if (pcPointedFrom->miDistToRoot != -1)
		{
			if (pcPointedFrom->IsSubRoot())
			{
				return this;
			}

			pcTemp = pcPointedFrom->ClearDistToSubRoot();
			if (pcTemp != NULL)
			{
				pcRootSet = pcTemp;
			}
		}
	}
	return pcRootSet;;
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
		if ((iPointedFromDist != -1) && (!ppcPointedFrom[i]->TestedForRoot()))
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
		if (pcFrom->miDistToRoot != -1)
		{
			if ((miDistToRoot == -1) || (pcFrom->miDistToRoot < miDistToRoot-1))
			{
				SetDistToRoot(pcFrom->miDistToRoot+1);
			}
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
	if ((pcTos->miDistToRoot == -1) || (pcTos->miDistToRoot > iExpectedDistToRoot))
	{
		pcTos->SetDistToRoot(iExpectedDistToRoot);
	}
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

