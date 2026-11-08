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
#include "Objects.h"
#include "FreeObjects.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeObjects::Init(void)
{
	PreInit();
	mpFreeObjects = OMalloc<CSetObject>();
	mpFreeObjects->MakeSubRoot();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeObjects::Free(void)
{
	//A stored CFreeObjects is only leaving memory: its objects are in the database with it.
	if (!IsStored() && !mpFreeObjects->IsEmpty())
	{
		gcLogger.Error2(__METHOD__, " Could not free FreeObjects.  Every Object contained must be explicitly freed first.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CFreeObjects::Class(void)
{
	Nes(M_Pointer(mpFreeObjects));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CFreeObjects::Add(CPointer &p)
{
	bool	bResult;

	if (p.IsNull())
	{
		gcLogger.Error2(__METHOD__, " Could not add null Object.", NULL);
		return NULL;
	}

	if (p->IsEmbedded())
	{
		gcLogger.Error2(__METHOD__, " Could not add embedded Object [", ObjectToString(p.Object(), true), "].", NULL);
		return NULL;
	}

	bResult = mpFreeObjects->Add(p);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not add Object [", ObjectToString(p.Object()), "].", NULL);
		return NULL;
	}

	return (CBaseObject*)p.Object();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFreeObjects::Remove(CBaseObject* pcObject)
{
	bool	bResult; 

	if (pcObject == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Could not remove null Object.", NULL);
	}

	bResult = mpFreeObjects->Remove(pcObject);
	if (!bResult)
	{
		return gcLogger.Error2(__METHOD__, " Could not remove Object [", ObjectToString(pcObject), "].", NULL);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CFreeObjects::NumObjects(void)
{
	return mpFreeObjects->NumElements();
}


