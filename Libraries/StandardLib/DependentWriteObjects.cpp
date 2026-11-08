/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "BaseObject.h"
#include "DependentWriteObjects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentWriteObjects::Init(void)
{
	mcObjects.Init();
	mapcInOrder.Init();
	muiNextUnwritten = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentWriteObjects::Kill(void)
{
	mapcInOrder.Kill();
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
		mapcInOrder.Add(pcObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The first object added that isn't written yet (the same one again until
// it's Marked), or NULL when all of them are.  Objects are never unmarked,
// so the written ones before it are skipped once: a whole write takes
// every object once.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDependentWriteObjects::GetUnwritten(void)
{
	size			uiNumObjects;
	CBaseObject*	pcObject;

	uiNumObjects = mapcInOrder.NumElements();
	while (muiNextUnwritten < uiNumObjects)
	{
		pcObject = mapcInOrder.GetPtr(muiNextUnwritten);
		if (!IsWritten(pcObject))
		{
			return pcObject;
		}
		muiNextUnwritten++;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDependentWriteObjects::IsWritten(CBaseObject* pcObject)
{
	CDependentWriteObject	cObject;
	size					iIndex;

	cObject.Init(pcObject, false);
	if (mcObjects.FindInSorted(&cObject, &CompareDependentWriteObject, &iIndex))
	{
		return mcObjects.Get(iIndex)->mbWritten;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDependentWriteObjects::Mark(CBaseObject* pcObject)
{
	CDependentWriteObject	cObject;
	size					iIndex;
	CDependentWriteObject*	pcDependent;

	cObject.Init(pcObject, true);
	if (!mcObjects.FindInSorted(&cObject, &CompareDependentWriteObject, &iIndex))
	{
		return gcLogger.Error2(__METHOD__, " Object {", ObjectToString(pcObject), "} written is not a dependent.", NULL);
	}
	pcDependent = mcObjects.Get(iIndex);
	pcDependent->mbWritten = true;
	return true;
}

