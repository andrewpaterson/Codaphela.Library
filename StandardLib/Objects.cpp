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
	moiNext = 1LL;

	CIndexedConfig	cConfig;

	cConfig.OptimiseForStreaming(szWorkingDirectory);
	mcData.Init(&cConfig);
	mcIndexes.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Kill(void)
{
	mcIndexes.Kill();
	mcData.Kill();
	moiNext = INVALID_OBJECT_IDENTIFIER;
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
	mcIndexes.AddWithID(pvObject, moiNext);
	StepNextObjectID();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::AddWithIDAndName(CBaseObject* pvObject, char* szName)
{
	mcIndexes.AddWithIDAndName(pvObject, moiNext, szName);
	StepNextObjectID();
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

