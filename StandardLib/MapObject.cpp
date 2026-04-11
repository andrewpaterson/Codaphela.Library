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
#include "MapObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMapObject::~CMapObject()
{
	//This Destructor code will only be called if the object was allocated on the stack.
	if (muiFlags != OBJECT_FLAGS_CALLED_CONSTRUCTOR)
	{
		Kill();

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
Ptr<CMapObject> CMapObject::Init(void)
{
	PreInit();
	CCollection::Init();
	mcMap.Init(false, true);
	mbSorted = true;
	PostInit();
	return Ptr<CMapObject>(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::Class(void)
{
	CCollection::Class();

	U_Unknown(CMapUnknownUnknown, mcMap);
	U_Bool(mbSorted);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapObject::Put(CPointer& pKey, CPointer& pValue)
{
	CBaseObject*		pcPointedTo;
	bool				bResult;
	CBaseObject*		pcKey;
	CBaseObject*		pcValue;

	EnsureSorted();

	pcKey = (CBaseObject*)pKey.Object();
	pcValue = (CBaseObject*)pValue.Object();


	pcPointedTo = (CBaseObject*)mcMap.Get(pcKey);
	if (pcPointedTo)
	{
		bResult = RemoveObjectTryFree(pcPointedTo, true, false);
		if (!bResult)
		{
			return bResult;
		}
	}
	bResult = mcMap.Put(pcKey, pcValue);
	if (!bResult)
	{
		return bResult;
	}

	bResult = AddObjectFrom(pcKey, bResult, false);
	bResult &= AddObjectFrom(pcValue, bResult, true);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapObject::Remove(CPointer& pKey)
{
	CBaseObject*	pcPointedTo;
	bool			bResult;
	CBaseObject*	pcKey;

	EnsureSorted();

	pcKey = (CBaseObject*)pKey.Object();
	pcPointedTo = (CBaseObject*)mcMap.Get(pcKey);
	bResult = mcMap.Remove(pcKey);
	bResult = RemoveObjectTryFree(pcPointedTo, bResult, false);  
	bResult = RemoveObjectTryFree(pcKey, bResult, true);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMapObject::NumElements(void)
{
	return mcMap.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMapObject::NonNullElements(void)
{
	EnsureSorted();

	return mcMap.NonNullElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapObject::IsEmpty(void)
{
	return mcMap.IsEmpty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CMapObject::Get(CPointer& pKey)
{
	CBaseObject*	pcValue;
	CPointer		pValue;
	CBaseObject*	pcKey;

	EnsureSorted();

	pcKey = (CBaseObject*)pKey.Object();
	pcValue = (CBaseObject*)mcMap.Get(pcKey);
	if (pcValue)
	{
		pcValue = pcValue->Dehollow();
		pValue = pcValue;
	}
	return pValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMapEntry CMapObject::StartIteration(SMapIterator* psIterator)
{
	CMapEntry		cEntry;
	bool			bExists;
	CBaseObject*	pcKey;
	CBaseObject*	pcValue;

	EnsureSorted();

	bExists = mcMap.StartIteration(psIterator, (CUnknown**)&pcKey, (CUnknown**)&pcValue);
	if (bExists)
	{
		cEntry.Init(pcKey, pcValue);
	}
	else
	{
		cEntry.Init();
	}
	return cEntry;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMapEntry CMapObject::Iterate(SMapIterator* psIterator)
{
	CMapEntry		cEntry;
	bool			bExists;
	CBaseObject*	pcKey;
	CBaseObject*	pcValue;

	bExists = mcMap.Iterate(psIterator, (CUnknown**)&pcKey, (CUnknown**)&pcValue);
	if (bExists)
	{
		cEntry.Init(pcKey, pcValue);
	}
	else
	{
		cEntry.Init();
	}
	return cEntry;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::SetPointerTosExpectedDistToRoot(int iDistToRoot)
{
	CBaseObject*	pcContainer;
	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	SMapIterator	sIter;
	bool			bExists;

	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	while (bExists)
	{
		if (iDistToRoot >= ROOT_DIST_TO_ROOT)
		{
			pcContainer = pcPointedToKey->GetEmbeddingContainer();
			pcContainer->SetExpectedDistToRoot(iDistToRoot + 1);
			if (pcPointedToValue)
			{
				pcContainer = pcPointedToValue->GetEmbeddingContainer();
				pcContainer->SetExpectedDistToRoot(iDistToRoot + 1);
			}
		}
		else if (iDistToRoot == UNATTACHED_DIST_TO_ROOT)
		{
			pcContainer = pcPointedToKey->GetEmbeddingContainer();
			pcContainer->SetCalculatedDistToRoot();
			if (pcPointedToValue)
			{
				pcContainer = pcPointedToValue->GetEmbeddingContainer();
				pcContainer->SetCalculatedDistToRoot();
			}
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Don't know how to set dist to root to [", IntToString(iDistToRoot), "].", NULL);
		}

		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::ValidatePointerTos(void)
{
	size 			iCount;
	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	SMapIterator	sIter;
	bool			bExists;

	iCount = 0;
	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	while (bExists)
	{
		ValidatePointerTo(pcPointedToKey);
		if (pcPointedToValue)
		{
			ValidatePointerTo(pcPointedToValue);
		}
		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
		iCount++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMapUnknownUnknown* CMapObject::GetUnknownMap(void)
{
	return &mcMap;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CMapObject::GetEmbeddedObject(size iMap)
{
	if (iMap == 0)
	{
		return this;
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMapObject::NumPointerTos(void)
{
	return BaseNumPointerTos();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMapObject::BaseNumPointerTos(void)
{
	return NonNullElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos)
{
	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	SMapIterator	sIter;
	bool			bExists;

	//No need to EnsureSorted().  If the pointers come out in an unsorted order it doesn't matter.
	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	while (bExists)
	{
		papcTos->Add(pcPointedToKey);
		if (pcPointedToValue)
		{
			papcTos->Add(pcPointedToValue);
		}
		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::BaseGetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos)
{
	GetPointerTos(papcTos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapObject::ContainsPointerTo(CEmbeddedObject* pcEmbedded)
{
	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	SMapIterator	sIter;
	bool			bExists;

	//No need to EnsureSorted().
	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	while (bExists)
	{
		if (pcPointedToKey == pcEmbedded)
		{
			return true;
		}
		if ((pcPointedToValue) && (pcPointedToValue == pcEmbedded))
		{
			return true;
		}
		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::RemoveAllPointerTosDontFree(void)
{
	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	SMapIterator	sIter;
	bool			bResult;
	bool			bExists;

	//No need to EnsureSorted().
	bResult = true;
	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	while (bExists)
	{
		bResult &= RemovePointerToDontFree(pcPointedToKey);
		bResult &= RemovePointerToDontFree(pcPointedToValue);
		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	}
	mcMap.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapObject::RemoveAllPointerTosTryFree(void)
{
	//Called by KillInternal and RemoveAll.

	bool			bResult;
	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	SMapIterator	sIter;
	bool			bExists;

	//No need to EnsureSorted().
	bResult = true;
	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	while (bExists)
	{
		bResult &= RemovePointerToTryFree(pcPointedToKey);
		bResult &= RemovePointerToTryFree(pcPointedToValue);
		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	}

	mcMap.ReInit();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters)
{
	CBaseObject*	pcContainer;
	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	SMapIterator	sIter;
	bool			bExists;

	//No need to EnsureSorted().
	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	while (bExists)
	{
		pcContainer = pcPointedToKey->GetEmbeddingContainer();
		pcContainer->CollectAndClearInvalidDistToRootObjects(pcParameters);
		if (pcPointedToValue)
		{
			pcContainer = pcPointedToValue->GetEmbeddingContainer();
			pcContainer->CollectAndClearInvalidDistToRootObjects(pcParameters);
		}
		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapObject::Save(CObjectWriter* pcFile)
{
	void*						pvKey;
	void*						pvValue;
	CBaseObject*				pcPointedToKey;
	CBaseObject*				pcPointedToValue;
	CMapPtrPtr*					pcMapPtrPtr;
	size						uiNumElements;
	size						ui;

	EnsureSorted();

	ReturnOnFalse(mcMap.WriteMapUnknownHeader(pcFile));
	ReturnOnFalse(pcFile->WriteBool(mbSubRoot));

	pcMapPtrPtr = mcMap.GetPointerMap();
	uiNumElements = pcMapPtrPtr->GetSortedSize();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pcMapPtrPtr->GetInSorted(ui, &pvKey, &pvValue);
		pcPointedToKey = *((CBaseObject**)pvKey);
		pcPointedToValue = *((CBaseObject**)pvValue);
		ReturnOnFalse(pcFile->WriteDependent(pcPointedToKey, true));
		ReturnOnFalse(pcFile->WriteDependent(pcPointedToValue, false));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapObject::Load(CObjectReader* pcFile)
{
	CEmbeddedObject**	pcPointedToKey;
	CEmbeddedObject**	pcPointedToValue;
	CMapPtrPtr*			pcMapPtrPtr; 
	size				uiNumElements;
	size				i;

	ReturnOnFalse(mcMap.ReadMapUnknownHeader(pcFile));
	ReturnOnFalse(pcFile->ReadBool(&mbSubRoot));

	pcMapPtrPtr = mcMap.GetPointerMap();
	uiNumElements = pcMapPtrPtr->GetSortedSize();
	for (i = 0; i < uiNumElements; i++)
	{
		pcMapPtrPtr->PutInSorted(i, (void**)(&pcPointedToKey), (void**)&pcPointedToValue);
		ReturnOnFalse(pcFile->ReadDependent((CEmbeddedObject**)pcPointedToKey, this));
		ReturnOnFalse(pcFile->ReadDependent((CEmbeddedObject**)pcPointedToValue, this));
	}

	mbSorted = false;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::Sort(void)
{
	size			uiNumInserted;

	uiNumInserted = mcMap.Sort();

	mbSorted = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CMapObject::Dereference(CEmbeddedObject** ppcObject)
{
	if (*ppcObject)
	{
		*ppcObject = (*ppcObject)->Dehollow();
		mbSorted = false;
		return *ppcObject;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::TouchAll(void)
{
	CPointer		pKey;
	CPointer		pValue;
	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	SMapIterator	sIter;
	bool			bExists;

	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	while (bExists)
	{
		pKey = pcPointedToKey;
		if (pcPointedToValue)
		{
			pValue = pcPointedToValue;
		}
		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::KillAll(void)
{
	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	CBaseObject*	pcPointedToKeyNext;
	CBaseObject*	pcPointedToValueNext;
	SMapIterator	sIter;
	bool			bExists;

	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	while (bExists)
	{
		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcPointedToKeyNext, (CUnknown**)&pcPointedToValueNext);

		pcPointedToKey->Kill();
		if (pcPointedToValue)
		{
			pcPointedToValue->Kill();
		}

		pcPointedToKey = pcPointedToKeyNext;
		pcPointedToValue = pcPointedToValueNext;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::FreePointers(void)
{
	CBaseObject::FreePointers();
	mcMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::RemovePointerTo(CEmbeddedObject* pcTo)
{
	mcMap.Remove(pcTo);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMapObject::RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	SMapIterator	sIter;
	bool			bExists;
	size 			iCount;

	iCount = 0;
	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	while (bExists)
	{
		if (pcPointedToKey == pcOld)
		{
			if (pcOld != pcNew)
			{
				mcMap.SetKey(&sIter, pcNew);
				mbSorted = false;
			}
			iCount++;
		}

		if (pcPointedToValue == pcOld)
		{
			if (pcOld != pcNew)
			{
				mcMap.SetValue(&sIter, pcNew);
			}
			iCount++;
		}

		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::SetPointedTosDistToRoot(int iDistToRoot)
{
	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	SMapIterator	sIter;
	bool			bExists;
	CBaseObject*	pcContainer;

	if (iDistToRoot >= ROOT_DIST_TO_ROOT)
	{
		bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
		while (bExists)
		{
			pcContainer = pcPointedToKey->GetEmbeddingContainer();
			pcContainer->SetExpectedDistToRoot(iDistToRoot + 1);

			if (pcPointedToValue)
			{
				pcContainer = pcPointedToValue->GetEmbeddingContainer();
				pcContainer->SetExpectedDistToRoot(iDistToRoot + 1);
			}
			bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
		}
	}
	else if (iDistToRoot == UNATTACHED_DIST_TO_ROOT)
	{
		bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
		while (bExists)
		{
			pcContainer = pcPointedToKey->GetEmbeddingContainer();
			pcContainer->SetCalculatedDistToRoot();

			if (pcPointedToValue)
			{
				pcContainer = pcPointedToValue->GetEmbeddingContainer();
				pcContainer->SetCalculatedDistToRoot();
			}
			bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
		}
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Don't know how to set dist to root to [", IntToString(iDistToRoot), "].", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist)
{
	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	SMapIterator	sIter;
	bool			bExists;

	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	while (bExists)
	{
		AddExpectedDistToRoot(pcPointedToKey, iExpectedDist + 1, pcParameters);
		if (pcPointedToValue)
		{
			AddExpectedDistToRoot(pcPointedToValue, iExpectedDist + 1, pcParameters);
		}
		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::EnsureSorted(void)
{
	if (!mbSorted)
	{
		Sort();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapObject::IsSorted(void)
{
	return mbSorted;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::ValidateInternalConsistency(void)
{
	bool	bSorted;
	if (mbSorted)
	{
		bSorted = mcMap.IsSorted();
		if (!bSorted)
		{
			gcLogger.Error2(__METHOD__, " Map is not sorted.", NULL);
		}
	}
}
