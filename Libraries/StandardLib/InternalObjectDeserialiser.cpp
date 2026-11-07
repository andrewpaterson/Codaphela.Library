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
#include "BaseLib/ErrorTypes.h"
#include "BaseLib/StackMemory.h"
#include "SerialisedObject.h"
#include "ObjectReader.h"
#include "Objects.h"
#include "HollowObject.h"
#include "InternalObjectDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInternalObjectDeserialiser::Init(CObjects* pcObjects, CDataConnection* pcDataConnection, CNamedIndexedObjects* pcMemory)
{
	mpcObjects = pcObjects;
	mpcDataConnection = pcDataConnection;
	mpcMemory = pcMemory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInternalObjectDeserialiser::Kill(void)
{
	mpcDataConnection = NULL;
	mpcMemory = NULL;
	mpcObjects = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CInternalObjectDeserialiser::Read(OIndex oi)
{
	CSerialisedObject*	pcSerialised;
	lsize				uiDataSize;
	CRecordMemory		cMemory;
	CBaseObject*		pcBaseObject;

	pcSerialised = (CSerialisedObject*)GetRecord(mpcDataConnection, oi, &cMemory, &uiDataSize);
	if (pcSerialised == NULL)
	{
		return NULL;
	}

	pcBaseObject = ReadSerialised(pcSerialised, uiDataSize);
	cMemory.Kill();
	return pcBaseObject;
}


//////////////////////////////////////////////////////////////////////////
//
// The record of oi, in pcMemory: on its stack when it fits, else in what
// it allocates.  NULL (with pcMemory killed) if the record can't be read.
// The caller kills pcMemory.
//
//////////////////////////////////////////////////////////////////////////
void* CInternalObjectDeserialiser::GetRecord(CDataConnection* pcDataConnection, OIndex oi, CRecordMemory* pcMemory, lsize* puiDataSize)
{
	void*	pvRecord;

	pvRecord = pcMemory->Init();
	if (!pcDataConnection->Get(oi, puiDataSize, pvRecord, pcMemory->GetStackSize()))
	{
		pcMemory->Kill();
		return NULL;
	}
	if (*puiDataSize > pcMemory->GetStackSize())
	{
		pvRecord = pcMemory->Init(*puiDataSize);
		if (!pcDataConnection->Get(oi, pvRecord))
		{
			pcMemory->Kill();
			return NULL;
		}
	}
	return pvRecord;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CInternalObjectDeserialiser::Read(const char* szObjectName)
{
	OIndex	oi;

	oi = mpcDataConnection->GetIndex(szObjectName);
	if (oi == INVALID_O_INDEX)
	{
		return NULL;
	}
	return Read(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CInternalObjectDeserialiser::ReadSerialised(CSerialisedObject* pcSerialised, lsize iSize)
{
	CObjectReader	cReader;
	CBaseObject*	pcBaseObject;
	bool			bResult;
	CMemoryFile		cMemoryFile;

	cMemoryFile.Init(pcSerialised, iSize);
	cMemoryFile.Open(EFM_Read);
	cReader.Init(&cMemoryFile, this);
	pcBaseObject = cReader.Read();

	if (!pcBaseObject)
	{
		cMemoryFile.Close();
		cReader.Kill();
		cMemoryFile.Kill();
		return NULL;
	}

	bResult = cReader.ReadHeapFroms(pcBaseObject);

	cMemoryFile.Close();
	cReader.Kill();
	cMemoryFile.Kill();

	if (!bResult)
	{
		mpcObjects->ReplaceWithHollow(pcBaseObject);
		return NULL;
	}

	pcBaseObject->SetStored(true);
	return pcBaseObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CInternalObjectDeserialiser::AllocateForDeserialisation(CObjectHeader* pcHeader)
{
	if (pcHeader->meType == OBJECT_POINTER_NULL)
	{
		return NULL;
	}
	return mpcObjects->AllocateForInternalDeserialisation(pcHeader->mszClassName.Text(), pcHeader->IsNamed() ? pcHeader->GetName() : "", pcHeader->moi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CInternalObjectDeserialiser::GetFromMemory(CObjectIdentifier* pcIdentifier)
{
	OIndex	oiNew;

	if (pcIdentifier->meType == OBJECT_POINTER_ID)
	{
		oiNew = pcIdentifier->moi;
		return mpcMemory->Get(oiNew);
	}
	else if (pcIdentifier->meType == OBJECT_POINTER_NAMED)
	{
		//The name is the identity.  If the named object has been replaced (and so has a new index) resolve to the replacement.
		return mpcMemory->Get(pcIdentifier->GetName());
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The object already in memory (perhaps hollow), or else a hollow for it.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CInternalObjectDeserialiser::GetFromMemoryOrAllocateHollow(CObjectIdentifier* pcIdentifier, uint16 iNumEmbedded)
{
	CBaseObject*	pcObject;

	pcObject = GetFromMemory(pcIdentifier);
	if (pcObject)
	{
		return pcObject;
	}
	return mpcObjects->AllocateInternalHollow(pcIdentifier->IsNamed() ? pcIdentifier->GetName() : "", pcIdentifier->moi, iNumEmbedded, pcIdentifier->GetFatSize());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInternalObjectDeserialiser::AddDependent(CObjectIdentifier* pcObjectIdentifier, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex)
{
	CBaseObject*	pcObject;

	pcObject = GetFromMemoryOrAllocateHollow(pcObjectIdentifier, iNumEmbedded);
	if (pcObject == NULL)
	{
		return false;
	}
	return PointTo(pcObject, iEmbeddedIndex, ppcPtrToBeUpdated, pcObjectContainingPtrToBeUpdated);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInternalObjectDeserialiser::AddReverseDependent(CObjectIdentifier* pcObjectIdentifier, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingHeapFrom, uint16 iNumEmbedded, uint16 iEmbeddedIndex)
{
	CBaseObject*	pcObject;

	pcObject = GetFromMemoryOrAllocateHollow(pcObjectIdentifier, iNumEmbedded);
	if (pcObject == NULL)
	{
		return false;
	}
	SafeAssign(ppcPtrToBeUpdated, pcObject);
	return true;
}

