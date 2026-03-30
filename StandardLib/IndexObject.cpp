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
#include "ClassDefines.h"
#include "Objects.h"
#include "ObjectWriter.h"
#include "ObjectReader.h"
#include "IndexObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexObject::~CIndexObject()
{
	Kill();

	//This Destructor code will only be called if the object was allocated on the stack.
	if (muiFlags != OBJECT_FLAGS_CALLED_CONSTRUCTOR)
	{
		if (mpcUnknownsThisIn == NULL)
		{
			if (!HasClass())
			{
				ValidateHasClassFlag(__METHOD__);
			}
		}
		FreePointers();

		ValidateInitCalled();
		ValidateKillCalled();

		muiFlags = 0;
	}
	else
	{
		//Skip all destruction as this object was only constructed to get its ClassName().
		muiFlags = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CIndexObject> CIndexObject::Init(EIndexKeyReverse eKeyReverse)
{
	PreInit();
	CCollection::Init();
	mcIndex.Init(false, true, eKeyReverse);
	PostInit();
	return Ptr<CIndexObject>(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::Class(void)
{
	CCollection::Class();

	U_Unknown(CIndexUnknown, mcIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::Put(char* szKey, CPointer& pObject)
{
	size	uiLength;

	uiLength = StrLen(szKey);
	return Put((uint8*)szKey, uiLength, pObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::Put(uint8* pvKey, size iKeySize, CPointer& pObject)
{
	CBaseObject*		pcPointedTo;
	bool				bResult;
	CEmbeddedObject*	pvObject;

	pvObject = pObject.Object();
	pcPointedTo = (CBaseObject*)mcIndex.Get(pvKey, iKeySize);
	if (pcPointedTo)
	{
		bResult = RemoveObjectTryFree(pcPointedTo, true, false);
		if (!bResult)
		{
			return bResult;
		}
	}
	bResult = mcIndex.Put(pvKey, iKeySize, pvObject);
	if (!bResult)
	{
		return bResult;
	}

	bResult = AddObjectFrom(pvObject, bResult, true);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::Remove(char* szKey)
{
	size	uiLength;

	uiLength = StrLen(szKey);
	return Remove((uint8*)szKey, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::Remove(uint8* pvKey, size iKeySize)
{
	CBaseObject*	pcPointedTo;
	bool			bResult;

	pcPointedTo = (CBaseObject*)mcIndex.Get(pvKey, iKeySize);
	bResult = mcIndex.Remove(pvKey, iKeySize);
	bResult = RemoveObjectTryFree(pcPointedTo, bResult, true);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexObject::NumElements(void)
{
	return mcIndex.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexObject::NonNullElements(void)
{
	return mcIndex.NonNullElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::IsEmpty(void)
{
	return mcIndex.IsEmpty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CIndexObject::Get(char* szKey)
{
	size	uiLength;

	uiLength = StrLen(szKey);
	return Get((uint8*)szKey, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CIndexObject::Get(uint8* pvKey, size iKeySize)
{
	CBaseObject*	pcObject;
	CPointer		pObject;

	pcObject = (CBaseObject*)mcIndex.Get(pvKey, iKeySize);
	if (pcObject)
	{
		pObject.AssignObject(pcObject);
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CIndexObject::StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, bool* bExists)
{
	CBaseObject*	pcObject;
	CPointer		pObject;
	bool			bResult;

	bResult = mcIndex.StartIteration(psIterator, pvKey, piKeySize, iMaxKeySize, (CUnknown**)&pcObject);
	SafeAssign(bExists, bResult);
	if (bResult)
	{
		pObject.AssignObject(pcObject);
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CIndexObject::Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, bool* bExists)
{
	CBaseObject*	pcObject;
	CPointer		pObject;
	bool			bResult;

	bResult = mcIndex.Iterate(psIterator, pvKey, piKeySize, iMaxKeySize, (CUnknown**)&pcObject);
	SafeAssign(bExists, bResult);
	if (bResult)
	{
		pObject.AssignObject(pcObject);
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::SetPointerTosExpectedDistToRoot(int iDistToRoot)
{
	CBaseObject*				pcPointedTo;
	CBaseObject*				pcContainer;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;
	CPointer					pObject;

	pObject = StartIteration(&sIter, NULL, NULL, 0, &bExists);
	while (bExists)
	{
		pcPointedTo = (CBaseObject*)pObject.Object();
		if (iDistToRoot >= ROOT_DIST_TO_ROOT)
		{
			if (pcPointedTo)
			{
				pcContainer = pcPointedTo->GetEmbeddingContainer();
				pcContainer->SetExpectedDistToRoot(iDistToRoot + 1);
			}
		}
		else if (iDistToRoot == UNATTACHED_DIST_TO_ROOT)
		{
			if (pcPointedTo)
			{
				pcContainer = pcPointedTo->GetEmbeddingContainer();
				pcContainer->SetCalculatedDistToRoot();
			}
		}
		else
		{
			gcLogger.Error2(__METHOD__, "Don't know how to set dist to root to [", IntToString(iDistToRoot), "].", NULL);
		}

		pObject = Iterate(&sIter, NULL, NULL, 0, &bExists);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CIndexObject::StartIterationPointer(SIndexTreeMemoryIterator* psIter)
{
	CBaseObject*	pcObject;
	CPointer		pObject;
	bool			bExists;

	bExists = mcIndex.StartIteration(psIter, NULL, NULL, 0, (CUnknown**)&pcObject);
	while (bExists && (pcObject == NULL))
	{
		bExists = mcIndex.Iterate(psIter, NULL, NULL, 0, (CUnknown**)&pcObject);
	}
	if (pcObject)
	{
		pObject.AssignObject(pcObject);
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CIndexObject::IteratePointer(SIndexTreeMemoryIterator* psIter)
{
	CBaseObject*	pcObject;
	CPointer		pObject;
	bool			bExists;

	bExists = mcIndex.Iterate(psIter, NULL, NULL, 0, (CUnknown**)&pcObject);
	while (bExists && (pcObject == NULL))
	{
		bExists = mcIndex.Iterate(psIter, NULL, NULL, 0, (CUnknown**)&pcObject);
	}
	if (pcObject)
	{
		pObject.AssignObject(pcObject);
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::ValidatePointerTos(void)
{
	size 						iCount;
	CBaseObject*				pcPointedTo;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;

	iCount = 0;
	bExists = mcIndex.StartIteration(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		if (pcPointedTo)
		{
			ValidatePointerTo(pcPointedTo);
		}
		bExists = mcIndex.Iterate(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
		iCount++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexUnknown* CIndexObject::GetIndexForTesting(void)
{
	return &mcIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CIndexObject::GetEmbeddedObject(size iIndex)
{
	if (iIndex == 0)
	{
		return this;
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexObject::NumPointerTos(void)
{
	return BaseNumPointerTos();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexObject::BaseNumPointerTos(void)
{
	return NonNullElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos)
{
	CBaseObject*				pcPointedTo;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;

	bExists = mcIndex.StartIteration(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		if (pcPointedTo)
		{
			papcTos->Add(pcPointedTo);
		}
		bExists = mcIndex.Iterate(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::BaseGetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos)
{
	GetPointerTos(papcTos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::ContainsPointerTo(CEmbeddedObject* pcEmbedded)
{
	CBaseObject*				pcPointedTo;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;

	bExists = mcIndex.StartIteration(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		if ((pcPointedTo) && (pcPointedTo == pcEmbedded))
		{
			return true;
		}
		bExists = mcIndex.Iterate(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::RemoveAllPointerTosDontFree(void)
{
	CBaseObject*				pcPointedTo;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;
	bool						bResult;

	bResult = true;
	bExists = mcIndex.StartIteration(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		bResult &= RemovePointerToDontFree(pcPointedTo);
		bExists = mcIndex.Iterate(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
	}
	mcIndex.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::RemoveAllPointerTosTryFree(void)
{
	//Called by KillInternal and RemoveAll.

	CBaseObject*					pcPointedTo;
	SIndexTreeMemoryUnsafeIterator	sIter;
	bool							bExists;
	bool							bResult;

	bResult = true;
	bExists = mcIndex.StartIteration(&sIter, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		bResult &= RemovePointerToTryFree(pcPointedTo);
		bExists = mcIndex.Iterate(&sIter, (CUnknown**)&pcPointedTo);
	}

	mcIndex.ReInit();
	return bResult;  // This can return an error but RemoveAllPointerTosTryFree callers don't handle it.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters)
{
	CBaseObject*				pcPointedTo;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;
	CBaseObject*				pcContainer;

	bExists = mcIndex.StartIteration(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		if (pcPointedTo)
		{
			pcContainer = pcPointedTo->GetEmbeddingContainer();
			pcContainer->CollectAndClearInvalidDistToRootObjects(pcParameters);
		}
		bExists = mcIndex.Iterate(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::Save(CObjectWriter* pcFile)
{
	CBaseObject*				pcPointedTo;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;
	uint8						auiKey[MAX_KEY_SIZE];
	size						uiKeySize;

	ReturnOnFalse(mcIndex.WriteIndexUnknownHeader(pcFile));
	ReturnOnFalse(pcFile->WriteBool(mbSubRoot));

	bExists = mcIndex.StartIteration(&sIter, auiKey, &uiKeySize, MAX_KEY_SIZE, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		ReturnOnFalse(pcFile->WriteSize(uiKeySize));
		ReturnOnFalse(pcFile->WriteData(auiKey, uiKeySize));
		ReturnOnFalse(pcFile->WriteDependent(pcPointedTo));
		bExists = mcIndex.Iterate(&sIter, auiKey, &uiKeySize, MAX_KEY_SIZE, (CUnknown**)&pcPointedTo);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::Load(CObjectReader* pcFile)
{
	size				iCount;
	size				i;
	CEmbeddedObject**	pcPointedTo;
	uint8				auiKey[MAX_KEY_SIZE];
	size				uiKeySize;

	iCount = mcIndex.ReadIndexUnknownHeader(pcFile);
	if (iCount == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	ReturnOnFalse(pcFile->ReadBool(&mbSubRoot));
	
	for (i = 0; i < iCount; i++)
	{
		ReturnOnFalse(pcFile->ReadSize(&uiKeySize));
		if (uiKeySize >= MAX_KEY_SIZE)
		{
			return false;
		}
		ReturnOnFalse(pcFile->ReadData(auiKey, uiKeySize));
		pcPointedTo = (CEmbeddedObject**)mcIndex.PutNode(auiKey, uiKeySize);
		if (pcPointedTo == NULL)
		{
			return false;
		}
		ReturnOnFalse(pcFile->ReadDependent(pcPointedTo, this));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::TouchAll(void)
{
	CBaseObject*				pcObject;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;
	CPointer					pObject;

	bExists = mcIndex.StartIteration(&sIter, NULL, NULL, 0, (CUnknown**)&pcObject);
	while (bExists)
	{
		if (pcObject)
		{
			pObject.AssignObject(pcObject);
		}
		bExists = mcIndex.Iterate(&sIter, NULL, NULL, 0, (CUnknown**)&pcObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::KillAll(void)
{
	CBaseObject*				pcObject;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;
	CPointer					pObject;
	CBaseObject*				pcObjectNext;

	bExists = mcIndex.StartIteration(&sIter, NULL, NULL, 0, (CUnknown**)&pcObject);
	while (bExists)
	{
		//Iterate must be called before the node is removed.
		bExists = mcIndex.Iterate(&sIter, NULL, NULL, 0, (CUnknown**)&pcObjectNext);
		if (pcObject)
		{
			pcObject->Kill();
		}
		pcObject = pcObjectNext;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::FreePointers(void)
{
	CBaseObject::FreePointers();
	mcIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::RemovePointerTo(CEmbeddedObject* pcTo)
{
	mcIndex.Remove(pcTo);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexObject::RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	CBaseObject*					pcPointedTo;
	SIndexTreeMemoryUnsafeIterator	sIter;
	bool							bExists;
	CIndexTreeNodeMemory*			pcNode;
	size 							iCount;

	iCount = 0;
	bExists = mcIndex.StartIteration(&sIter, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		if (pcPointedTo == pcOld)
		{
			pcNode = sIter.pcNode;
			pcNode->SetData(&pcNew, sizeof(CUnknown*));
			iCount++;
		}
		bExists = mcIndex.Iterate(&sIter, (CUnknown**)&pcPointedTo);
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::SetPointedTosDistToRoot(int iDistToRoot)
{
	CBaseObject*				pcPointedTo;
	CBaseObject*				pcContainer;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;

	if (iDistToRoot >= ROOT_DIST_TO_ROOT)
	{
		bExists = mcIndex.StartIteration(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
		while (bExists)
		{
			if (pcPointedTo)
			{
				pcContainer = pcPointedTo->GetEmbeddingContainer();
				pcContainer->SetExpectedDistToRoot(iDistToRoot + 1);
			}
			bExists = mcIndex.Iterate(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
		}
	}
	else if (iDistToRoot == UNATTACHED_DIST_TO_ROOT)
	{
		bExists = mcIndex.StartIteration(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
		while (bExists)
		{
			if (pcPointedTo)
			{
				pcContainer = pcPointedTo->GetEmbeddingContainer();
				pcContainer->SetCalculatedDistToRoot();
			}
			bExists = mcIndex.Iterate(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
		}
	}
	else
	{
		gcLogger.Error2(__METHOD__, "Don't know how to set dist to root to [", IntToString(iDistToRoot), "].", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist)
{
	CBaseObject*				pcPointedTo;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;

	bExists = mcIndex.StartIteration(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		if (pcPointedTo)
		{
			AddExpectedDistToRoot(pcPointedTo, iExpectedDist + 1, pcParameters);
		}
		bExists = mcIndex.Iterate(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
	}
}

