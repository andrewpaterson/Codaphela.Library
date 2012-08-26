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
#include "BaseObject.h"
#include "NamedObject.h"
#include "Objects.h"


CObjects gcObjects;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Init(CUnknowns* pcUnknownsAllocatingFrom, char* szWorkingDirectory)
{
	mpcUnknownsAllocatingFrom = pcUnknownsAllocatingFrom;
	moiNext = FIRST_O_INDEX;

	CIndexedConfig	cConfig;

	cConfig.OptimiseForStreaming(szWorkingDirectory);
	mcDatabase.Init(&cConfig);
	mcMemory.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Kill(void)
{
	mcMemory.Kill();
	mcDatabase.Kill();
	moiNext = INVALID_O_INDEX;
	mpcUnknownsAllocatingFrom = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::StepNextObjectID(void)
{
	moiNext++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::AddWithID(CBaseObject* pvObject)
{
	mcMemory.AddWithID(pvObject, moiNext);
	StepNextObjectID();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::AddWithIDAndName(CBaseObject* pvObject, char* szObjectName)
{
	mcMemory.AddWithIDAndName(pvObject, moiNext, szObjectName);
	StepNextObjectID();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::Add(char* szClassName, OIndex oi)
{
	CBaseObject*	pvObject;

	pvObject = (CBaseObject*)mpcUnknownsAllocatingFrom->Add(szClassName);
	if (pvObject)
	{
		if (!pvObject->IsNamed())
		{
			CPointerObject	pObject;

			mcMemory.AddWithID(pvObject, oi);
			pObject.mpcObject = pvObject;
			return pObject;
		}
		else
		{
			pvObject->Kill();
			return ONull;
		}
	}
	else
	{
		return ONull;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::Add(char* szClassName, char* szObjectName, OIndex oi)
{
	CBaseObject*	pvObject;

	pvObject = (CBaseObject*)mpcUnknownsAllocatingFrom->Add(szClassName);
	if (pvObject)
	{
		if (pvObject->IsNamed())
		{
			CPointerObject	pObject;

			mcMemory.AddWithIDAndName(pvObject, oi, szObjectName);
			pObject.mpcObject = pvObject;
			return pObject;
		}
		else
		{
			pvObject->Kill();
			return ONull;
		}
	}
	else
	{
		return ONull;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::Add(char* szClassName, char* szObjectName)
{
	CBaseObject*	pvObject;

	pvObject = (CBaseObject*)mpcUnknownsAllocatingFrom->Add(szClassName);
	if (pvObject)
	{
		if (pvObject->IsNamed())
		{
			CPointerObject	pObject;

			mcMemory.AddWithIDAndName(pvObject, moiNext, szObjectName);
			StepNextObjectID();
			pObject.mpcObject = pvObject;
			return pObject;
		}
		else
		{
			pvObject->Kill();
			return ONull;
		}
	}
	else
	{
		return ONull;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::Get(OIndex oi)
{
	CBaseObject*	pvObject;

	pvObject = mcMemory.Get(oi);
	if (pvObject)
	{
		CPointerObject		pObject;

		pObject.mpcObject = pvObject;
		return pObject;
	}
	else
	{
		return Null();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::Get(char* szObjectName)
{
	CBaseObject*	pvObject;

	pvObject = mcMemory.Get(szObjectName);
	if (pvObject)
	{
		CPointerObject		pObject;

		pObject.mpcObject = pvObject;
		return pObject;
	}
	else
	{
		return Null();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::Null(void)
{
	CPointerObject		pvObject;
	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CObjects::NumMemoryObjects(void)
{
	return mcMemory.NumObjects();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CObjects::NumDatabaseObjects(void)
{
	return mcDatabase.NumObjects();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetBaseObject(OIndex oi)
{
	CBaseObject*	pvObject;

	pvObject = mcMemory.Get(oi);
	if (pvObject)
	{
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
void ObjectsInit(char* szWorkingDirectory)
{
	UnknownsInit();
	gcObjects.Init(&gcUnknowns, szWorkingDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsKill(void)
{
	gcObjects.Kill();
	UnknownsKill();
}

