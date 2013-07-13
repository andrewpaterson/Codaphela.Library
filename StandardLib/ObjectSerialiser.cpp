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
#include "ObjectSerialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectSerialiser::Init(CObjectSingleSerialiser* pcSerialiser, CBaseObject* pcObject)
{
	mpcThis = pcObject;
	mpcMemory = MemoryFile();
	mcFile.Init(mpcMemory);
	mpcSerialiser = pcSerialiser;
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
	BOOL		bResult;
	filePos		iLength;

	bResult = mcFile.Open(EFM_Write_Create);
	ReturnOnFalse(bResult);

	bResult = WriteInt(0);
	ReturnOnFalse(bResult);
	bResult = WriteIdentifier(mpcThis);
	ReturnOnFalse(bResult);
	bResult = mpcThis->SaveHeader(this);
	ReturnOnFalse(bResult);
	bResult = mpcThis->Save(this);
	ReturnOnFalse(bResult);
	iLength = mcFile.GetFileLength();
	mcFile.Seek(0);
	bResult = WriteInt((int)iLength);
	ReturnOnFalse(bResult);

	bResult = mcFile.Close();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSerialiser::WritePointer(CPointer pObject)
{
	CEmbeddedObject*	pcEmbeddedObject;

	pcEmbeddedObject = (&pObject);
	return WriteDependent(pcEmbeddedObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSerialiser::WriteDependent(CEmbeddedObject* pcDependent)
{
	BOOL			bResult;
	CBaseObject*	pcContainer;
	int				iEmbeddedIndex;

	if (pcDependent)
	{
		pcContainer = pcDependent->GetEmbeddingContainer();
		iEmbeddedIndex = pcContainer->GetEmbeddedIndex(pcDependent);

		bResult = WriteIdentifier(pcContainer);
		bResult &= WriteInt(iEmbeddedIndex);

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
		bResult = WriteIdentifier(NULL);
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectSerialiser::WriteIdentifier(CBaseObject* pcObject)
{
	OIndex		oi;
	int			c;
	char*		szName;
	
	if (pcObject)
	{
		if (!pcObject->IsNamed())
		{
			c = OBJECT_POINTER_ID;
			WriteInt(c);

			oi = pcObject->GetOI();
			return WriteLong(oi);
		}
		else
		{
			c = OBJECT_POINTER_NAMED;
			WriteInt(c);

			oi = pcObject->GetOI();
			WriteLong(oi);

			szName = pcObject->GetName();
			return WriteString(szName);
		}
	}
	else
	{
		c = OBJECT_POINTER_NULL;
		return WriteInt(c);
	}
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

