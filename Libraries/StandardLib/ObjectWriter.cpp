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
#include "ObjectFileGeneral.h"
#include "PointerObject.h"
#include "Object.h"
#include "FatHollowConfiguration.h"
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjectWriter::Write(CBaseObject* pcThis)
{
	bool			bResult;
	filePos			iStart;
	filePos			iLength;
	CObjectHeader	sHeader;

	if (!pcThis)
	{
		return gcLogger.Error2(__METHOD__, " Cannot serialise a NULL object.", NULL);
	}

	//The length, written once the object has been, counts from where the object starts.
	iStart = mcFile.GetFilePos();
	bResult = WriteSize((lsize)0);
	ObjectFileErrorCheck(bResult, pcThis, __METHOD__, " Could not write object stream size saving object [", sz.Text(), "].", NULL);

	bResult = mcFile.WriteInt32(OBJECT_DATA);
	ObjectFileErrorCheck(bResult, pcThis, __METHOD__, " Could not write object magic saving object [", sz.Text(), "].", NULL);

	InitObjectHeader(&sHeader, pcThis, false);
	bResult &= WriteObjectHeader(&sHeader);
	ObjectFileErrorCheck(bResult, pcThis, __METHOD__, " Could not write object header saving object [", sz.Text(), "].", NULL);

	bResult = pcThis->SaveManaged(this);
	ObjectFileErrorCheck(bResult, pcThis, __METHOD__, " Could not Save() object [", sz.Text(), "].", NULL);

	iLength = mcFile.GetFilePos();
	bResult = mcFile.Seek(iStart);
	bResult = bResult && WriteSize((lsize)(iLength - iStart));
	ObjectFileErrorCheck(bResult, pcThis, __METHOD__, " Could not write object stream size saving object [", sz.Text(), "].", NULL);

	return mcFile.Seek(iLength);
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

	bResult = WriteSize((lsize)0);
	ObjectFileErrorCheck(bResult, pcThis, __METHOD__, " Could not write object stream size saving object [", sz.Text(), "] 'froms'.", NULL);

	bResult = mcFile.WriteInt32(OBJECT_FROM_HEAP);
	ObjectFileErrorCheck(bResult, pcThis, __METHOD__, " Could not write object froms magic saving object [", sz.Text(), "] 'froms'.", NULL);

	bResult = pcThis->SaveHeapFroms(this);
	ObjectFileErrorCheck(bResult, pcThis, __METHOD__, " Could not SaveHeapFroms() object [", sz.Text(), "] 'froms'.", NULL);

	iLength = mcFile.GetFilePos();
	bResult = mcFile.Seek(iStart);
	bResult = bResult && WriteSize((lsize)(iLength - iStart));
	ObjectFileErrorCheck(bResult, pcThis, __METHOD__, " Could not write object stream size saving object [", sz.Text(), "] 'froms'.", NULL);

	return mcFile.Seek(iLength);
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
		if (iEmbeddedIndex == ARRAY_ELEMENT_NOT_FOUND)
		{
			return gcLogger.Error2(__METHOD__, " Object {", ObjectToString(pcDependent), "} is not embedded in its container {", ObjectToString(pcContainer), "}.", NULL);
		}
		iNumEmbedded = pcContainer->NumEmbedded();

		InitIdentifier(&sIdentifier, pcContainer, bFatHollow || gbForceFatHollows);
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
lsize CObjectWriter::Write(const void* pvSource, lsize iSize, lsize iCount)
{
	return mcFile.Write(pvSource, iSize, iCount);
}

