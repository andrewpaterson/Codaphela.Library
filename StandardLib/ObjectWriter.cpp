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
#include "ObjectFileGeneral.h"
#include "PointerObject.h"
#include "Object.h"
#include "ObjectWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectWriter::Init(CAbstractFile* pcFile, CDependentWriteObjects* pcDependentObjects)
{
	mpcDependentObjects = pcDependentObjects;

	mcFile.Init(pcFile);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectWriter::Kill(void)
{
	mcFile.Kill();

	mpcDependentObjects = NULL;
}


#define ObjectWriterErrorCheck(result, object, ...) \
if (!result) \
{ \
	CChars			sz; \
\
	sz.Init(); \
	object->GetIdentifier(&sz); \
	gcLogger.Error2(__VA_ARGS__); \
	sz.Kill(); \
	return FALSE; \
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectWriter::Write(CBaseObject* pcThis)
{
	BOOL			bResult;
	filePos			iLength;
	CObjectHeader	sHeader;

	if (!pcThis)
	{
		return gcLogger.Error2(__METHOD__, " Cannot serialse a NULL object.", NULL);
	}

	bResult = WriteInt(0);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not write object steam size saving object [", sz.Text(), "].", NULL);

	bResult = mcFile.WriteInt(OBJECT_DATA);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not write object magic saving object [", sz.Text(), "].", NULL);

	InitObjectHeader(&sHeader, pcThis);
	bResult &= WriteObjectHeader(&sHeader);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not write object header saving object [", sz.Text(), "].", NULL);

	bResult = pcThis->SaveManaged(this);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not Save() object [", sz.Text(), "].", NULL);

	iLength = mcFile.GetFilePos();
	mcFile.Seek(0);
	bResult = WriteInt((int)iLength);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not write object steam size saving object [", sz.Text(), "].", NULL);

	mcFile.Seek(iLength);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectWriter::WriteHeapFroms(CBaseObject* pcThis)
{
	filePos			iStart;
	filePos			iLength;
	BOOL			bResult;

	iStart = mcFile.GetFilePos();

	bResult = WriteInt(0);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not write object steam size saving object [", sz.Text(), "] 'froms'.", NULL);

	bResult = mcFile.WriteInt(OBJECT_FROM_HEAP);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not write object froms magic saving object [", sz.Text(), "] 'froms'.", NULL);

	bResult = pcThis->SaveHeapFroms(this);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not SaveHeapFroms() object [", sz.Text(), "] 'froms'.", NULL);

	iLength = mcFile.GetFilePos();
	mcFile.Seek(iStart);
	bResult = WriteInt((int)(iLength - iStart));
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not write object steam size saving object [", sz.Text(), "] 'froms'.", NULL);

	mcFile.Seek(iLength);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectWriter::WritePointer(CPointer& pObject)
{
	CEmbeddedObject* pcEmbeddedObject;

	pcEmbeddedObject = pObject.Object();
	return WriteDependent(pcEmbeddedObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectWriter::WritePointer(CPointer* pObject)
{
	CEmbeddedObject* pcEmbeddedObject;

	pcEmbeddedObject = pObject->Object();
	return WriteDependent(pcEmbeddedObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectWriter::WriteDependent(CEmbeddedObject* pcDependent)
{
	BOOL				bResult;
	CBaseObject*		pcContainer;
	uint16				iEmbeddedIndex;
	uint16				iNumEmbedded;
	CObjectIdentifier	sIdentifier;

	if (pcDependent)
	{
		pcContainer = pcDependent->GetEmbeddingContainer();
		iEmbeddedIndex = pcContainer->GetEmbeddedIndex(pcDependent);
		iNumEmbedded = pcContainer->GetNumEmbedded();

		InitIdentifier(&sIdentifier, pcContainer);
		bResult = WriteIdentifier(&sIdentifier);
		bResult &= WriteInt(iNumEmbedded);
		bResult &= WriteInt(iEmbeddedIndex);

		if (bResult)
		{
			if (mpcDependentObjects)
			{
				mpcDependentObjects->Add(pcContainer);
			}
		}
		return bResult;
	}
	else
	{
		InitIdentifier(&sIdentifier, NULL);
		bResult = WriteIdentifier(&sIdentifier);
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectWriter::InitObjectHeader(CObjectHeader* psHeader, CBaseObject* pcObject)
{
	const char* szClassName;

	InitIdentifier(psHeader, pcObject);

	if (pcObject)
	{
		szClassName = pcObject->ClassName();
		psHeader->mszClassName.Fake(szClassName);
	}
	else
	{
		psHeader->mszClassName._Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectWriter::InitIdentifier(CObjectIdentifier* psHeader, CBaseObject* pcObject)
{
	OIndex		oi;
	const char* szObjectName;

	if (pcObject)
	{
		oi = pcObject->GetIndex();

		if (!pcObject->IsNamed())
		{
			psHeader->mcType = OBJECT_POINTER_ID;
			psHeader->moi = oi;
			psHeader->mszObjectName._Init();
		}
		else
		{
			szObjectName = pcObject->GetName();
			psHeader->mcType = OBJECT_POINTER_NAMED;
			psHeader->moi = oi;
			psHeader->mszObjectName.Fake(szObjectName);
		}
	}
	else
	{
		psHeader->mcType = OBJECT_POINTER_NULL;
		psHeader->moi = NULL_O_INDEX;
		psHeader->mszObjectName._Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectWriter::WriteObjectHeader(CObjectHeader* psHeader)
{
	ReturnOnFalse(WriteIdentifier(psHeader));
	return WriteString(psHeader->mszClassName.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectWriter::WriteIdentifier(CObjectIdentifier* psIdentifier)
{
	BOOL bResult;

	bResult = WriteInt(psIdentifier->mcType);
	ReturnOnFalse(bResult);

	if (psIdentifier->mcType != OBJECT_POINTER_NULL)
	{
		bResult = WriteInt(psIdentifier->moi);
		ReturnOnFalse(bResult);

		if (psIdentifier->mcType == OBJECT_POINTER_NAMED)
		{
			bResult = WriteString(psIdentifier->mszObjectName.Text());
			ReturnOnFalse(bResult);
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CObjectWriter::Write(const void* pvSource, filePos iSize, filePos iCount)
{
	return mcFile.Write(pvSource, iSize, iCount);
}



