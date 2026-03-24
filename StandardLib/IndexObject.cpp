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
Ptr<CIndexObject> CIndexObject::Init(void)
{
	CCollection::Init();

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
	bResult = mcIndex.Put(pvKey, iKeySize, pvObject);
	bResult = RemoveObjectTryFree(pcPointedTo, bResult);
	bResult = AddObjectFrom(pvObject, bResult);
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
	CEmbeddedObject*			pcPointedTo;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;

	iCount = 0;
	bExists = mcIndex.StartIteration(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		ValidatePointerTo(pcPointedTo);
		bExists = mcIndex.Iterate(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
		iCount++;
	}
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
	CEmbeddedObject*			pcPointedTo;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;

	bExists = mcIndex.StartIteration(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		if (pcPointedTo)
		{
			papcTos->Add(&pcPointedTo);
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
	CEmbeddedObject*			pcPointedTo;
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
void CIndexObject::RemoveAllPointerTosTryFree(void)
{
	//Called by KillInternal.

	CBaseObject*					pcPointedTo;
	SIndexTreeMemoryUnsafeIterator	sIter;
	bool							bExists;
	bool							bResult;
	CBaseObject**					ppcNodePointer;


	bResult = true;
	bExists = mcIndex.StartIteration(&sIter, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		bResult &= RemovePointerToTryFree(pcPointedTo);
		ppcNodePointer = (CBaseObject**)sIter.pcNode->GetNodeData();
		*ppcNodePointer = NULL;
		bExists = mcIndex.Iterate(&sIter, (CUnknown**)&pcPointedTo);
	}

	mcIndex.ReInit();
	//return bResult;  // This can return an error but RemoveAllPointerTosTryFree callers don't handle it.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters)
{
	CEmbeddedObject*			pcPointedTo;
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
	CEmbeddedObject*	pcObject;
	bool				bResult;

	pcObject = (CEmbeddedObject*)mcIndex.Get(pvKey, iKeySize);
	bResult = mcIndex.Remove(pvKey, iKeySize);
	bResult = RemoveObjectTryFree(pcObject, bResult);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::Save(CObjectWriter* pcFile)
{
	return gcLogger.Error2(__METHOD__, " Not yet implemented.", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::Load(CObjectReader* pcFile)
{
	return gcLogger.Error2(__METHOD__, " Not yet implemented.", NULL);
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
	size 						iCount;
	uint8						auiKey[MAX_KEY_SIZE];
	size						uiKeyLength;
	CBaseObject* pcPointedTo;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;

	iCount = 0;
	bExists = mcIndex.StartIteration(&sIter, auiKey, &uiKeyLength, 0, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		if (pcPointedTo)
		{
			if (pcPointedTo == pcOld)
			{
				mcIndex.Put(auiKey, uiKeyLength, pcNew);
				iCount++;
			}
		}
		bExists = mcIndex.Iterate(&sIter, NULL, NULL, 0, (CUnknown**)&pcPointedTo);
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
	CEmbeddedObject*			pcPointedTo;
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

