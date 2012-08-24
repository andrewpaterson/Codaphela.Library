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
#include "DependentObjectSerialiser.h"
#include "ObjectGraphSerialiser.h"
#include "SerialisedObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphSerialiser::Init(CObjectWriter* pcWriter)
{
	mpcWriter = pcWriter;
	mcDependentObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphSerialiser::Kill(void)
{
	mcDependentObjects.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphSerialiser::Write(CBaseObject* pcObject)
{
	CBaseObject*	pcUnwritten;

	ReturnOnFalse(mpcWriter->Begin());

	AddDependent(pcObject);

	for (;;)
	{
		pcUnwritten = mcDependentObjects.GetUnwritten();
		if (pcUnwritten)
		{
			ReturnOnFalse(WriteUnwritten(pcUnwritten));
		}
		else
		{
			break;
		}
	}

	return mpcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphSerialiser::WriteUnwritten(CBaseObject* pcObject)
{
	CDependentObjectSerialiser	cSerialiser;
	BOOL						bResult;
	CSerialisedObject*			pcSerialised;

	cSerialiser.Init(this, pcObject);

	bResult = cSerialiser.Save();
	ReturnOnFalse(bResult);

	pcSerialised = (CSerialisedObject*)cSerialiser.GetData();

	bResult = mpcWriter->Write(pcSerialised);
	ReturnOnFalse(bResult);

	cSerialiser.Kill();

	MarkWritten(pcObject);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphSerialiser::AddDependent(CBaseObject* pcObject)
{
	mcDependentObjects.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphSerialiser::MarkWritten(CBaseObject* pcObject)
{
	mcDependentObjects.Mark(pcObject);
}

