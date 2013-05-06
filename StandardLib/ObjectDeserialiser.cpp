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
#include "Null.h"
#include "ObjectFileGeneral.h"
#include "ObjectHeader.h"
#include "ObjectGraphDeserialiser.h"
#include "ObjectDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::Init(CSerialisedObject* pcSerialised, CObjectAllocator* pcAllocator)
{
	if (!pcSerialised)
	{
		return FALSE;
	}

	mpcAllocator = pcAllocator;

	mpcMemory = MemoryFile(pcSerialised, pcSerialised->GetLength());
	mcFile.Init(mpcMemory);
	return TRUE;
} 


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectDeserialiser::Kill(void)
{
	mcFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectDeserialiser::Load(OIndex oiNew)
{
	BOOL			bResult;
	int				iLength;
	CObjectHeader	sHeader;
	OIndex			oiReplaced;

	bResult = mcFile.Open(EFM_Read);
	if (!bResult)
	{
		return Null();
	}

	bResult = ReadInt(&iLength);
	if (!bResult)
	{
		return Null();
	}

	bResult = ReadObjectHeader(&sHeader);
	if (!bResult)
	{
		sHeader.Kill();
		return Null();
	}

	CPointerObject	pObject;

	if (sHeader.mcType == OBJECT_POINTER_NULL)
	{
		sHeader.Kill();
		return Null();
	}
	else if (sHeader.mcType == OBJECT_POINTER_ID)
	{
		pObject = mpcAllocator->Add(sHeader.mszClassName.Text(), oiNew, &oiReplaced);
	}
	else if (sHeader.mcType == OBJECT_POINTER_NAMED)
	{
		pObject = mpcAllocator->Add(sHeader.mszClassName.Text(), sHeader.mszObjectName.Text(), oiNew, &oiReplaced);
	}
	sHeader.Kill();

	if (pObject.IsNull())
	{
		return Null();
	}

	if (oiReplaced != INVALID_O_INDEX)
	{
		AddIndexRemap(oiNew, oiReplaced);
	}

	bResult = pObject.Load(this);
	if (!bResult)
	{
		pObject->Kill();
		return Null();
	}

	bResult = mcFile.Close();
	return pObject;
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
		ReturnOnFalse(ReadString(&pcPointerHeader->mszObjectName, TRUE));
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
	ReturnOnFalse(ReadString(&pcObjectHeader->mszClassName, TRUE));
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
	pObject->ClearObject();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadPointer(CPointerObject* pObject)
{
	CPointerHeader	cHeader;
	BOOL			bResult;
	CBaseObject**	ppcObjectPtr;
	CObject*		pcEmbedding;

	ClearPointer(pObject);
	bResult = ReadPointerHeader(&cHeader);
	if (!bResult)
	{
		cHeader.Kill();
		return FALSE;
	}
	ppcObjectPtr = pObject->ObjectPtr();
	pcEmbedding = pObject->Embedding();

	AddDependent(&cHeader, ppcObjectPtr, (CBaseObject*)pcEmbedding);

	//cHeader is killed by mpcGraphDeserialiser.
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadDependent(CBaseObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	CPointerHeader	cHeader;
	BOOL			bResult;

	*ppcObjectPtr = NULL;
	bResult = ReadPointerHeader(&cHeader);
	bResult &= AddDependent(&cHeader, ppcObjectPtr, pcContaining);

	//cHeader is killed by mpcGraphDeserialiser.
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectDeserialiser::AddIndexRemap(OIndex oiNew, OIndex oiOld)
{
}

