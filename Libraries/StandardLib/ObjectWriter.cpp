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
	filePos			iStart;
	filePos			iLength;
	CObjectHeader	sHeader;

	if (!pcThis)
	{
		return gcLogger.Error2(__METHOD__, " Cannot serialise a NULL object.", NULL);
	}

	//The length, written once the object has been, counts from where the object starts.
	iStart = mcFile.GetFilePos();
	if (!WriteSize((lsize)0))
	{
		return gcLogger.Error2(__METHOD__, " Could not write object stream size saving object [", ObjectToString(pcThis), "].", NULL);
	}
	if (!mcFile.WriteInt32(OBJECT_DATA))
	{
		return gcLogger.Error2(__METHOD__, " Could not write object magic saving object [", ObjectToString(pcThis), "].", NULL);
	}

	InitObjectHeader(&sHeader, pcThis, false);
	if (!WriteObjectHeader(&sHeader))
	{
		return gcLogger.Error2(__METHOD__, " Could not write object header saving object [", ObjectToString(pcThis), "].", NULL);
	}
	if (!pcThis->SaveManaged(this))
	{
		return gcLogger.Error2(__METHOD__, " Could not Save() object [", ObjectToString(pcThis), "].", NULL);
	}

	iLength = mcFile.GetFilePos();
	if (!mcFile.Seek(iStart) || !WriteSize((lsize)(iLength - iStart)))
	{
		return gcLogger.Error2(__METHOD__, " Could not write object stream size saving object [", ObjectToString(pcThis), "].", NULL);
	}
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

	iStart = mcFile.GetFilePos();
	if (!WriteSize((lsize)0))
	{
		return gcLogger.Error2(__METHOD__, " Could not write object stream size saving object [", ObjectToString(pcThis), "] 'froms'.", NULL);
	}
	if (!mcFile.WriteInt32(OBJECT_FROM_HEAP))
	{
		return gcLogger.Error2(__METHOD__, " Could not write object froms magic saving object [", ObjectToString(pcThis), "] 'froms'.", NULL);
	}
	if (!pcThis->SaveHeapFroms(this))
	{
		return gcLogger.Error2(__METHOD__, " Could not SaveHeapFroms() object [", ObjectToString(pcThis), "] 'froms'.", NULL);
	}

	iLength = mcFile.GetFilePos();
	if (!mcFile.Seek(iStart) || !WriteSize((lsize)(iLength - iStart)))
	{
		return gcLogger.Error2(__METHOD__, " Could not write object stream size saving object [", ObjectToString(pcThis), "] 'froms'.", NULL);
	}

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
		if ((bFatHollow || gbForceFatHollows) && (pcContainer->ClassSize() >= OBJECT_IDENTIFIER_SIZE_NOT_SET))
		{
			return gcLogger.Error2(__METHOD__, " Object {", ObjectToString(pcContainer), "} of [", SizeToString(pcContainer->ClassSize()), "] bytes is too big to be a fat hollow (65534 at most).", NULL);
		}

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
			//WriteDependent refuses classes too big to say (the only fat hollows written).
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
	ReturnOnFalse(WriteInt32(pcObjectIdentifier->meType));

	if (pcObjectIdentifier->meType != OBJECT_POINTER_NULL)
	{
		ReturnOnFalse(WriteInt64(pcObjectIdentifier->moi));
		ReturnOnFalse(WriteInt16(pcObjectIdentifier->muiSize));

		if (pcObjectIdentifier->meType == OBJECT_POINTER_NAMED)
		{
			ReturnOnFalse(WriteString(pcObjectIdentifier->mszObjectName.Text()));
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

