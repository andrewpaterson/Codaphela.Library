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
	lsize			iExpectedLength;
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
		pcObject->GetObjectsThisIn()->ReplaceWithHollow(pcObject);
		return NULL;
	}

	sHeader.Kill();

	iLength = mcFile.GetFilePos();
	bResult = (lsize)(iLength - iStart) == iExpectedLength;
	if (!bResult)
	{
		CChars	sz;
		sz.Init();
		gcLogger.Error2(__METHOD__, " Could not load serialised object [", sHeader.GetIdentifier(&sz), "] fields.", NULL);
		sHeader.Kill();
		sz.Kill();
		pcObject->GetObjectsThisIn()->ReplaceWithHollow(pcObject);
		return NULL;
	}

	pcObject->SetDirty(false);
	pcObject->Initialised();
	return pcObject;
}


#define ObjectReaderErrorCheck(result, object, ...) \
if (!result) \
{ \
	CChars			sz; \
\
	sz.Init(); \
	object->PrintIdentifier(&sz); \
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
	lsize			iExpectedLength;
	filePos			iLength;
	bool			bResult;
	int32			iMagic;

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
	bResult = (lsize)(iLength - iStart) == iExpectedLength;
	ObjectReaderErrorCheck(bResult, pcThis, __METHOD__, " Size mismatch expected stream length [", LongToString((uint64)iExpectedLength), "] but read length [", LongToString(iLength - iStart), "] reading object froms [", sz.Text(), "] 'froms'.", NULL);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectReader::ReadIdentifier(CObjectIdentifier* pcObjectIdentifier)
{
	int32	iIgnored;
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
lsize CObjectReader::Read(void* pvDest, lsize iSize, lsize iCount)
{
	return mcFile.Read(pvDest, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer's identifier and, unless it's NULL, which of the object's
// embedded objects it points to.  ReadIdentifier refuses unknown types.
//
//////////////////////////////////////////////////////////////////////////
bool CObjectReader::ReadPointerHeader(CPointerHeader* pcHeader)
{
	bool	bResult;

	ReturnOnFalse(ReadIdentifier(pcHeader));
	if (pcHeader->meType == OBJECT_POINTER_NULL)
	{
		return true;
	}

	bResult = ReadInt16(&pcHeader->miNumEmbedded);
	bResult &= ReadInt16(&pcHeader->miEmbeddedIndex);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectReader::ReadPointer(CPointer* pObject)
{
	pObject->UnsafeClearObject();
	return ReadDependent(pObject->ObjectPtr(), (CBaseObject*)pObject->Embedding());
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer in pcContaining: *ppcObjectPtr is pointed at what it points to
// once that's known.  A NULL pointer is read as NULL: the place it's read
// into (an array's element) needn't be cleared already.
//
//////////////////////////////////////////////////////////////////////////
bool CObjectReader::ReadDependent(CEmbeddedObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	CPointerHeader	cHeader;
	bool			bResult;

	*ppcObjectPtr = NULL;
	bResult = ReadPointerHeader(&cHeader);
	if (bResult && (cHeader.meType != OBJECT_POINTER_NULL))
	{
		bResult = mpcDependents->AddDependent(&cHeader, ppcObjectPtr, pcContaining, cHeader.miNumEmbedded, cHeader.miEmbeddedIndex);
	}
	cHeader.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// An object that points to pcContaining.
//
//////////////////////////////////////////////////////////////////////////
bool CObjectReader::ReadReverseDependent(CEmbeddedObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	CPointerHeader	cHeader;
	bool			bResult;

	SafeAssign(ppcObjectPtr, NULL);
	bResult = ReadPointerHeader(&cHeader);
	if (bResult && (cHeader.meType != OBJECT_POINTER_NULL))
	{
		bResult = mpcDependents->AddReverseDependent(&cHeader, ppcObjectPtr, pcContaining, cHeader.miNumEmbedded, cHeader.miEmbeddedIndex);
	}
	cHeader.Kill();
	return bResult;
}

