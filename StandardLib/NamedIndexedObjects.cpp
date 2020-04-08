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
#include "NamedObject.h"
#include "NamedHollowObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedObjects::Init(void)
{
	mcNames.Init();
	mcIndexedObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedObjects::Kill(void)
{
	mcIndexedObjects.Kill();
	mcNames.Kill();
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
	return (CBaseObject*)mcIndexedObjects.Get(oi);
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

	cAccess.Init(&mcNames);
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
		bResult = mcIndexedObjects.Remove(oi);
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
	cAccess.Init(&mcNames);
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
	BOOL	bResult;

	bResult = mcIndexedObjects.Add(oi, pvObject);
	if (bResult)
	{
		pvObject->SetObjectID(oi);
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
BOOL CNamedIndexedObjects::AddWithIDAndName(CBaseObject* pvObject, OIndex oi, char* szName)
{
	CNamedObject*			pcNamed;
	CNamedHollowObject*		pcNamedHollow;
	BOOL					bResult;
	CIndexTreeMemoryAccess	cAccess;
	BOOL					bHasObject;

	cAccess.Init(&mcNames);
	bHasObject = cAccess.HasString(szName);
	cAccess.Kill();
	if (bHasObject)
	{
		
		gcLogger.Error2(__METHOD__, " Cannot add object named [", szName, "].  It already exists.", NULL);
		return FALSE;
	}

	bResult = AddWithID(pvObject, oi);
	if (!bResult)
	{
		char sz[32];

		gcLogger.Error2(__METHOD__, " Cannot add object named [", szName, "].  An index [", IToA(oi, sz, 10), "] already exists.", NULL);
		return FALSE;
	}

	if (!pvObject->IsHollow())
	{
		pcNamed = (CNamedObject*)pvObject;
		bResult = pcNamed->InitName(szName);
	}
	else
	{
		pcNamedHollow = (CNamedHollowObject*)pvObject;
		bResult = pcNamedHollow->InitName(szName);
	}

	if ((szName != NULL) && (szName[0] != 0))
	{
		cAccess.Init(&mcNames);
		oi = pvObject->GetOI();
		szName = (char*)cAccess.PutStringLong(szName, oi);
		bResult = szName != NULL;
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
	return mcIndexedObjects.NumIndexed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CNamedIndexedObjects::NumNames(void)
{
	return mcNames.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexedObjects* CNamedIndexedObjects::GetObjects(void)
{
	return &mcIndexedObjects;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CNamedIndexedObjects::StartIteration(SIndexesIterator* psIter)
{
	OIndex		oi;

	oi = mcIndexedObjects.StartIteration(psIter);
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

	oi = mcIndexedObjects.Iterate(psIter);
	if (oi != INVALID_O_INDEX)
	{
		return Get(oi);
	}
	else
	{
		return NULL;
	}
}

