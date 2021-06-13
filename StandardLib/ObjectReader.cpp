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
#include "BaseLib/Log.h"
#include "CoreLib/IndexedGeneral.h"
#include "Null.h"
#include "ObjectFileGeneral.h"
#include "ObjectHeader.h"
#include "ExternalObjectDeserialiser.h"
#include "ObjectReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectReader::Init(CAbstractFile* pcFile, CDependentReadObjects* pcDependents)
{
	mpcDependents = pcDependents;
	mcFile.Init(pcFile);
	return TRUE;
} 


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectReader::Kill(void)
{
	mcFile.Kill();
	mpcDependents = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectReader::Read(void)
{
	BOOL			bResult;
	int				iLength;
	CObjectHeader	sHeader;
	int				iMagic;

	bResult = ReadInt(&iLength);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not read serialised object length.", NULL);
		return NULL;
	}

	bResult = mcFile.ReadInt(&iMagic);
	if (bResult)
	{
		bResult = iMagic = OBJECT_DATA;
	}
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Serialised object length magic corrupt.", NULL);
	}

	bResult = ReadObjectHeader(&sHeader);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not read serialised object header.");
		sHeader.Kill();
		return NULL;
	}

	CBaseObject*	pvObject;

	pvObject = mpcDependents->AllocateForDeserialisation(&sHeader);

	sHeader.Kill();

	if (pvObject == NULL)
	{
		gcLogger.Error2(__METHOD__, " Could not load serialised object.");
		return NULL;
	}

	bResult = pvObject->LoadManaged(this);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not load serialised object.");

		pvObject->Kill();
		return NULL;
	}

	pvObject->Initialised();
	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectReader::ReadHeapFroms(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectReader::ReadIdentifier(CObjectIdentifier* pcPointerHeader)
{
	pcPointerHeader->Init();

	ReturnOnFalse(ReadInt(&pcPointerHeader->mcType));

	if (pcPointerHeader->mcType == OBJECT_POINTER_NULL)
	{
		return TRUE;
	}
	else if (pcPointerHeader->mcType == OBJECT_POINTER_ID)
	{
		ReturnOnFalse(ReadInt(&pcPointerHeader->moi));
	}
	else if (pcPointerHeader->mcType == OBJECT_POINTER_NAMED)
	{
		ReturnOnFalse(ReadInt(&pcPointerHeader->moi));
		ReturnOnFalse(pcPointerHeader->mszObjectName.ReadChars(this));
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
BOOL CObjectReader::ReadObjectHeader(CObjectHeader* pcObjectHeader)
{
	pcObjectHeader->Init();
	ReturnOnFalse(ReadIdentifier(pcObjectHeader));
	ReturnOnFalse(pcObjectHeader->mszClassName.ReadChars(this));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CObjectReader::Read(void* pvDest, filePos iSize, filePos iCount)
{
	return mcFile.Read(pvDest, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectReader::ReadPointer(CPointer* pObject)
{
	CPointerHeader		cHeader;
	BOOL				bResult;
	CEmbeddedObject**	ppcObjectPtr;
	CObject*			pcEmbedding;

	pObject->UnsafeClearObject();

	bResult = ReadIdentifier(&cHeader);
	if (!bResult)
	{
		cHeader.Kill();
		return FALSE;
	}

	if ((cHeader.mcType == OBJECT_POINTER_NAMED) || (cHeader.mcType == OBJECT_POINTER_ID))
	{
		bResult = ReadInt(&cHeader.miNumEmbedded);
		bResult &= ReadInt(&cHeader.miEmbeddedIndex);
		if (!bResult)
		{
			cHeader.Kill();
			return FALSE;
		}
		ppcObjectPtr = pObject->ObjectPtr();
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
BOOL CObjectReader::ReadDependent(CEmbeddedObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	CPointerHeader	cHeader;
	BOOL			bResult;

	bResult = ReadIdentifier(&cHeader);
	if (bResult)
	{
		if ((cHeader.mcType == OBJECT_POINTER_NAMED) || (cHeader.mcType == OBJECT_POINTER_ID))
		{
			bResult = ReadInt(&cHeader.miNumEmbedded);
			bResult &= ReadInt(&cHeader.miEmbeddedIndex);

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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectReader::ReadReverseDependent(CEmbeddedObject** ppcObjectPtr)
{
	CPointerHeader	cHeader;
	BOOL			bResult;

	bResult = ReadIdentifier(&cHeader);
	if (bResult)
	{
		if ((cHeader.mcType == OBJECT_POINTER_NAMED) || (cHeader.mcType == OBJECT_POINTER_ID))
		{
			bResult = ReadInt(&cHeader.miNumEmbedded);
			bResult &= ReadInt(&cHeader.miEmbeddedIndex);

			*ppcObjectPtr = NULL;
			//bResult &= mpcDependents->AddDependent(&cHeader, ppcObjectPtr, pcContaining, cHeader.miNumEmbedded, cHeader.miEmbeddedIndex);
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


