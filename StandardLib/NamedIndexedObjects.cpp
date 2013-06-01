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
#include "NamedIndexedObjects.h"
#include "NamedObject.h"
#include "BaseLib/Logger.h"


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
	OIndex	oi;

	oi = mcNames.Get(szName);
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
BOOL CNamedIndexedObjects::RemoveIndex(OIndex oi)
{
	BOOL	bResult;

	if (oi != INVALID_O_INDEX)
	{
		//This only removes the index from the indexes, it does not free the object pointer to.
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
	BOOL	bResult;

	if ((szName != NULL) && (szName[0] != 0))
	{
		//This only removes the name from the names, it does not free the object pointer to.
		bResult = mcNames.Remove(szName);
		return bResult;
	}
	return TRUE;
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
	CNamedObject*	pcNamed;
	BOOL			bResult;
	int				iResult;

	if (mcNames.Contains(szName))
	{
		gcLogger.Error2("CNamedIndexedObjects::AddWithIDAndName cannot add object named [", szName, "].  It already exists.", NULL);
		return FALSE;
	}

	if (!mcNames.IsOnlyValidCharacters(szName))
	{
		gcLogger.Error2("CNamedIndexedObjects::AddWithIDAndName cannot add object named [", szName, "].  It's name contains invalid characters.", NULL);
		return FALSE;
	}

	bResult = AddWithID(pvObject, oi);
	if (!bResult)
	{
		char sz[32];

		gcLogger.Error2("CNamedIndexedObjects::AddWithIDAndName cannot add object named [", szName, "].  An index [", IToA(oi, sz, 10), "] already exists.", NULL);
		return FALSE;
	}

	pcNamed = (CNamedObject*)pvObject;
	bResult = pcNamed->InitName(szName);

	if ((szName != NULL) && (szName[0] != 0))
	{
		iResult = mcNames.Add(pcNamed->GetOI(), szName);

		//This should never happen.  The checks at the top cut it out.
		if (iResult == -1)
		{
			char sz[32];

			gcLogger.Error2("CNamedIndexedObjects::AddWithIDAndName cannot add object named [", szName, "] and index [", IToA(oi, sz, 10), "].  It broke unexpectedly", NULL);
			bResult = FALSE;
		}
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

