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
#include "ObjectWriter.h"
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
bool CExternalObjectSerialiser::Write(CBaseObject* pcObject)
{
	CBaseObject*	pcUnwritten;

	ReturnOnFalse(mpcWriter->Begin());

	AddDependent(pcObject);

	for (pcUnwritten = mcDependentObjects.GetUnwritten(); pcUnwritten != NULL; pcUnwritten = mcDependentObjects.GetUnwritten())
	{
		if (!WriteUnwritten(pcUnwritten))
		{
			//What was begun is ended, written or not.
			mpcWriter->End();
			return false;
		}
	}

	return mpcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExternalObjectSerialiser::WriteUnwritten(CBaseObject* pcObject)
{
	CObjectWriter		cObjectWriter;
	bool				bResult;
	CSerialisedObject*	pcSerialised;
	CChars				szDescription;
	CMemoryFile			cMemory;

	if (!pcObject)
	{
		return gcLogger.Error2(__METHOD__, " Could not write [NULL] object.", NULL);
	}

	cMemory.Init();
	cMemory.Open(EFM_ReadWrite_Create);

	cObjectWriter.Init(&cMemory, &mcDependentObjects);

	bResult = cObjectWriter.Write(pcObject);

	cMemory.Close();
	cObjectWriter.Kill();

	if (!bResult)
	{
		cMemory.Kill();
		szDescription.Init();
		gcLogger.Error2(__METHOD__, " Could not serialise object [", pcObject->PrintIdentifier(&szDescription), "].", NULL);
		szDescription.Kill();
		return false;
	}

	pcSerialised = (CSerialisedObject*)cMemory.GetBufferPointer();

	bResult = mpcWriter->Write(pcSerialised);

	cMemory.Kill();

	if (!bResult)
	{
		szDescription.Init();
		gcLogger.Error2(__METHOD__, " Could not write object [", pcObject->PrintIdentifier(&szDescription), "].", NULL);
		szDescription.Kill();
		return false;
	}

	return MarkWritten(pcObject);
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
bool CExternalObjectSerialiser::MarkWritten(CBaseObject* pcObject)
{
	return mcDependentObjects.Mark(pcObject);
}

