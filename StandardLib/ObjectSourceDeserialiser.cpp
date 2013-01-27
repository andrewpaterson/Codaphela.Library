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
#include "ObjectGraphDeserialiser.h"
#include "ObjectSourceDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSourceDeserialiser::Init(CObjectGraphDeserialiser* pcGraphDeserialiser, CSerialisedObject* pcSerialised, CObjectAllocator* pcAllocator)
{
	if (!CObjectDeserialiser::Init(pcSerialised, pcAllocator))
	{
		return FALSE;
	}

	mpcGraphDeserialiser = pcGraphDeserialiser;
	return TRUE;
} 


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSourceDeserialiser::Kill(void)
{
	mpcGraphDeserialiser = NULL;
	CObjectDeserialiser::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSourceDeserialiser::AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	mpcGraphDeserialiser->AddDependent(pcHeader, ppcObjectPtr, pcContaining);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSourceDeserialiser::AddIndexRemap(OIndex oiNew, OIndex oiOld)
{
	mpcGraphDeserialiser->AddIndexRemap(oiNew, oiOld);
}

