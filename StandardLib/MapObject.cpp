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
Ptr<CMapObject> CMapObject::Init(void)
{
	PreInit();
	CCollection::Init();
	mcMap.Init(false, true);
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


	pcKey = (CBaseObject*)pKey.Object();
	pcPointedTo = (CBaseObject*)mcMap.Get(pcKey);
	bResult = mcMap.Remove(pcKey);
	bResult = RemoveObjectTryFree(pcPointedTo, bResult, true);  
	//The Key object will be freed (if necessary) when the pKey pointer goes out of scope.
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

	pcKey = (CBaseObject*)pKey.Object();
	pcValue = (CBaseObject*)mcMap.Get(pcKey);
	if (pcValue)
	{
		pValue.AssignObject(pcValue);
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
	CMapEntry		cEntry;

	cEntry = StartIteration(&sIter);
	while (cEntry.Exists())
	{
		pcPointedToKey = (CBaseObject*)cEntry.KeyObject();
		pcPointedToValue = (CBaseObject*)cEntry.ValueObject();
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
			gcLogger.Error2(__METHOD__, "Don't know how to set dist to root to [", IntToString(iDistToRoot), "].", NULL);
		}

		cEntry = Iterate(&sIter);
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
	CMapEntry		cEntry;

	iCount = 0;
	cEntry = StartIteration(&sIter);
	while (cEntry.Exists())
	{
		pcPointedToKey = (CBaseObject*)cEntry.KeyObject();
		pcPointedToValue = (CBaseObject*)cEntry.ValueObject();

		ValidatePointerTo(pcPointedToKey);
		if (pcPointedToValue)
		{
			ValidatePointerTo(pcPointedToValue);
		}
		cEntry = Iterate(&sIter);
		iCount++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMapUnknownUnknown* CMapObject::GetMapForTesting(void)
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
	CMapEntry		cEntry;

	cEntry = StartIteration(&sIter);
	while (cEntry.Exists())
	{
		pcPointedToKey = (CBaseObject*)cEntry.KeyObject();
		pcPointedToValue = (CBaseObject*)cEntry.ValueObject();

		papcTos->Add(pcPointedToKey);
		if (pcPointedToValue)
		{
			papcTos->Add(pcPointedToValue);
		}
		cEntry = Iterate(&sIter);
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
	CMapEntry		cEntry;

	cEntry = StartIteration(&sIter);
	while (cEntry.Exists())
	{
		pcPointedToKey = (CBaseObject*)cEntry.KeyObject();
		pcPointedToValue = (CBaseObject*)cEntry.ValueObject();

		if (pcPointedToKey == pcEmbedded)
		{
			return true;
		}
		if ((pcPointedToValue) && (pcPointedToValue == pcEmbedded))
		{
			return true;
		}
		cEntry = Iterate(&sIter);
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
	CMapEntry		cEntry;
	bool			bResult;

	bResult = true;
	cEntry = StartIteration(&sIter);
	while (cEntry.Exists())
	{
		pcPointedToKey = (CBaseObject*)cEntry.KeyObject();
		pcPointedToValue = (CBaseObject*)cEntry.ValueObject();
		
		bResult &= RemovePointerToDontFree(pcPointedToKey);
		bResult &= RemovePointerToDontFree(pcPointedToKey);
		cEntry = Iterate(&sIter);
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
	CMapEntry		cEntry;

	bResult = true;
	cEntry = StartIteration(&sIter);
	while (cEntry.Exists())
	{
		pcPointedToKey = (CBaseObject*)cEntry.KeyObject();
		pcPointedToValue = (CBaseObject*)cEntry.ValueObject();
		
		bResult &= RemovePointerToTryFree(pcPointedToKey);
		bResult &= RemovePointerToTryFree(pcPointedToValue);
		cEntry = Iterate(&sIter);
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
	CMapEntry		cEntry;

	cEntry = StartIteration(&sIter);
	while (cEntry.Exists())
	{
		pcPointedToKey = (CBaseObject*)cEntry.KeyObject();
		pcPointedToValue = (CBaseObject*)cEntry.ValueObject();

		pcContainer = pcPointedToKey->GetEmbeddingContainer();
		pcContainer->CollectAndClearInvalidDistToRootObjects(pcParameters);
		if (pcPointedToValue)
		{
			pcContainer = pcPointedToValue->GetEmbeddingContainer();
			pcContainer->CollectAndClearInvalidDistToRootObjects(pcParameters);
		}
		cEntry = Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapObject::Save(CObjectWriter* pcFile)
{
	//CBaseObject*				pcPointedTo;
	//SMapIterator				sIter;
	//bool						bExists;
	//uint8						auiKey[MAX_KEY_SIZE];
	//size						uiKeySize;

	//ReturnOnFalse(mcMap.WriteMapUnknownHeader(pcFile));
	//ReturnOnFalse(pcFile->WriteBool(mbSubRoot));

	//bExists = mcMap.StartIteration(&sIter, auiKey, &uiKeySize, MAX_KEY_SIZE, (CUnknown**)&pcPointedTo);
	//while (bExists)
	//{
	//	ReturnOnFalse(pcFile->WriteSize(uiKeySize));
	//	ReturnOnFalse(pcFile->WriteData(auiKey, uiKeySize));
	//	ReturnOnFalse(pcFile->WriteDependent(pcPointedTo));
	//	bExists = mcMap.Iterate(&sIter, auiKey, &uiKeySize, MAX_KEY_SIZE, (CUnknown**)&pcPointedTo);
	//}
	//return true;

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapObject::Load(CObjectReader* pcFile)
{
	//size				iCount;
	//size				i;
	//CEmbeddedObject**	pcPointedTo;
	//uint8				auiKey[MAX_KEY_SIZE];
	//size				uiKeySize;

	//iCount = mcMap.ReadMapUnknownHeader(pcFile);
	//if (iCount == ARRAY_ELEMENT_NOT_FOUND)
	//{
	//	return false;
	//}
	//ReturnOnFalse(pcFile->ReadBool(&mbSubRoot));
	//
	//for (i = 0; i < iCount; i++)
	//{
	//	ReturnOnFalse(pcFile->ReadSize(&uiKeySize));
	//	if (uiKeySize >= MAX_KEY_SIZE)
	//	{
	//		return false;
	//	}
	//	ReturnOnFalse(pcFile->ReadData(auiKey, uiKeySize));
	//	pcPointedTo = (CEmbeddedObject**)mcMap.PutNode(auiKey, uiKeySize);
	//	if (pcPointedTo == NULL)
	//	{
	//		return false;
	//	}
	//	ReturnOnFalse(pcFile->ReadDependent(pcPointedTo, this));
	//}
	//return true;
	return false;
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
	CMapEntry		cEntry;

	cEntry = StartIteration(&sIter);
	while (cEntry.Exists())
	{
		pcPointedToKey = (CBaseObject*)cEntry.KeyObject();
		pcPointedToValue = (CBaseObject*)cEntry.ValueObject();

		pKey.AssignObject(pcPointedToKey);
		if (pcPointedToValue)
		{
			pValue.AssignObject(pcPointedToValue);
		}
		cEntry = Iterate(&sIter);
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
	SMapIterator	sIter;
	CMapEntry		cEntry;

	cEntry = StartIteration(&sIter);
	while (cEntry.Exists())
	{
		pcPointedToKey = (CBaseObject*)cEntry.KeyObject();
		pcPointedToValue = (CBaseObject*)cEntry.ValueObject();

		cEntry = Iterate(&sIter);

		pcPointedToKey->Kill();
		if (pcPointedToValue)
		{
			pcPointedToValue->Kill();
		}
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
	CMapEntry		cEntry;
	size 			iCount;

	iCount = 0;
	cEntry = StartIteration(&sIter);
	while (cEntry.Exists())
	{
		pcPointedToKey = (CBaseObject*)cEntry.KeyObject();
		pcPointedToValue = (CBaseObject*)cEntry.ValueObject();

		if (pcPointedToKey == pcOld)
		{
			mcMap.Remove(pcOld);
			mcMap.Put(pcNew, pcPointedToValue);
			iCount++;
		}

		if (pcPointedToValue == pcOld)
		{
			mcMap.Put(pcPointedToKey, pcNew);
			iCount++;
		}

		cEntry = Iterate(&sIter);
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
	CMapEntry		cEntry;
	CBaseObject*	pcContainer;

	if (iDistToRoot >= ROOT_DIST_TO_ROOT)
	{
		cEntry = StartIteration(&sIter);
		while (cEntry.Exists())
		{
			pcPointedToKey = (CBaseObject*)cEntry.KeyObject();
			pcPointedToValue = (CBaseObject*)cEntry.ValueObject();

			cEntry = Iterate(&sIter);

			pcContainer = pcPointedToKey->GetEmbeddingContainer();
			pcContainer->SetExpectedDistToRoot(iDistToRoot + 1);

			if (pcPointedToValue)
			{
				pcContainer = pcPointedToValue->GetEmbeddingContainer();
				pcContainer->SetExpectedDistToRoot(iDistToRoot + 1);
			}
			cEntry = Iterate(&sIter);
		}
	}
	else if (iDistToRoot == UNATTACHED_DIST_TO_ROOT)
	{
		cEntry = StartIteration(&sIter);
		while (cEntry.Exists())
		{
			pcPointedToKey = (CBaseObject*)cEntry.KeyObject();
			pcPointedToValue = (CBaseObject*)cEntry.ValueObject();

			cEntry = Iterate(&sIter);

			pcContainer = pcPointedToKey->GetEmbeddingContainer();
			pcContainer->SetCalculatedDistToRoot();

			if (pcPointedToValue)
			{
				pcContainer = pcPointedToValue->GetEmbeddingContainer();
				pcContainer->SetCalculatedDistToRoot();
			}
			cEntry = Iterate(&sIter);
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
void CMapObject::UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist)
{
	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	SMapIterator	sIter;
	CMapEntry		cEntry;

	cEntry = StartIteration(&sIter);
	while (cEntry.Exists())
	{
		pcPointedToKey = (CBaseObject*)cEntry.KeyObject();
		pcPointedToValue = (CBaseObject*)cEntry.ValueObject();

		cEntry = Iterate(&sIter);

		AddExpectedDistToRoot(pcPointedToKey, iExpectedDist + 1, pcParameters);
		if (pcPointedToValue)
		{
			AddExpectedDistToRoot(pcPointedToValue, iExpectedDist + 1, pcParameters);
		}
		cEntry = Iterate(&sIter);
	}
}

