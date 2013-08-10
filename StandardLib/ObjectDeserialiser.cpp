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
BOOL CObjectDeserialiser::Init(CDependentObjectAdder* pcDependents)
{
	mpcDependents = pcDependents;

	return TRUE;
} 


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectDeserialiser::Kill(void)
{
	mpcDependents = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjectDeserialiser::Load(CSerialisedObject* pcSerialised)
{
	BOOL			bResult;
	int				iLength;
	CObjectHeader	sHeader;
	CMemoryFile*	pcMemory;

	if (!pcSerialised)
	{
		gcLogger.Error("CObjectDeserialiser::Load Serialised Object is NULL.");
		return Null();
	}

	pcMemory = MemoryFile(pcSerialised, pcSerialised->GetLength());
	mcFile.Init(pcMemory);
	bResult = mcFile.Open(EFM_Read);
	if (!bResult)
	{
		mcFile.Kill();
		return Null();
	}

	bResult = ReadInt(&iLength);
	if (!bResult)
	{
		gcLogger.Error("CObjectDeserialiser::Load Could not read serialised object length.");
		mcFile.Close();
		mcFile.Kill();

		return Null();
	}

	bResult = ReadObjectHeader(&sHeader);
	if (!bResult)
	{
		gcLogger.Error("CObjectDeserialiser::Load Could not read serialised object header.");
		mcFile.Close();
		mcFile.Kill();
		sHeader.Kill();
		return Null();
	}

	CPointer	pObject;
	pObject = mpcDependents->AllocateObject(&sHeader);

	sHeader.Kill();

	if (pObject.IsNull())
	{
		gcLogger.Error("CObjectDeserialiser::Load Could not load serialised object.");
		mcFile.Close();
		mcFile.Kill();
		return Null();
	}

	bResult = pObject.Load(this);
	if (!bResult)
	{
		gcLogger.Error("CObjectDeserialiser::Load Could not load serialised object.");
		mcFile.Close();
		mcFile.Kill();

		pObject->Kill();
		return Null();
	}

	mcFile.Close();
	mcFile.Kill();

	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadIdentifier(CObjectIdentifier* pcPointerHeader)
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
	ReturnOnFalse(ReadIdentifier(pcObjectHeader));
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
void CObjectDeserialiser::ClearPointer(CPointer* pObject)
{
	pObject->ClearObject();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadPointer(CPointer* pObject)
{
	CPointerHeader	cHeader;
	BOOL			bResult;
	CBaseObject**	ppcObjectPtr;
	CObject*		pcEmbedding;

	ClearPointer(pObject);

	bResult = ReadIdentifier(&cHeader);
	if (!bResult)
	{
		cHeader.Kill();
		return FALSE;
	}

	if ((cHeader.mcType == OBJECT_POINTER_NAMED) || (cHeader.mcType == OBJECT_POINTER_ID))
	{
		bResult = ReadShort(&cHeader.miNumEmbedded);
		bResult &= ReadShort(&cHeader.miEmbeddedIndex);
		if (!bResult)
		{
			cHeader.Kill();
			return FALSE;
		}
		ppcObjectPtr = (CBaseObject**)(pObject->ObjectPtr());
		pcEmbedding = pObject->Embedding();

		bResult &= mpcDependents->AddDependent(&cHeader, ppcObjectPtr, (CBaseObject*)pcEmbedding, cHeader.miNumEmbedded, cHeader.miEmbeddedIndex);
		return bResult;
	}
	else
	{
		return TRUE;
	}


	//cHeader is killed by mpcDependents.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectDeserialiser::ReadDependent(CBaseObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	CPointerHeader	cHeader;
	BOOL			bResult;

	bResult = ReadIdentifier(&cHeader);
	if (bResult)
	{
		if ((cHeader.mcType == OBJECT_POINTER_NAMED) || (cHeader.mcType == OBJECT_POINTER_ID))
		{
			bResult = ReadShort(&cHeader.miNumEmbedded);
			bResult &= ReadShort(&cHeader.miEmbeddedIndex);

			*ppcObjectPtr = NULL;
			bResult &= mpcDependents->AddDependent(&cHeader, ppcObjectPtr, pcContaining, cHeader.miNumEmbedded, cHeader.miEmbeddedIndex);
			return bResult;
		}
		else
		{
			return TRUE;
		}
	}
	else
	{
		return FALSE;
	}

	//cHeader is killed by mpcDependents.
}

