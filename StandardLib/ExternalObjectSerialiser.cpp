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
#include "BaseLib/Logger.h"
#include "ObjectSerialiser.h"
#include "ExternalObjectSerialiser.h"
#include "SerialisedObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalObjectSerialiser::Init(CExternalObjectWriter* pcWriter)
{
	mpcWriter = pcWriter;
	mcDependentObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalObjectSerialiser::Kill(void)
{
	mcDependentObjects.Kill();
	mpcWriter = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CExternalObjectSerialiser::Write(CBaseObject* pcObject)
{
	CBaseObject*	pcUnwritten;
	BOOL			bResult;

	ReturnOnFalse(mpcWriter->Begin());

	AddDependent(pcObject);

	for (;;)
	{
		pcUnwritten = mcDependentObjects.GetUnwritten();
		if (pcUnwritten)
		{
			bResult = WriteUnwritten(pcUnwritten);
			if (!bResult)
			{
				return FALSE;
			}
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
BOOL CExternalObjectSerialiser::WriteUnwritten(CBaseObject* pcObject)
{
	CObjectSerialiser	cSerialiser;
	BOOL				bResult;
	CSerialisedObject*	pcSerialised;
	CChars				szDescription;

	if (!pcObject)
	{
		return gcLogger.Error2(__METHOD__, " Could write [NULL] object.", NULL);
	}

	cSerialiser.Init(&mcDependentObjects);

	bResult = cSerialiser.Save(pcObject);
	if (!bResult)
	{
		szDescription.Init();
		gcLogger.Error2(__METHOD__, " Could not serialise object [", pcObject->GetIdentifier(&szDescription), "].", NULL);
		szDescription.Kill();
		return FALSE;
	}

	pcSerialised = (CSerialisedObject*)cSerialiser.GetData();

	bResult = mpcWriter->Write(pcSerialised);
	if (!bResult)
	{
		szDescription.Init();
		gcLogger.Error2(__METHOD__, " Could write object [", pcObject->GetIdentifier(&szDescription), "].", NULL);
		szDescription.Kill();
		return FALSE;
	}

	cSerialiser.Kill();

	MarkWritten(pcObject);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalObjectSerialiser::AddDependent(CBaseObject* pcObject)
{
	mcDependentObjects.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalObjectSerialiser::MarkWritten(CBaseObject* pcObject)
{
	mcDependentObjects.Mark(pcObject);
}

