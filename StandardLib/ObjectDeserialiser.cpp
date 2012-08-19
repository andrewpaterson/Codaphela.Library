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
#include "CoreLib/IndexedGeneral.h"
#include "ObjectFileGeneral.h"
#include "PointerObject.h"
#include "ObjectDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadPointer(CPointerObject* pObject)
{
	CBaseObject*	pcBaseObject;

	pObject->Clear();
	pcBaseObject = &*pObject;
	return ReadHeader(pcBaseObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadHeader(CBaseObject* pcBaseObject)
{
	OIndex		oi;
	int			c;
	CChars		szName;

	ReturnOnFalse(ReadInt(&c));

	if (c == OBJECT_POINTER_NULL)
	{
		return TRUE;
	}
	else if (c == OBJECT_POINTER_ID)
	{
		ReturnOnFalse(ReadLong(&oi));
		return FALSE;
	}
	else if (c == OBJECT_POINTER_NAMED)
	{
		ReturnOnFalse(ReadString(&szName));
		szName.Kill();
		return FALSE;
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
BOOL CObjectDeserialiser::ReadDependent(CBaseObject* pcBaseObject)
{
	return ReadHeader(pcBaseObject);
}

