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
#include "ObjectDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectDeserialiser::Init(CSerialisedObject* pcSerialised)
{
	mpcSerialised = pcSerialised;
	mpcMemory = MemoryFile(pcSerialised, pcSerialised->GetLength());
	mcFile.Init(mpcMemory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectDeserialiser::Kill(void)
{
	mcFile.Kill();
	mpcSerialised = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectDeserialiser::Load(void)
{
	BOOL			bResult;
	int				iLength;
	CObjectHeader	sHeader;

	bResult = mcFile.Open(EFM_Read);
	if (!bResult)
	{
		return ONull;
	}

	bResult = ReadInt(&iLength);
	if (!bResult)
	{
		return ONull;
	}

	bResult = ReadObjectHeader(&sHeader);
	if (!bResult)
	{
		sHeader.Kill();
		return ONull;
	}

	CPointerObject	pObject;

	if (sHeader.mcType == OBJECT_POINTER_NULL)
	{
		sHeader.Kill();
		return ONull;
	}
	else if (sHeader.mcType == OBJECT_POINTER_ID)
	{

		pObject = gcObjects.Add(sHeader.mszClassName.Text(), sHeader.moi);
	}
	else if (sHeader.mcType == OBJECT_POINTER_NAMED)
	{
		pObject = gcObjects.Add(sHeader.mszClassName.Text(), sHeader.mszObjectName.Text(), sHeader.moi);
	}
	sHeader.Kill();

	bResult = pObject->Load(this);
	if (!bResult)
	{
		pObject->Kill();
		return ONull;
	}

	bResult = mcFile.Close();
	return ONull;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadPointer(CPointerObject* pObject)
{
	CPointerHeader	cHeader;
	BOOL			bResult;

	//This *is* the complete implementation of ReadPointer for this class.
	//The sub classes known how to load/find pointed to objects but not this class.

	ClearPointer(pObject);
	bResult = ReadPointerHeader(&cHeader);
	cHeader.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadPointerHeader(CPointerHeader* pcPointerHeader)
{
	pcPointerHeader->Init();

	ReturnOnFalse(ReadInt(&pcPointerHeader->mcType));

	if (pcPointerHeader->mcType == OBJECT_POINTER_NULL)
	{
		return TRUE;
	}
	else if (pcPointerHeader->mcType == OBJECT_POINTER_ID)
	{
		ReturnOnFalse(ReadLong(&pcPointerHeader->moi));
	}
	else if (pcPointerHeader->mcType == OBJECT_POINTER_NAMED)
	{
		ReturnOnFalse(ReadLong(&pcPointerHeader->moi));
		ReturnOnFalse(ReadString(&pcPointerHeader->mszObjectName));
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadObjectHeader(CObjectHeader* pcObjectHeader)
{
	pcObjectHeader->Init();
	ReturnOnFalse(ReadPointerHeader(pcObjectHeader));
	ReturnOnFalse(ReadInt(&pcObjectHeader->miClassSize));
	ReturnOnFalse(ReadString(&pcObjectHeader->mszClassName));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CObjectDeserialiser::Read(void* pvDest, filePos iSize, filePos iCount)
{
	return mcFile.Read(pvDest, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectDeserialiser::ClearPointer(CPointerObject* pObject)
{
	pObject->Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadDependent(CUnknown** ppcUnknown)
{
	CPointerHeader	cHeader;
	BOOL			bResult;

	//This *is* the complete implementation of ReadDependent for this class.
	//The sub classes known how to load/find dependent objects but not this.

	*ppcUnknown = NULL;
	bResult = ReadPointerHeader(&cHeader);
	cHeader.Kill();
	return bResult;
}

