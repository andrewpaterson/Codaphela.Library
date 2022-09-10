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
#include "ExternalObjectDeserialiser.h"
#include "ObjectReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectReader::Init(CAbstractFile* pcFile, CDependentReadObjects* pcDependents)
{
	mpcDependents = pcDependents;
	mcFile.Init(pcFile);
	return true;
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
	bool			bResult;
	filePos			iLength;
	filePos			iStart;
	int				iExpectedLength;
	CObjectHeader	sHeader;
	int				iMagic;

	iStart = mcFile.GetFilePos();
	bResult = ReadInt(&iExpectedLength);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not read serialised object length.", NULL);
		return NULL;
	}

	iMagic = OBJECT_DATA;
	bResult = mcFile.ReadInt(&iMagic);
	if (bResult)
	{
		bResult = iMagic == OBJECT_DATA;
	}
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not read serialised object magic or magic corrupt [", IntToString(iMagic, 16), "].", NULL);
		return NULL;
	}

	bResult = ReadObjectHeader(&sHeader);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not read serialised object header.", NULL);
		sHeader.Kill();
		return NULL;
	}

	CBaseObject*	pvObject;

	pvObject = mpcDependents->AllocateForDeserialisation(&sHeader);

	if (pvObject == NULL)
	{
		CChars	sz;
		sz.Init();
		gcLogger.Error2(__METHOD__, " Could not allocate serialised object [", sHeader.GetIdentifier(&sz), "] header.", NULL);
		sHeader.Kill();
		sz.Kill();
		return NULL;
	}


	bResult = pvObject->LoadManaged(this);
	if (!bResult)
	{
		CChars	sz;
		sz.Init();
		gcLogger.Error2(__METHOD__, " Could not load serialised object [", sHeader.GetIdentifier(&sz), "] fields.", NULL);
		sHeader.Kill();
		sz.Kill();
		pvObject->Kill();
		return NULL;
	}

	sHeader.Kill();

	iLength = mcFile.GetFilePos();
	bResult = (iLength - iStart) == iExpectedLength;
	if (!bResult)
	{
		CChars	sz;
		sz.Init();
		gcLogger.Error2(__METHOD__, " Could not load serialised object [", sHeader.GetIdentifier(&sz), "] fields.", NULL);
		sHeader.Kill();
		sz.Kill();
		pvObject->Kill();
		return NULL;
	}


	pvObject->Initialised();
	return pvObject;
}


#define ObjectReaderErrorCheck(result, object, ...) \
if (!result) \
{ \
	CChars			sz; \
\
	sz.Init(); \
	object->GetIdentifier(&sz); \
	gcLogger.Error2(__VA_ARGS__); \
	sz.Kill(); \
	return false; \
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectReader::ReadHeapFroms(CBaseObject* pcThis)
{
	filePos			iStart;
	int				iExpectedLength;
	filePos			iLength;
	bool			bResult;
	int				iMagic;

	iStart = mcFile.GetFilePos();
	bResult = ReadInt(&iExpectedLength);
	
	ObjectReaderErrorCheck(bResult, pcThis, __METHOD__, " Could not read object steam size loading object [", sz.Text(), "] 'froms'.", NULL);

	iMagic = OBJECT_FROM_HEAP;
	bResult = mcFile.ReadInt(&iMagic);
	if (bResult)
	{
		bResult = iMagic == OBJECT_FROM_HEAP;
	}

	ObjectReaderErrorCheck(bResult, pcThis, __METHOD__, " Could not read object froms magic loading object [", sz.Text(), "] 'froms'.", NULL);

	bResult = pcThis->LoadHeapFroms(this);
	ObjectReaderErrorCheck(bResult, pcThis, __METHOD__, " Could not LoadHeadFroms() object [", sz.Text(), "] 'froms'.", NULL);

	iLength = mcFile.GetFilePos();
	bResult = (iLength - iStart) == iExpectedLength;
	ObjectReaderErrorCheck(bResult, pcThis, __METHOD__, " Size mismatch expected stream length [", LongLongToString(iExpectedLength), "] but read length [", LongLongToString(iLength - iStart), "] reading object froms [", sz.Text(), "] 'froms'.", NULL);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectReader::ReadIdentifier(CObjectIdentifier* pcPointerHeader)
{
	int		iIgnored;

	pcPointerHeader->Init();

	ReturnOnFalse(ReadInt(&pcPointerHeader->mcType));
	ReturnOnFalse(ReadInt(&iIgnored));

	if (pcPointerHeader->mcType == OBJECT_POINTER_NULL)
	{
		return true;
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
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectReader::ReadObjectHeader(CObjectHeader* pcObjectHeader)
{
	pcObjectHeader->Init();
	ReturnOnFalse(ReadIdentifier(pcObjectHeader));
	ReturnOnFalse(pcObjectHeader->mszClassName.ReadChars(this));
	return true;
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
bool CObjectReader::ReadPointer(CPointer* pObject)
{
	CPointerHeader		cHeader;
	bool				bResult;
	CEmbeddedObject**	ppcObjectPtr;
	CObject*			pcEmbedding;

	pObject->UnsafeClearObject();

	bResult = ReadIdentifier(&cHeader);
	if (!bResult)
	{
		cHeader.Kill();
		return false;
	}

	if ((cHeader.mcType == OBJECT_POINTER_NAMED) || (cHeader.mcType == OBJECT_POINTER_ID))
	{
		bResult = ReadInt(&cHeader.miNumEmbedded);
		bResult &= ReadInt(&cHeader.miEmbeddedIndex);
		if (!bResult)
		{
			cHeader.Kill();
			return false;
		}
		ppcObjectPtr = pObject->ObjectPtr();
		pcEmbedding = pObject->Embedding();

		bResult &= mpcDependents->AddDependent(&cHeader, ppcObjectPtr, (CBaseObject*)pcEmbedding, cHeader.miNumEmbedded, cHeader.miEmbeddedIndex);
		return bResult;
	}
	else
	{
		return true;
	}


	//cHeader is killed by mpcDependents.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectReader::ReadDependent(CEmbeddedObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	CPointerHeader	cHeader;
	bool			bResult;

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
			return true;
		}
	}
	else
	{
		return false;
	}

	//cHeader is killed by mpcDependents.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectReader::ReadReverseDependent(CEmbeddedObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	CPointerHeader	cHeader;
	bool			bResult;
	int				iDistToRoot;

	bResult = ReadIdentifier(&cHeader);
	if (bResult)
	{
		if ((cHeader.mcType == OBJECT_POINTER_NAMED) || (cHeader.mcType == OBJECT_POINTER_ID))
		{
			bResult = ReadInt(&cHeader.miNumEmbedded);
			bResult &= ReadInt(&cHeader.miEmbeddedIndex);
			bResult &= ReadInt(&iDistToRoot);

			*ppcObjectPtr = NULL;
			
			bResult &= mpcDependents->AddReverseDependent(&cHeader, ppcObjectPtr, pcContaining, cHeader.miNumEmbedded, cHeader.miEmbeddedIndex, iDistToRoot);
			return bResult;
		}
		else if (cHeader.mcType == OBJECT_POINTER_NULL)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	//cHeader is killed by mpcDependents.
}


