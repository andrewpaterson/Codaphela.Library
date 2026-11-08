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
#include "Objects.h"
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
// Logs szBefore, the header's identifier (class, index and name) and
// szAfter: a serialised object that couldn't be read.
//
//////////////////////////////////////////////////////////////////////////
static void LogHeaderError(const char* szMethod, const char* szBefore, CObjectHeader* pcHeader, const char* szAfter)
{
	CChars	sz;

	sz.Init();
	gcLogger.Error2(szMethod, szBefore, pcHeader->GetIdentifier(&sz), szAfter, NULL);
	sz.Kill();
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
	CChars			szLengths;

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
		LogHeaderError(__METHOD__, " Could not allocate serialised object [", &sHeader, "] header.");
		sHeader.Kill();
		return NULL;
	}

	bResult = pcObject->LoadManaged(this);
	if (!bResult)
	{
		LogHeaderError(__METHOD__, " Could not load serialised object [", &sHeader, "] fields.");
		sHeader.Kill();
		pcObject->GetObjectsThisIn()->ReplaceWithHollow(pcObject);
		return NULL;
	}

	iLength = mcFile.GetFilePos();
	bResult = (lsize)(iLength - iStart) == iExpectedLength;
	if (!bResult)
	{
		szLengths.Init("] should be [");
		szLengths.Append((uint64)iExpectedLength);
		szLengths.Append("] bytes long but [");
		szLengths.Append((uint64)(iLength - iStart));
		szLengths.Append("] were read.");
		LogHeaderError(__METHOD__, " Serialised object [", &sHeader, szLengths.Text());
		szLengths.Kill();
		sHeader.Kill();
		pcObject->GetObjectsThisIn()->ReplaceWithHollow(pcObject);
		return NULL;
	}
	sHeader.Kill();

	pcObject->SetDirty(false);
	pcObject->Initialised();
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectReader::ReadStoredHeapFroms(CBaseObject* pcThis)
{
	//The holders a serialised object was saved with, without loading its fields: they are skipped.  pcThis (the object in
	//memory) gives the layout of its embedded objects.  The holders go to the dependents' AddReverseDependent.
	filePos	iStart;
	lsize	iLength;
	int32	iMagic;

	iStart = mcFile.GetFilePos();
	if (!ReadSize(&iLength) || !mcFile.Seek(iStart + (filePos)iLength))
	{
		return gcLogger.Error2(__METHOD__, " Could not skip a serialised object's fields.", NULL);
	}
	if (!ReadSize(&iLength) || !mcFile.ReadInt32(&iMagic) || (iMagic != OBJECT_FROM_HEAP))
	{
		return gcLogger.Error2(__METHOD__, " Could not read a serialised object's froms header.", NULL);
	}
	return pcThis->LoadHeapFroms(this);
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
	int32			iMagic;

	iStart = mcFile.GetFilePos();
	if (!ReadSize(&iExpectedLength))
	{
		return gcLogger.Error2(__METHOD__, " Could not read object stream size loading object [", ObjectToString(pcThis), "] 'froms'.", NULL);
	}
	if (!mcFile.ReadInt32(&iMagic) || (iMagic != OBJECT_FROM_HEAP))
	{
		return gcLogger.Error2(__METHOD__, " Could not read object froms magic loading object [", ObjectToString(pcThis), "] 'froms'.", NULL);
	}
	if (!pcThis->LoadHeapFroms(this))
	{
		return gcLogger.Error2(__METHOD__, " Could not LoadHeapFroms() object [", ObjectToString(pcThis), "] 'froms'.", NULL);
	}

	iLength = mcFile.GetFilePos();
	if ((lsize)(iLength - iStart) != iExpectedLength)
	{
		return gcLogger.Error2(__METHOD__, " Size mismatch expected stream length [", LongToString((uint64)iExpectedLength), "] but read length [", LongToString(iLength - iStart), "] reading object froms [", ObjectToString(pcThis), "] 'froms'.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectReader::ReadIdentifier(CObjectIdentifier* pcObjectIdentifier)
{
	pcObjectIdentifier->Init();
	ReturnOnFalse(ReadInt32(&pcObjectIdentifier->meType));

	if (pcObjectIdentifier->meType == OBJECT_POINTER_NULL)
	{
		return true;
	}
	if ((pcObjectIdentifier->meType != OBJECT_POINTER_ID) && (pcObjectIdentifier->meType != OBJECT_POINTER_NAMED))
	{
		return false;
	}

	//An index and a size, then (named) the name.
	ReturnOnFalse(ReadInt64(&pcObjectIdentifier->moi));
	ReturnOnFalse(ReadInt16(&pcObjectIdentifier->muiSize));
	if (pcObjectIdentifier->meType == OBJECT_POINTER_NAMED)
	{
		ReturnOnFalse(pcObjectIdentifier->mszObjectName.ReadChars(this));
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

	pcHeader->Init();
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

