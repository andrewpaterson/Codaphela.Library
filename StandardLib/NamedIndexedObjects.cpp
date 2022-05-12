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
CBaseObject* CNamedIndexedObjects::Get(char* szObjectName, OIndex oi)
{
	CBaseObject* pvObject;

	pvObject = Get(szObjectName);
	if (pvObject)
	{
		if (pvObject->GetIndex() != oi)
		{
			gcLogger.Error2(__METHOD__, " Cannot get object named [", szObjectName, "].  Mismatch expected index [", IndexToString(oi), "] with object index [", IndexToString(pvObject->GetIndex()), "].", NULL);
		}
		return pvObject;
	}

	pvObject = Get(oi);
	if (pvObject)
	{
		if (StringCompare(pvObject->GetName(), szObjectName) != 0)
		{
			gcLogger.Error2(__METHOD__, " Cannot get object with index [", IndexToString(oi), "].  Mismatch expected name [", szObjectName, "] with object name [", pvObject->GetName(), "].", NULL);
		}
		return pvObject;
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
BOOL CNamedIndexedObjects::Has(OIndex oi)
{
	return mcMemoryIndexedObjects.Has(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedObjects::Has(char* szName)
{
	CIndexTreeMemoryAccess	cAccess;
	BOOL					bHasObject;

	cAccess.Init(&mcMemoryNames);
	bHasObject = cAccess.HasString(szName);
	cAccess.Kill();
	return bHasObject;
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
BOOL CNamedIndexedObjects::AddUnitialisedIntoMemoryWithIndex(CBaseObject* pvObject)
{
	BOOL	bResult;
	OIndex	oi;
	BOOL	bHasObject;

	oi = pvObject->GetIndex();
	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot add object with an invalid index [", IndexToString(oi), " ].", NULL);
	}

	bHasObject = Has(oi);
	if (bHasObject)
	{
		return gcLogger.Error2(__METHOD__, " Cannot add object with index[", IndexToString(oi), "].  It already exists.", NULL);
	}

	bResult = mcMemoryIndexedObjects.Add(oi, pvObject);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedObjects::AddUnitialisedIntoMemoryWithNameAndIndex(CBaseObject* pvObject)
{
	BOOL					bResult;
	BOOL					bHasObject;
	OIndex					oi;
	char*					szName;
	CIndexTreeMemoryAccess	cAccess;

	oi = pvObject->GetIndex();
	szName = pvObject->GetName();
	if (StrEmpty(szName))
	{
		return gcLogger.Error2(__METHOD__, " Cannot add object with an empty name.", NULL);
	}

	if (!IsValidIndex(oi))
	{
		return gcLogger.Error2(__METHOD__, " Cannot add object named [", szName, "] with invalid index [", IndexToString(oi), " ].", NULL);
	}


	bHasObject = Has(szName);
	if (bHasObject)
	{
		return gcLogger.Error2(__METHOD__, " Cannot add object named [", szName, "].  It already exists.", NULL);
	}

	bHasObject = Has(oi);
	if (bHasObject)
	{
		return gcLogger.Error2(__METHOD__, " Cannot add object with index[", IndexToString(oi), "].  It already exists.", NULL);
	}

	bResult = mcMemoryIndexedObjects.Add(oi, pvObject);
	if (!bResult)
	{
		return FALSE;
	}
	
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
CIndexTreeMemory* CNamedIndexedObjects::GetNames(void)
{
	return &mcMemoryNames;
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedObjects::FreeObjects(void)
{
	SIndexesIterator	sIter;
	OIndex				oi;
	CBaseObject*		pcObject;

	oi = mcMemoryIndexedObjects.StartIteration(&sIter);
	while (oi != INVALID_O_INDEX)
	{
		pcObject = Get(oi);
		pcObject->FreeInternal();
		oi = mcMemoryIndexedObjects.Iterate(&sIter);
	}
	return TRUE;
}

