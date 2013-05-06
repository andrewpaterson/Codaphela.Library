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
#include "ObjectHeader.h"
#include "Objects.h"
#include "HollowObject.h"
#include "ObjectGraphDeserialiser.h"
#include "ObjectIndexedDataDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectIndexedDataDeserialiser::Init(CSerialisedObject* pcSerialised, CObjectAllocator* pcAllocator)
{
	if (!CObjectDeserialiser::Init(pcSerialised, pcAllocator))
	{
		return FALSE;
	}

	return TRUE;
} 


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectIndexedDataDeserialiser::Kill(void)
{
	CObjectDeserialiser::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectIndexedDataDeserialiser::AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	CPointerObject	pObject;
	CBaseObject*	pcObject;

	pcObject = gcObjects.GetInMemoryObject(pcHeader->moi);
	if (pcObject == NULL)
	{
		 pcObject = AddHollow(pcHeader, ppcObjectPtr, pcContaining);
	}
	FixPointer(pcObject, ppcObjectPtr, pcContaining);
	return pcObject != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHollowObject* CObjectIndexedDataDeserialiser::AddHollow(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	CPointerObject		pObject;

	if (pcHeader->IsNamed())
	{
		pObject = gcObjects.AddHollow(pcHeader->mszObjectName.Text(), pcHeader->moi);
	}
	else
	{
		pObject = gcObjects.AddHollow(pcHeader->moi);
	}
	return (CHollowObject*)pObject.Object();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectIndexedDataDeserialiser::AddIndexRemap(OIndex oiNew, OIndex oiOld)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectIndexedDataDeserialiser::FixPointer(CBaseObject* pcBaseObject, CBaseObject** ppcPointedFrom, CBaseObject* pcContaining)
{
	*ppcPointedFrom = pcBaseObject;

	if ((pcContaining) && (pcBaseObject))
	{
		pcBaseObject->AddFrom(pcContaining);
	}
}