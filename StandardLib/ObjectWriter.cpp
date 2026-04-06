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
#include "ObjectFileGeneral.h"
#include "PointerObject.h"
#include "Object.h"
#include "ObjectWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectWriter::Init(CAbstractFile* pcFile, CDependentWriteObjects* pcDependentObjects)
{
	mpcDependentObjects = pcDependentObjects;

	mcFile.Init(pcFile);
	return true;
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
	return false; \
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectWriter::Write(CBaseObject* pcThis)
{
	bool			bResult;
	size			iLength;
	CObjectHeader	sHeader;

	if (!pcThis)
	{
		return gcLogger.Error2(__METHOD__, " Cannot serialse a NULL object.", NULL);
	}

	bResult = WriteSize(0);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not write object steam size saving object [", sz.Text(), "].", NULL);

	bResult = mcFile.WriteInt32(OBJECT_DATA);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not write object magic saving object [", sz.Text(), "].", NULL);

	InitObjectHeader(&sHeader, pcThis, false);
	bResult &= WriteObjectHeader(&sHeader);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not write object header saving object [", sz.Text(), "].", NULL);

	bResult = pcThis->SaveManaged(this);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not Save() object [", sz.Text(), "].", NULL);

	iLength = (size)mcFile.GetFilePos();
	mcFile.Seek(0);
	bResult = WriteSize(iLength);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not write object steam size saving object [", sz.Text(), "].", NULL);

	mcFile.Seek(iLength);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectWriter::WriteHeapFroms(CBaseObject* pcThis)
{
	filePos			iStart;
	filePos			iLength;
	bool			bResult;

	iStart = mcFile.GetFilePos();

	bResult = WriteSize(0);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not write object steam size saving object [", sz.Text(), "] 'froms'.", NULL);

	bResult = mcFile.WriteInt32(OBJECT_FROM_HEAP);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not write object froms magic saving object [", sz.Text(), "] 'froms'.", NULL);

	bResult = pcThis->SaveHeapFroms(this);
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not SaveHeapFroms() object [", sz.Text(), "] 'froms'.", NULL);

	iLength = mcFile.GetFilePos();
	mcFile.Seek(iStart);
	bResult = WriteSize((size)(iLength - iStart));
	ObjectWriterErrorCheck(bResult, pcThis, __METHOD__, " Could not write object steam size saving object [", sz.Text(), "] 'froms'.", NULL);

	mcFile.Seek(iLength);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectWriter::WritePointer(CPointer& pObject)
{
	CEmbeddedObject* pcEmbeddedObject;

	pcEmbeddedObject = pObject.Object();
	return WriteDependent(pcEmbeddedObject, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectWriter::WritePointer(CPointer* pObject)
{
	CEmbeddedObject* pcEmbeddedObject;

	pcEmbeddedObject = pObject->Object();
	return WriteDependent(pcEmbeddedObject, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectWriter::WriteDependent(CEmbeddedObject* pcDependent, bool bFatHollow)
{
	bool				bResult;
	CBaseObject*		pcContainer;
	size				iEmbeddedIndex;
	size				iNumEmbedded;
	CObjectIdentifier	sIdentifier;

	if (pcDependent)
	{
		pcContainer = pcDependent->GetEmbeddingContainer();
		iEmbeddedIndex = pcContainer->GetEmbeddedIndex(pcDependent);
		iNumEmbedded = pcContainer->NumEmbedded();

		InitIdentifier(&sIdentifier, pcContainer, bFatHollow);
		bResult = WriteIdentifier(&sIdentifier);
		bResult &= WriteInt16((uint16)iNumEmbedded);
		bResult &= WriteInt16((uint16)iEmbeddedIndex);

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
		InitIdentifier(&sIdentifier, NULL, bFatHollow);
		bResult = WriteIdentifier(&sIdentifier);
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectWriter::InitObjectHeader(CObjectHeader* pcHeader, CBaseObject* pcObject, bool bFatHollow)
{
	const char* szClassName;

	InitIdentifier(pcHeader, pcObject, bFatHollow);

	if (pcObject)
	{
		szClassName = pcObject->ClassName();
		pcHeader->mszClassName.Fake(szClassName);
	}
	else
	{
		pcHeader->mszClassName._Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectWriter::InitIdentifier(CObjectIdentifier* pcObjectIdentifier, CBaseObject* pcObject, bool bFatHollow)
{
	OIndex			oi;
	const char*		szObjectName;
	uint16			uiSize;

	if (pcObject)
	{
		if (bFatHollow)
		{
			uiSize = (uint16)pcObject->ClassSize();
		}
		else
		{
			uiSize = OBJECT_IDENTIFIER_SIZE_NOT_SET;
		}

		oi = pcObject->GetIndex();

		if (!pcObject->IsNamed())
		{
			pcObjectIdentifier->Init(oi, uiSize);
		}
		else
		{
			szObjectName = pcObject->GetName();
			pcObjectIdentifier->Init(szObjectName, oi, uiSize);
		}
	}
	else
	{
		pcObjectIdentifier->Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectWriter::WriteObjectHeader(CObjectHeader* pcHeader)
{
	ReturnOnFalse(WriteIdentifier(pcHeader));
	return WriteString(pcHeader->mszClassName.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectWriter::WriteIdentifier(CObjectIdentifier* pcObjectIdentifier)
{
	bool bResult;

	bResult = WriteInt32(pcObjectIdentifier->meType);
	ReturnOnFalse(bResult);

	bResult = WriteInt32(0xffffffff);
	ReturnOnFalse(bResult);

	if (pcObjectIdentifier->meType != OBJECT_POINTER_NULL)
	{
		bResult = WriteInt64(pcObjectIdentifier->moi);
		ReturnOnFalse(bResult);

		bResult = WriteInt16(pcObjectIdentifier->muiSize);
		ReturnOnFalse(bResult);

		if (pcObjectIdentifier->meType == OBJECT_POINTER_NAMED)
		{
			bResult = WriteString(pcObjectIdentifier->mszObjectName.Text());
			ReturnOnFalse(bResult);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObjectWriter::Write(const void* pvSource, size iSize, size iCount)
{
	return mcFile.Write(pvSource, iSize, iCount);
}

