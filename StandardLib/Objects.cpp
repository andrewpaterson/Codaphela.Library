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
	CIndexedConfig	cConfig;

	mpcUnknownsAllocatingFrom = pcUnknownsAllocatingFrom;
	mcIndexGenerator.Init();

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
	mcIndexGenerator.Kill();
	mpcUnknownsAllocatingFrom = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::AddWithID(CBaseObject* pvObject)
{
	return mcMemory.AddWithID(pvObject, mcIndexGenerator.PopIndex(), NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::AddWithIDAndName(CBaseObject* pvObject, char* szObjectName)
{
	return mcMemory.AddWithIDAndName(pvObject, mcIndexGenerator.PopIndex(), szObjectName, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::Add(char* szClassName, OIndex oi, OIndex* oiExisting)
{
	CBaseObject*	pvObject;
	CBaseObject*	pvExisting;
	BOOL			bResult;

	pvObject = (CBaseObject*)mpcUnknownsAllocatingFrom->Add(szClassName);
	if (pvObject)
	{
		if (!pvObject->IsNamed())
		{
			CPointerObject	pObject;

			pvExisting = NULL;
			mcMemory.AddWithID(pvObject, oi, &pvExisting);
			if (pvExisting)
			{
				bResult = mcMemory.AddWithID(pvExisting, mcIndexGenerator.PopIndex(), NULL);
				if (!bResult)
				{
					pvObject->Kill();
					return ONull;
				}
				*oiExisting = pvExisting->GetOI();
			}
			else
			{
				*oiExisting = INVALID_O_INDEX;
			}

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
CPointerObject CObjects::Add(char* szClassName, char* szObjectName, OIndex oi, OIndex* oiExisting)
{
	CBaseObject*	pvObject;
	CBaseObject*	pvExisting;
	BOOL			bResult;

	pvObject = (CBaseObject*)mpcUnknownsAllocatingFrom->Add(szClassName);
	if (pvObject)
	{
		if (pvObject->IsNamed())
		{
			CPointerObject	pObject;

			pvExisting = NULL;
			mcMemory.AddWithIDAndName(pvObject, oi, szObjectName, &pvExisting);
			if (pvExisting)
			{
				bResult = mcMemory.AddWithID(pvExisting, mcIndexGenerator.PopIndex(), NULL);
				if (!bResult)
				{
					pvObject->Kill();
					return ONull;
				}
				*oiExisting = pvExisting->GetOI();
			}
			else
			{
				*oiExisting = INVALID_O_INDEX;
			}

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

			mcMemory.AddWithIDAndName(pvObject, mcIndexGenerator.PopIndex(), szObjectName, NULL);
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
CPointer<CRoot> CObjects::AddRoot(void)
{
	CPointer<CRoot>	cRoot;

	cRoot = Get(ROOT_NAME);
	if (!cRoot)
	{
		cRoot = Add<CRoot>(ROOT_NAME);
		cRoot->Init(this);
	}
	return cRoot;
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
CIndexGenerator* CObjects::GetIndexGenerator(void)
{
	return &mcIndexGenerator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::Allocate(char* szClassName)
{
	CBaseObject*	pvObject;

	pvObject = (CBaseObject*)mpcUnknownsAllocatingFrom->Add(szClassName);
	if (pvObject)
	{
		pvObject->CBaseObject::PreInit(this);
	}
	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::RemoveInKill(CBaseObject* pvObject)
{
	mcMemory.Remove(pvObject->GetOI());
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

