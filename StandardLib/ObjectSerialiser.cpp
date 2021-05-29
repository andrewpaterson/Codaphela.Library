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
#include "ObjectSerialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSerialiser::Init(CObjectSingleSerialiser* pcSerialiser, CBaseObject* pcObject)
{
	mpcThis = pcObject;
	if (!mpcThis)
	{
		return gcLogger.Error2(__METHOD__, " Cannot serialse a NULL object.", NULL);
	}

	mpcMemory = MemoryFile();
	mcFile.Init(mpcMemory);
	mpcSerialiser = pcSerialiser;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSerialiser::Kill(void)
{
	mpcSerialiser = NULL;
	mpcThis = NULL;
	mcFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSerialiser::Save(void)
{
	BOOL			bResult;
	filePos			iLength;
	CObjectHeader	sHeader;
	CChars			sz;

	bResult = mcFile.Open(EFM_Write_Create);
	if (!bResult)
	{
		sz.Init();
		mpcThis->GetIdentifier(&sz);
		gcLogger.Error2(__METHOD__, " Could not open serialiser to save object [", sz.Text(), "].", NULL);
		sz.Kill();
		return FALSE;
	}

	bResult = WriteInt(0);
	InitObjectHeader(&sHeader, mpcThis);
	bResult &= WriteObjectHeader(&sHeader);
	if (!bResult)
	{
		sz.Init();
		mpcThis->GetIdentifier(&sz);
		gcLogger.Error2(__METHOD__, " Could not write object header saving object [", sz.Text(), "].", NULL);
		sz.Kill();
		return FALSE;
	}

	bResult = mpcThis->SaveManaged(this);
	if (!bResult)
	{
		sz.Init();
		mpcThis->GetIdentifier(&sz);
		gcLogger.Error2(__METHOD__, " Could not Save() object [", sz.Text(), "].", NULL);
		sz.Kill();
		return FALSE;
	}

	iLength = mcFile.GetFileLength();
	mcFile.Seek(0);
	bResult = WriteInt((int)iLength);
	ReturnOnFalse(bResult);

	bResult = mcFile.Close();
	if (!bResult)
	{
		sz.Init();
		mpcThis->GetIdentifier(&sz);
		gcLogger.Error2(__METHOD__, " Could not close serialiser saving object [", sz.Text(), "].", NULL);
		sz.Kill();
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSerialiser::WritePointer(CPointer& pObject)
{
	CEmbeddedObject* pcEmbeddedObject;

	pcEmbeddedObject = pObject.Object();
	return WriteDependent(pcEmbeddedObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSerialiser::WritePointer(CPointer* pObject)
{
	CEmbeddedObject* pcEmbeddedObject;

	pcEmbeddedObject = pObject->Object();
	return WriteDependent(pcEmbeddedObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSerialiser::WriteDependent(CEmbeddedObject* pcDependent)
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
		bResult &= WriteShort(iNumEmbedded);
		bResult &= WriteShort(iEmbeddedIndex);

		if (bResult)
		{
			if (mpcSerialiser)
			{
				mpcSerialiser->AddDependent(pcContainer);
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
void CObjectSerialiser::InitObjectHeader(CObjectHeader* psHeader, CBaseObject* pcObject)
{
	const char* szClassName;

	InitIdentifier(psHeader, pcObject);

	if (pcObject)
	{
		szClassName = mpcThis->ClassName();
		psHeader->mszClassName.Fake(szClassName);
	}
	else
	{
		psHeader->mszClassName.Zero();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSerialiser::InitIdentifier(CObjectIdentifier* psHeader, CBaseObject* pcObject)
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
			psHeader->mszObjectName.Zero();
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
		psHeader->mszObjectName.Zero();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSerialiser::WriteObjectHeader(CObjectHeader* psHeader)
{
	ReturnOnFalse(WriteIdentifier(psHeader));
	return WriteString(psHeader->mszClassName.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSerialiser::WriteIdentifier(CObjectIdentifier* psIdentifier)
{
	BOOL bResult;

	bResult = WriteInt(psIdentifier->mcType);
	ReturnOnFalse(bResult);

	if (psIdentifier->mcType != OBJECT_POINTER_NULL)
	{
		bResult = WriteLong(psIdentifier->moi);
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
filePos CObjectSerialiser::Write(const void* pvSource, filePos iSize, filePos iCount)
{
	return mcFile.Write(pvSource, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CObjectSerialiser::GetData(void)
{
	return mpcMemory->GetBufferPointer();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObjectSerialiser::GetLength(void)
{
	return mpcMemory->GetBufferSize();
}

