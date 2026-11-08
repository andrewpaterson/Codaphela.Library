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
#include "BaseLib/LogString.h"
#include "BaseLib/SystemAllocator.h"
#include "ObjectIO.h"
#include "ExternalObjectReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CExternalObjectReader::Kill(void)
{
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExternalObjectReader::Begin(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CExternalObjectReader::End(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSerialisedObject* CExternalObjectReader::ReadSerialised(CFileReader* pcReader)
{
	int32				iLength;
	CSerialisedObject*	pcSerialised;
	void*				pvOffset;
	bool				bResult;

	if (!pcReader->ReadInt32(&iLength))
	{
		return NULL;
	}
	//The length counts itself.  An int32's 2GB is the upper bound: a length the data can't fill fails to read.
	if (iLength < (int32)sizeof(uint32))
	{
		gcLogger.Error2(__METHOD__, " Serialised object length [", IntToString(iLength), "] is shorter than the length.", NULL);
		return NULL;
	}

	pcSerialised = (CSerialisedObject*)gcSystemAllocator.Malloc(iLength);
	if (!pcSerialised)
	{
		return NULL;
	}

	if (!pcSerialised->SetLength(iLength))
	{
		gcSystemAllocator.Free(pcSerialised);
		return NULL;
	}
	iLength -= (int32)sizeof(uint32);

	pvOffset = RemapSinglePointer(pcSerialised, sizeof(uint32));
	bResult = pcReader->ReadData(pvOffset, iLength);
	if (!bResult)
	{
		gcSystemAllocator.Free(pcSerialised);
		return NULL;
	}
	return pcSerialised;
}

