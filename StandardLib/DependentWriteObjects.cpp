/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "DependentWriteObjects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentWriteObjects::Init(void)
{
	mcObjects.Init();
	miGetIndex = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentWriteObjects::Kill(void)
{
	mcObjects.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentWriteObjects::Add(CBaseObject* pcObject)
{
	CDependentWriteObject	cObject;
	bool					bExists;
	size 					iIndex;

	cObject.Init(pcObject, false);

	bExists = mcObjects.FindInSorted(&cObject, &CompareDependentWriteObject, &iIndex);
	if (!bExists)
	{
		mcObjects.InsertAt(&cObject, iIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDependentWriteObjects::GetUnwritten(void)
{
	size 					iOldIndex;
	CDependentWriteObject*	psObject;
	size					uiObjects;

	uiObjects = mcObjects.NumElements();
	if (uiObjects == 0)
	{
		return NULL;
	}

	iOldIndex = miGetIndex;
	for (;;)
	{
		if (miGetIndex >= uiObjects - 1)
		{
			miGetIndex = 0;
		}
		else
		{
			miGetIndex++;
		}

		psObject = mcObjects.Get(miGetIndex);
		if (!psObject->mbWritten)
		{
			return psObject->mpcObject;
		}

		if (miGetIndex == iOldIndex)
		{
			return NULL;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentWriteObjects::Mark(CBaseObject* pcObject)
{
	CDependentWriteObject	cObject;
	size					iIndex;
	bool					bResult;
	CDependentWriteObject*	pcDependent;

	cObject.Init(pcObject, true);
	bResult = mcObjects.FindInSorted(&cObject, &CompareDependentWriteObject, &iIndex);
	pcDependent = mcObjects.Get(iIndex);
	pcDependent->mbWritten = true;
}

