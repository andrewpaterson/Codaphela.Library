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
#include "BaseLib/ErrorTypes.h"
#include "Objects.h"
#include "ObjectFileGeneral.h"
#include "DependentReadObjects.h"


//////////////////////////////////////////////////////////////////////////
//
// Points a pointer that was read (*ppcPtrToBeUpdated, in pcContaining) at
// pcObject's embedded object iEmbeddedIndex: the object itself for 0, or
// if it's hollow one of its embedded hollows.
//
//////////////////////////////////////////////////////////////////////////
bool CDependentReadObjects::PointTo(CBaseObject* pcObject, uint16 iEmbeddedIndex, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcContaining)
{
	CEmbeddedObject*	pcEmbeddedObject;

	pcEmbeddedObject = pcObject->GetEmbeddedObject(iEmbeddedIndex);
	if (pcEmbeddedObject == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Object {", ObjectToString(pcObject), "} has no embedded object [", IntToString(iEmbeddedIndex), "].", NULL);
	}

	*ppcPtrToBeUpdated = pcEmbeddedObject;
	if (pcContaining)
	{
		pcEmbeddedObject->AddHeapFrom(pcContaining, false);
	}
	return true;
}

