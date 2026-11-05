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
#include "IndexObject.h"


//Where a member of an index is: a copy of its key.
struct SIndexMemberKey
{
	size	uiKeySize;
	uint8	auiKey[1];
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SIndexMemberKey* NewIndexMemberKey(uint8* pvKey, size uiKeySize)
{
	SIndexMemberKey*	psKey;

	psKey = (SIndexMemberKey*)malloc(sizeof(SIndexMemberKey) + uiKeySize);
	psKey->uiKeySize = uiKeySize;
	memcpy(psKey->auiKey, pvKey, uiKeySize);
	return psKey;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsIndexMemberKey(SIndexMemberKey* psKey, uint8* pvKey, size uiKeySize)
{
	return (psKey->uiKeySize == uiKeySize) && (memcmp(psKey->auiKey, pvKey, uiKeySize) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexObject::~CIndexObject()
{
	//This Destructor code will only be called if the object was allocated on the stack.
	if (muiFlags != OBJECT_FLAGS_CALLED_CONSTRUCTOR)
	{
		Kill();

		if (mpcUnknownsThisIn == NULL)
		{
			if (!HasClass())
			{
				FailHasClassFlag(__METHOD__);
				return;
			}
		}
		FreePointers();

		ValidateInitCalled();
		ValidateKillCalled();

		//Still killed: the objects embedded in this are destroyed after it, and are killed with it.
		muiFlags = OBJECT_FLAGS_CALLED_KILL;
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
void CIndexObject::Init(EIndexKeyReverse eKeyReverse)
{
	PreInit();
	CCollection::Init();
	mcIndex.Init(false, true, eKeyReverse);
	PostInit();
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
bool CIndexObject::Put(const char* szKey, const CPointer& pObject)
{
	size	uiLength;

	uiLength = StrLen(szKey);
	return Put((uint8*)szKey, uiLength, pObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::Put(uint8* pvKey, size iKeySize, const CPointer& pObject)
{
	CBaseObject*		pcPointedTo;
	CEmbeddedObject*	pvObject;

	//The new object's from is added before the old one's is removed: they may be the same object, which removing
	//first would free.
	pvObject = pObject.Object();
	pcPointedTo = (CBaseObject*)mcIndex.Get(pvKey, iKeySize);
	if (!mcIndex.Put(pvKey, iKeySize, pvObject))
	{
		return false;
	}

	RemoveMemberKey(pcPointedTo, pvKey, iKeySize);
	AddMemberKey(pvObject, pvKey, iKeySize);

	AddObjectFrom(pvObject, pcPointedTo == NULL);
	return RemoveObjectTryFree(pcPointedTo, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::Remove(const char* szKey)
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

	pcPointedTo = (CBaseObject*)mcIndex.Get(pvKey, iKeySize);
	if (!mcIndex.Remove(pvKey, iKeySize))
	{
		return false;
	}

	RemoveMemberKey(pcPointedTo, pvKey, iKeySize);
	return RemoveObjectTryFree(pcPointedTo, true);
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
CPointer CIndexObject::Get(const char* szKey)
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
CPointer CIndexObject::StartIterationPointer(SIndexTreeMemoryIterator* psIterator)
{
	CBaseObject*	pcObject;
	CPointer		pObject;
	bool			bExists;

	bExists = mcIndex.StartIteration(psIterator, NULL, NULL, 0, (CUnknown**)&pcObject);
	while (bExists && (pcObject == NULL))
	{
		bExists = mcIndex.Iterate(psIterator, NULL, NULL, 0, (CUnknown**)&pcObject);
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
CPointer CIndexObject::IteratePointer(SIndexTreeMemoryIterator* psIterator)
{
	CBaseObject*	pcObject;
	CPointer		pObject;
	bool			bExists;

	bExists = mcIndex.Iterate(psIterator, NULL, NULL, 0, (CUnknown**)&pcObject);
	while (bExists && (pcObject == NULL))
	{
		bExists = mcIndex.Iterate(psIterator, NULL, NULL, 0, (CUnknown**)&pcObject);
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
size CIndexObject::BaseNumPointerTos(void)
{
	return NonNullElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::BaseGetPointerTos(CArrayEmbeddedObjectPtr* papcTos)
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
// Called by KillInternal and RemoveAll (bFree), and when killed with the
// objects around it.
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::ClearPointerTos(bool bFree)
{
	CBaseObject*					pcPointedTo;
	SIndexTreeMemoryUnsafeIterator	sIter;
	bool							bExists;
	bool							bResult;

	if (mcIndex.NumElements() == 0)
	{
		return true;  
	}

	bResult = true;
	bExists = mcIndex.StartIteration(&sIter, (CUnknown**)&pcPointedTo);
	while (bExists)
	{
		if (bFree)
		{
			bResult &= RemoveObjectTryFree(pcPointedTo, false);
		}
		else
		{
			bResult &= UnlinkFromUnlessFreed(pcPointedTo);
		}
		bExists = mcIndex.Iterate(&sIter, (CUnknown**)&pcPointedTo);
	}

	mcIndex.ReInit();
	KillMemberLocations();
	return bResult;  // This can return an error but ClearPointerTos callers don't handle it.
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
		ReturnOnFalse(pcFile->WriteDependent(pcPointedTo, false));
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
	CEmbeddedObject**	ppcPointedTo;
	uint8				auiKey[MAX_KEY_SIZE];
	size				uiKeySize;

	KillMemberLocations();
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
		ppcPointedTo = (CEmbeddedObject**)mcIndex.PutNode(auiKey, uiKeySize);
		if (ppcPointedTo == NULL)
		{
			return false;
		}
		ReturnOnFalse(pcFile->ReadDependent(ppcPointedTo, this));
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
void CIndexObject::PreFields(void)
{
	CCollection::PreFields();
	mcIndex._Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::FreePointers(void)
{
	CBaseObject::FreePointers();
	KillMemberLocations();
	mcIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// pcTo is being killed: every key it is under goes.
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::RemovePointerTo(CEmbeddedObject* pcTo)
{
	if (GetMemberLocations() == NULL)
	{
		mcIndex.Remove(pcTo);
	}
	else
	{
		RemoveLocatedMember(pcTo);
	}
}

//////////////////////////////////////////////////////////////////////////
//
// pcOld (a hollow object) is replaced by pcNew under every key it is under.
//
//////////////////////////////////////////////////////////////////////////
size CIndexObject::RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	bool							bExists;
	CBaseObject*					pcPointedTo;
	SIndexTreeMemoryUnsafeIterator	sIter;
	CIndexTreeNodeMemory*			pcNode;
	size 							iCount;

	if (GetMemberLocations() != NULL)
	{
		return RemapLocatedMember(pcOld, pcNew);
	}

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
// uiLocation is the member's key.
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::RemoveMemberAt(ptr uiLocation)
{
	SIndexMemberKey*	psKey;

	psKey = (SIndexMemberKey*)uiLocation;
	mcIndex.Remove(psKey->auiKey, psKey->uiKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::ReplaceMemberAt(ptr uiLocation, CEmbeddedObject* pcMember)
{
	SIndexMemberKey*	psKey;

	psKey = (SIndexMemberKey*)uiLocation;
	mcIndex.Put(psKey->auiKey, psKey->uiKeySize, (CUnknown*)pcMember);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::CanLocateMembers(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Every member, with a copy of the key it is under.
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::AddMemberLocations(CPointerLocations* pcLocations)
{
	SIndexTreeMemoryIterator	sIter;
	CEmbeddedObject*			pcMember;
	uint8						auiKey[MAX_KEY_SIZE];
	size						uiKeySize;
	bool						bExists;

	bExists = mcIndex.StartIteration(&sIter, auiKey, &uiKeySize, MAX_KEY_SIZE, (CUnknown**)&pcMember);
	while (bExists)
	{
		if (pcMember != NULL)
		{
			pcLocations->Add(pcMember, (ptr)NewIndexMemberKey(auiKey, uiKeySize));
		}
		bExists = mcIndex.Iterate(&sIter, auiKey, &uiKeySize, MAX_KEY_SIZE, (CUnknown**)&pcMember);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::FreeMemberLocation(ptr uiLocation)
{
	free((SIndexMemberKey*)uiLocation);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexObject::IsMemberAt(CEmbeddedObject* pcMember, ptr uiLocation)
{
	SIndexMemberKey*	psKey;

	psKey = (SIndexMemberKey*)uiLocation;
	return mcIndex.Get(psKey->auiKey, psKey->uiKeySize) == (CUnknown*)pcMember;
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps that pcMember is under the key (if there is one, and the member
// keys are kept: they aren't until the index is big).
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::AddMemberKey(CEmbeddedObject* pcMember, uint8* pvKey, size uiKeySize)
{
	if ((mpcMemberLocations != NULL) && (pcMember != NULL))
	{
		mpcMemberLocations->Add(pcMember, (ptr)NewIndexMemberKey(pvKey, uiKeySize));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets that pcMember is under the key.
//
//////////////////////////////////////////////////////////////////////////
void CIndexObject::RemoveMemberKey(CEmbeddedObject* pcMember, uint8* pvKey, size uiKeySize)
{
	SPointerLocationsIterator	sIter;
	ptr							uiLocation;
	bool						bExists;
	SIndexMemberKey*			psKey;

	if ((mpcMemberLocations == NULL) || (pcMember == NULL))
	{
		return;
	}

	bExists = mpcMemberLocations->StartIteration(&sIter, pcMember, &uiLocation);
	while (bExists)
	{
		psKey = (SIndexMemberKey*)uiLocation;
		if (IsIndexMemberKey(psKey, pvKey, uiKeySize))
		{
			mpcMemberLocations->Remove(pcMember, uiLocation);
			free(psKey);
			return;
		}
		bExists = mpcMemberLocations->Iterate(&sIter, &uiLocation);
	}
}

