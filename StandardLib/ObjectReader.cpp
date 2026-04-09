/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "BaseLib/IndexedGeneral.h"
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
	size			iExpectedLength;
	CObjectHeader	sHeader;
	int32			iMagic;
	CBaseObject*	pcObject;

	iStart = mcFile.GetFilePos();
	bResult = ReadSize(&iExpectedLength);
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not read serialised object length.", NULL);
		return NULL;
	}

	iMagic = OBJECT_DATA;
	bResult = mcFile.ReadInt32(&iMagic);
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

	pcObject = mpcDependents->AllocateForDeserialisation(&sHeader);
	if (pcObject == NULL)
	{
		CChars	sz;
		sz.Init();
		gcLogger.Error2(__METHOD__, " Could not allocate serialised object [", sHeader.GetIdentifier(&sz), "] header.", NULL);
		sHeader.Kill();
		sz.Kill();
		return NULL;
	}

	bResult = pcObject->LoadManaged(this);
	if (!bResult)
	{
		CChars	sz;
		sz.Init();
		gcLogger.Error2(__METHOD__, " Could not load serialised object [", sHeader.GetIdentifier(&sz), "] fields.", NULL);
		sHeader.Kill();
		sz.Kill();
		pcObject->Kill();
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
		pcObject->Kill();
		return NULL;
	}

	pcObject->Initialised();
	return pcObject;
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
	size			iExpectedLength;
	filePos			iLength;
	bool			bResult;
	int				iMagic;

	iStart = mcFile.GetFilePos();
	bResult = ReadSize(&iExpectedLength);
	
	ObjectReaderErrorCheck(bResult, pcThis, __METHOD__, " Could not read object steam size loading object [", sz.Text(), "] 'froms'.", NULL);

	iMagic = OBJECT_FROM_HEAP;
	bResult = mcFile.ReadInt32(&iMagic);
	if (bResult)
	{
		bResult = iMagic == OBJECT_FROM_HEAP;
	}

	ObjectReaderErrorCheck(bResult, pcThis, __METHOD__, " Could not read object froms magic loading object [", sz.Text(), "] 'froms'.", NULL);

	bResult = pcThis->LoadHeapFroms(this);
	ObjectReaderErrorCheck(bResult, pcThis, __METHOD__, " Could not LoadHeadFroms() object [", sz.Text(), "] 'froms'.", NULL);

	iLength = mcFile.GetFilePos();
	bResult = (iLength - iStart) == iExpectedLength;
	ObjectReaderErrorCheck(bResult, pcThis, __METHOD__, " Size mismatch expected stream length [", SizeToString(iExpectedLength), "] but read length [", LongToString(iLength - iStart), "] reading object froms [", sz.Text(), "] 'froms'.", NULL);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectReader::ReadIdentifier(CObjectIdentifier* pcObjectIdentifier)
{
	int		iIgnored;
	bool	bResult;

	pcObjectIdentifier->Init();

	bResult = ReadInt32(&pcObjectIdentifier->meType);
	ReturnOnFalse(bResult);

	bResult = ReadInt32(&iIgnored);
	ReturnOnFalse(bResult);

	if (pcObjectIdentifier->meType == OBJECT_POINTER_NULL)
	{
		return true;
	}
	else if (pcObjectIdentifier->meType == OBJECT_POINTER_ID)
	{
		bResult = ReadInt64(&pcObjectIdentifier->moi);
		ReturnOnFalse(bResult);

		bResult = ReadInt16(&pcObjectIdentifier->muiSize);
		ReturnOnFalse(bResult);
	}
	else if (pcObjectIdentifier->meType == OBJECT_POINTER_NAMED)
	{
		bResult = ReadInt64(&pcObjectIdentifier->moi);
		ReturnOnFalse(bResult);

		bResult = ReadInt16(&pcObjectIdentifier->muiSize);
		ReturnOnFalse(bResult);

		bResult = pcObjectIdentifier->mszObjectName.ReadChars(this);
		ReturnOnFalse(bResult);
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
size CObjectReader::Read(void* pvDest, size iSize, size iCount)
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

	if ((cHeader.meType == OBJECT_POINTER_NAMED) || (cHeader.meType == OBJECT_POINTER_ID))
	{
		bResult = ReadInt16(&cHeader.miNumEmbedded);
		bResult &= ReadInt16(&cHeader.miEmbeddedIndex);
		if (!bResult)
		{
			cHeader.Kill();
			return false;
		}
		ppcObjectPtr = pObject->ObjectPtr();
		pcEmbedding = pObject->Embedding();

		bResult &= mpcDependents->AddDependent(&cHeader, ppcObjectPtr, (CBaseObject*)pcEmbedding, cHeader.miNumEmbedded, cHeader.miEmbeddedIndex);
		cHeader.Kill();
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
		if ((cHeader.meType == OBJECT_POINTER_NAMED) || (cHeader.meType == OBJECT_POINTER_ID))
		{
			bResult = ReadInt16(&cHeader.miNumEmbedded);
			bResult &= ReadInt16(&cHeader.miEmbeddedIndex);

			*ppcObjectPtr = NULL;
			bResult &= mpcDependents->AddDependent(&cHeader, ppcObjectPtr, pcContaining, cHeader.miNumEmbedded, cHeader.miEmbeddedIndex);
			cHeader.Kill();
			return bResult;
		}
		else
		{
			cHeader.Kill();
			return true;
		}
	}
	else
	{
		cHeader.Kill();
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
		if ((cHeader.meType == OBJECT_POINTER_NAMED) || (cHeader.meType == OBJECT_POINTER_ID))
		{
			bResult = ReadInt16(&cHeader.miNumEmbedded);
			bResult &= ReadInt16(&cHeader.miEmbeddedIndex);
			bResult &= ReadSInt(&iDistToRoot);

			SafeAssign(ppcObjectPtr, NULL);
			
			bResult &= mpcDependents->AddReverseDependent(&cHeader, ppcObjectPtr, pcContaining, cHeader.miNumEmbedded, cHeader.miEmbeddedIndex, iDistToRoot);
			cHeader.Kill();
			return bResult;
		}
		else if (cHeader.meType == OBJECT_POINTER_NULL)
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

