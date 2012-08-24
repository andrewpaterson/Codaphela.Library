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

	if (sHeader.mcType == OBJECT_POINTER_NULL)
	{
		sHeader.Kill();
		return ONull;
	}
	else if (sHeader.mcType == OBJECT_POINTER_ID)
	{
		CPointerObject	pObject;

		pObject = gcObjects.Add(sHeader.mszClassName.Text(), sHeader.moi);
		sHeader.Kill();
		return pObject;
	}
	else if (sHeader.mcType == OBJECT_POINTER_NAMED)
	{
		CPointerObject	pObject;

		pObject = gcObjects.Add(sHeader.mszClassName.Text(), sHeader.mszObjectName.Text(), sHeader.moi);
		sHeader.Kill();
		return pObject;
	}

	//bResult = mpcThis->Save(this);
	//ReturnOnFalse(bResult);
	//iLength = mcFile.GetFileLength();
	//mcFile.Seek(0);
	//bResult = WriteInt((int)iLength);
	//ReturnOnFalse(bResult);

	bResult = mcFile.Close();
	return ONull;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadPointer(CPointerObject* pObject)
{
	CBaseObject*	pcBaseObject;

	pObject->Clear();
	pcBaseObject = &*pObject;
	//return ReadHeader(pcBaseObject);
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadPointerHeader(CPointerHeader* pcPointerHeader)
{
	//CopyPaste below
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadObjectHeader(CObjectHeader* pcObjectHeader)
{
	pcObjectHeader->Init();

	ReturnOnFalse(ReadInt(&pcObjectHeader->mcType));

	if (pcObjectHeader->mcType == OBJECT_POINTER_NULL)
	{
		return TRUE;
	}
	else if (pcObjectHeader->mcType == OBJECT_POINTER_ID)
	{
		ReturnOnFalse(ReadLong(&pcObjectHeader->moi));
	}
	else if (pcObjectHeader->mcType == OBJECT_POINTER_NAMED)
	{
		ReturnOnFalse(ReadLong(&pcObjectHeader->moi));
		ReturnOnFalse(ReadString(&pcObjectHeader->mszObjectName));
	}
	else
	{
		return FALSE;
	}

	ReturnOnFalse(ReadInt(&pcObjectHeader->miClassSize));
	ReturnOnFalse(ReadString(&pcObjectHeader->mszClassName));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadDependent(CBaseObject* pcBaseObject)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CObjectDeserialiser::Read(void* pvDest, filePos iSize, filePos iCount)
{
	return mcFile.Read(pvDest, iSize, iCount);
}

