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
	//Only called for an object on the stack.
	DestructOnStack(__METHOD__, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::Init(void)
{
	PreInit();
	CCollection::Init();
	mcMap.Init(false, true);
	mbSorted = true;
	PostInit();
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
bool CMapObject::Put(const CPointer& pKey, const CPointer& pValue)
{
	CBaseObject*		pcOldValue;
	bool				bKeyExisted;
	CBaseObject*		pcKey;
	CBaseObject*		pcValue;

	EnsureSorted();

	pcKey = (CBaseObject*)pKey.Object();
	pcValue = (CBaseObject*)pValue.Object();

	//An existing key keeps its one from.
	bKeyExisted = mcMap.Contains(pcKey);
	pcOldValue = NULL;
	if (bKeyExisted)
	{
		pcOldValue = (CBaseObject*)mcMap.Get(pcKey);
	}

	if (!mcMap.Put(pcKey, pcValue))
	{
		return false;
	}
	RemoveValueLocation(pcOldValue, pcKey);
	AddValueLocation(pcValue, pcKey);

	if (!bKeyExisted)
	{
		AddObjectFrom(pcKey, false);
	}
	return ReplaceObjectFrom(pcValue, pcOldValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapObject::Remove(const CPointer& pKey)
{
	CBaseObject*	pcPointedTo;
	bool			bResult;
	CBaseObject*	pcKey;

	EnsureSorted();

	pcKey = (CBaseObject*)pKey.Object();
	pcPointedTo = (CBaseObject*)mcMap.Get(pcKey);
	if (!mcMap.Remove(pcKey))
	{
		return false;
	}

	RemoveValueLocation(pcPointedTo, pcKey);
	bResult = RemoveObjectTryFree(pcPointedTo, false);
	bResult &= RemoveObjectTryFree(pcKey, true);
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
	//A count, so the order doesn't matter.
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
CPointer CMapObject::Get(const CPointer& pKey)
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
bool CMapObject::Contains(const CPointer& pKey)
{
	EnsureSorted();
	return mcMap.Contains(pKey.Object());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMapEntry CMapObject::StartIteration(SMapIterator* psIterator)
{
	bool			bExists;
	CBaseObject*	pcKey;
	CBaseObject*	pcValue;

	EnsureSorted();

	bExists = mcMap.StartIteration(psIterator, (CUnknown**)&pcKey, (CUnknown**)&pcValue);
	return Entry(bExists, pcKey, pcValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMapEntry CMapObject::Iterate(SMapIterator* psIterator)
{
	bool			bExists;
	CBaseObject*	pcKey;
	CBaseObject*	pcValue;

	bExists = mcMap.Iterate(psIterator, (CUnknown**)&pcKey, (CUnknown**)&pcValue);
	return Entry(bExists, pcKey, pcValue);
}

//////////////////////////////////////////////////////////////////////////
//
// The entry an iteration step found (if bExists), or an empty one.  It is
// returned by value: a CMapEntry is a stack object holding Ptrs to the key
// and value, which keeps them while the caller looks at them.
//
//////////////////////////////////////////////////////////////////////////
CMapEntry CMapObject::Entry(bool bExists, CBaseObject* pcKey, CBaseObject* pcValue)
{
	CMapEntry	cEntry;

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
size CMapObject::BaseNumPointerTos(void)
{
	return NonNullElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::BaseGetPointerTos(CArrayEmbeddedObjectPtr* papcTos)
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
// Called by KillInternal and RemoveAll (bFree), and when killed with the
// objects around it.
//
//////////////////////////////////////////////////////////////////////////
bool CMapObject::ClearPointerTos(bool bFree)
{
	bool			bResult;
	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	SMapIterator	sIter;
	bool			bExists;

	if (mcMap.NumElements() == 0)
	{
		return true;
	}

	//No need to EnsureSorted().
	bResult = true;
	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	while (bExists)
	{
		if (bFree)
		{
			bResult &= RemoveObjectTryFree(pcPointedToKey, false);
			bResult &= RemoveObjectTryFree(pcPointedToValue, false);
		}
		else
		{
			bResult &= UnlinkFromUnlessFreed(pcPointedToKey);
			bResult &= UnlinkFromUnlessFreed(pcPointedToValue);
		}
		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
	}

	mcMap.ReInit();
	KillMemberLocations();
	return bResult;
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

	//Writing the header moves the map's holding entries into its sorted array (CMapBlock::WriteMapBlockHeader), so
	//GetSortedSize counts every entry only after it.
	ReturnOnFalse(mcMap.WriteMapUnknownHeader(pcFile));

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

	KillMemberLocations();
	ReturnOnFalse(mcMap.ReadMapUnknownHeader(pcFile));

	pcMapPtrPtr = mcMap.GetPointerMap();
	uiNumElements = pcMapPtrPtr->GetSortedSize();
	for (i = 0; i < uiNumElements; i++)
	{
		pcMapPtrPtr->PutInSorted(i, (void***)(&pcPointedToKey), (void***)&pcPointedToValue);
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
	mcMap.Sort();
	mbSorted = true;
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
	//Killing a key removes its entry (and can free its value) and killing a value clears it so always start again from the first entry.

	CBaseObject*	pcPointedToKey;
	CBaseObject*	pcPointedToValue;
	SMapIterator	sIter;
	bool			bExists;
	size			uiNumElements;

	for (;;)
	{
		bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
		if (!bExists)
		{
			return;
		}

		uiNumElements = mcMap.NumElements();
		if ((pcPointedToValue) && (pcPointedToValue != pcPointedToKey))
		{
			pcPointedToValue->Kill();
		}
		if (pcPointedToKey)
		{
			pcPointedToKey->Kill();
		}

		if (mcMap.NumElements() >= uiNumElements)
		{
			//The entry could not be removed.
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::PreFields(void)
{
	CCollection::PreFields();

	//mcMap is what every pointer-to walk reads.  Until Init runs it holds whatever was on
	//the stack, so give it a real empty state here;  Init still does the real configuration.
	mcMap._Init();
	mbSorted = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::FreePointers(void)
{
	CBaseObject::FreePointers();
	KillMemberLocations();
	mcMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// pcTo is being killed.  It may be a key, a value or both.
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::RemovePointerTo(CEmbeddedObject* pcTo)
{
	CBaseObject*	pcKey;
	CBaseObject*	pcValue;
	SMapIterator	sMapIter;
	bool			bExists;

	EnsureSorted();

	//Entries where pcTo is the value keep their key but no longer have a value.
	if (GetMemberLocations() != NULL)
	{
		RemoveLocatedMember(pcTo);
	}
	else
	{
		bExists = mcMap.StartIteration(&sMapIter, (CUnknown**)&pcKey, (CUnknown**)&pcValue);
		while (bExists)
		{
			if (pcValue == pcTo)
			{
				mcMap.SetValue(&sMapIter, NULL);
			}
			bExists = mcMap.Iterate(&sMapIter, (CUnknown**)&pcKey, (CUnknown**)&pcValue);
		}
	}

	//The entry where pcTo is the key is removed and its value is no longer pointed to from this map.
	pcValue = (CBaseObject*)mcMap.Get((CUnknown*)pcTo);
	if (mcMap.Remove((CUnknown*)pcTo))
	{
		RemoveValueLocation(pcValue, pcTo);
		RemoveObjectTryFree(pcValue, false);
	}
}

//////////////////////////////////////////////////////////////////////////
//
// pcOld (a hollow object) is replaced by pcNew, as a key and as a value.
//
//////////////////////////////////////////////////////////////////////////
size CMapObject::RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	CPointerLocations*	pcLocations;
	bool				bExists;
	CBaseObject*		pcPointedToKey;
	CBaseObject*		pcPointedToValue;
	SMapIterator		sMapIter;
	size 				iCount;
	CEmbeddedObject*	pcValue;

	if (pcOld == pcNew)
	{
		return 0;
	}

	EnsureSorted();
	pcLocations = GetMemberLocations();
	if (pcLocations == NULL)
	{
		iCount = 0;
		bExists = mcMap.StartIteration(&sMapIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
		while (bExists)
		{
			if (pcPointedToKey == pcOld)
			{
				mcMap.SetKey(&sMapIter, pcNew);
				mbSorted = false;
				iCount++;
			}
			if (pcPointedToValue == pcOld)
			{
				mcMap.SetValue(&sMapIter, pcNew);
				iCount++;
			}
			bExists = mcMap.Iterate(&sMapIter, (CUnknown**)&pcPointedToKey, (CUnknown**)&pcPointedToValue);
		}
		return iCount;
	}

	//Where pcOld is a value, then where it is a key: that entry is put again under pcNew, which keeps the map sorted.
	iCount = RemapLocatedMember(pcOld, pcNew);
	if (mcMap.GetNode(pcOld) != NULL)
	{
		pcValue = (CEmbeddedObject*)mcMap.Get(pcOld);
		mcMap.Remove(pcOld);
		RemoveValueLocation(pcValue, pcOld);
		mcMap.Put(pcNew, pcValue);
		AddValueLocation(pcValue, pcNew);
		iCount++;
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
// uiLocation is the key the member is the value of: the key stays.
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::RemoveMemberAt(ptr uiLocation)
{
	mcMap.Put((CUnknown*)uiLocation, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::ReplaceMemberAt(ptr uiLocation, CEmbeddedObject* pcMember)
{
	mcMap.Put((CUnknown*)uiLocation, pcMember);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapObject::CanLocateMembers(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Where each value is: the key it is under.  Keys are found by the map.
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::AddMemberLocations(CPointerLocations* pcLocations)
{
	CBaseObject*	pcKey;
	CBaseObject*	pcValue;
	SMapIterator	sIter;
	bool			bExists;

	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcKey, (CUnknown**)&pcValue);
	while (bExists)
	{
		if (pcValue != NULL)
		{
			pcLocations->Add(pcValue, (ptr)pcKey);
		}
		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcKey, (CUnknown**)&pcValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapObject::IsMemberAt(CEmbeddedObject* pcMember, ptr uiLocation)
{
	CBaseObject*	pcKey;
	CBaseObject*	pcValue;
	SMapIterator	sIter;
	bool			bExists;

	if (mbSorted)
	{
		return (mcMap.GetNode((CUnknown*)uiLocation) != NULL) && (mcMap.Get((CUnknown*)uiLocation) == (CUnknown*)pcMember);
	}

	//Unsorted it can't be searched by key.  It's a validator's question: it mustn't sort the map to answer it.
	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcKey, (CUnknown**)&pcValue);
	while (bExists)
	{
		if ((ptr)pcKey == uiLocation)
		{
			return pcValue == pcMember;
		}
		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcKey, (CUnknown**)&pcValue);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The non NULL values.
//
//////////////////////////////////////////////////////////////////////////
size CMapObject::NumLocatedMembers(void)
{
	CBaseObject*	pcKey;
	CBaseObject*	pcValue;
	SMapIterator	sIter;
	bool			bExists;
	size			uiCount;

	uiCount = 0;
	bExists = mcMap.StartIteration(&sIter, (CUnknown**)&pcKey, (CUnknown**)&pcValue);
	while (bExists)
	{
		if (pcValue != NULL)
		{
			uiCount++;
		}
		bExists = mcMap.Iterate(&sIter, (CUnknown**)&pcKey, (CUnknown**)&pcValue);
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// pcValue is under pcKey now (nothing is kept until the map is big).
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::AddValueLocation(CEmbeddedObject* pcValue, CEmbeddedObject* pcKey)
{
	if ((mpcMemberLocations != NULL) && (pcValue != NULL))
	{
		mpcMemberLocations->Add(pcValue, (ptr)pcKey);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapObject::RemoveValueLocation(CEmbeddedObject* pcValue, CEmbeddedObject* pcKey)
{
	if ((mpcMemberLocations != NULL) && (pcValue != NULL))
	{
		mpcMemberLocations->Remove(pcValue, (ptr)pcKey);
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


