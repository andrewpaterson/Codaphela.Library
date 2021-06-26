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
#include "BaseLib/IndexTreeMemoryAccess.h"
#include "NamedIndexedObjects.h"
#include "Object.h"
#include "HollowObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedObjects::Init(void)
{
	mcMemoryNames.Init();
	mcMemoryIndexedObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedObjects::Kill(void)
{
	mcMemoryIndexedObjects.Kill();
	mcMemoryNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedObjects::ReInit(void)
{
	Kill();
	Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CNamedIndexedObjects::Get(OIndex oi)
{
	return (CBaseObject*)mcMemoryIndexedObjects.Get(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CNamedIndexedObjects::Get(char* szName)
{
	OIndex					oi;
	CIndexTreeMemoryAccess	cAccess;
	BOOL					bExists;

	cAccess.Init(&mcMemoryNames);
	bExists = cAccess.GetStringData(szName, &oi);
	cAccess.Kill();
	if (bExists)
	{
		return Get(oi);
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
BOOL CNamedIndexedObjects::RemoveIndex(OIndex oi)
{
	BOOL	bResult;

	if (oi != INVALID_O_INDEX)
	{
		//This only removes the index from the indexes, it does not free the object pointer too.
		bResult = mcMemoryIndexedObjects.Remove(oi);
		return bResult;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedObjects::RemoveName(char* szName)
{
	CIndexTreeMemoryAccess	cAccess;
	BOOL					bResult;

	//This only removes the name from the names, it does not free the object pointer to.
	cAccess.Init(&mcMemoryNames);
	bResult = cAccess.DeleteString(szName);
	cAccess.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedObjects::AddWithID(CBaseObject* pvObject, OIndex oi)
{
	BOOL				bResult;

	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot add object with an invalid index [", LongLongToString(oi), " ].", NULL);
	}

	bResult = mcMemoryIndexedObjects.Add(oi, pvObject);
	if (bResult)
	{
		pvObject->SetIndex(oi);
		bResult = pvObject->InitName("");
		return bResult;
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
BOOL CNamedIndexedObjects::AddUnitialisedIntoMemoryWithIndexAndName(CBaseObject* pvObject, OIndex oi, char* szName)
{
	BOOL					bResult;
	CIndexTreeMemoryAccess	cAccess;
	BOOL					bHasObject;

	if (StrEmpty(szName))
	{
		return gcLogger.Error2(__METHOD__, " Cannot add object with an empty name.", NULL);
	}

	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot add object with name [", szName, "] and invalid index [", LongLongToString(oi), " ].", NULL);
	}


	cAccess.Init(&mcMemoryNames);
	bHasObject = cAccess.HasString(szName);
	cAccess.Kill();
	if (bHasObject)
	{
		return gcLogger.Error2(__METHOD__, " Cannot add object named [", szName, "].  It already exists.", NULL);
	}

	bResult = AddWithID(pvObject, oi);
	if (!bResult)
	{
		char sz[32];

		return gcLogger.Error2(__METHOD__, " Cannot add object named [", szName, "].  An index [", IToA(oi, sz, 10), "] already exists.", NULL);
	}

	bResult = pvObject->InitName(szName);

	cAccess.Init(&mcMemoryNames);
	szName = (char*)cAccess.PutStringLong(szName, oi);
	bResult = szName != NULL;
	cAccess.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedObjects::ReplaceWithIDAndName(CBaseObject* pvObject, char* szExistingName, OIndex oi)
{
	BOOL					bResult;
	CIndexTreeMemoryAccess	cAccess;

	//This does nothing meaningful.  The old object still exists believing it is named szExistingName too.
	bResult = AddWithID(pvObject, oi);
	if (!bResult)
	{
		char sz[32];

		gcLogger.Error2(__METHOD__, " Cannot add object named [", szExistingName, "].  An index [", IToA(oi, sz, 10), "] already exists.", NULL);
		return FALSE;
	}

	bResult = pvObject->SetName(szExistingName);

	if (!StrEmpty(szExistingName))
	{
		cAccess.Init(&mcMemoryNames);
		oi = pvObject->GetIndex();
		szExistingName = (char*)cAccess.PutStringLong(szExistingName, oi);
		bResult = szExistingName != NULL;
		cAccess.Kill();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexedObjects::NumIndexed(void)
{
	return mcMemoryIndexedObjects.NumIndexed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNamedIndexedObjects::NumNames(void)
{
	return mcMemoryNames.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedObjects* CNamedIndexedObjects::GetObjects(void)
{
	return &mcMemoryIndexedObjects;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CNamedIndexedObjects::StartIteration(SIndexesIterator* psIter)
{
	OIndex		oi;

	oi = mcMemoryIndexedObjects.StartIteration(psIter);
	if (oi != INVALID_O_INDEX)
	{
		return Get(oi);
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
CBaseObject* CNamedIndexedObjects::Iterate(SIndexesIterator* psIter)
{
	OIndex		oi;

	oi = mcMemoryIndexedObjects.Iterate(psIter);
	if (oi != INVALID_O_INDEX)
	{
		return Get(oi);
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
BOOL CNamedIndexedObjects::ValidateNoDirty(void)
{
	SIndexesIterator	sIter;
	OIndex				oi;
	CBaseObject*		pcObject;
	BOOL				bDirty;
	CChars				szIdentifier;
	BOOL				bCanFindRoot;

	oi = mcMemoryIndexedObjects.StartIteration(&sIter);
	while (oi != INVALID_O_INDEX)
	{
		pcObject = Get(oi);
		bDirty = pcObject->IsDirty();
		bCanFindRoot = pcObject->GetDistToRoot() != UNATTACHED_DIST_TO_ROOT;
		if (bDirty && bCanFindRoot)
		{
			szIdentifier.Init();
			pcObject->GetIdentifier(&szIdentifier);
			return gcLogger.Error2(__METHOD__, " Object [", szIdentifier.Text(), "] is dirty.", NULL);
		}
		oi = mcMemoryIndexedObjects.Iterate(&sIter);
	}
	return TRUE;
}

