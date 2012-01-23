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
#include "DependentObjects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentObjects::Init(void)
{
	mcObjects.Init(128);
	miGetIndex = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentObjects::Kill(void)
{
	mcObjects.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareDependentObject(const void* ps1, const void* ps2)
{
	SDependentObject*	psObj1;
	SDependentObject*	psObj2;

	psObj1 = (SDependentObject*)ps1;
	psObj2 = (SDependentObject*)ps2;

	if (psObj1->pcObject < psObj2->pcObject)
	{
		return -1;
	}
	else if (psObj1->pcObject > psObj2->pcObject)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentObjects::Add(CBaseObject* pcObject)
{
	SDependentObject	sObject;
	BOOL				bExists;
	int					iIndex;

	sObject.pcObject = pcObject;
	sObject.bWritten = FALSE;

	bExists = mcObjects.FindInSorted(&sObject, &CompareDependentObject, &iIndex);
	if (!bExists)
	{
		mcObjects.InsertAt(&sObject, iIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDependentObjects::GetUnwritten(void)
{
	int					iOldIndex;
	SDependentObject*	psObject;

	if (mcObjects.NumElements() == 0)
	{
		return NULL;
	}

	iOldIndex = miGetIndex;
	for (;;)
	{
		if (miGetIndex >= mcObjects.NumElements())
		{
			miGetIndex = 0;
		}
		else
		{
			miGetIndex++;
		}

		psObject = mcObjects.Get(miGetIndex);
		if (!psObject->bWritten)
		{
			return psObject->pcObject;
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
void CDependentObjects::Mark(CBaseObject* pcObject)
{
	SDependentObject	sObject;
	int					iIndex;

	sObject.pcObject = pcObject;
	sObject.bWritten = TRUE;

	iIndex = mcObjects.InsertIntoSorted(&CompareDependentObject, &sObject, TRUE);
}
