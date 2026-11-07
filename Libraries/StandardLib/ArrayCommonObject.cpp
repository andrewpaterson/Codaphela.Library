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
#include "SortPointersHelper.h"
#include "ArrayCommonObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayCommonObject::~CArrayCommonObject()
{
	//Only called for an object on the stack.
	DestructOnStack(__METHOD__, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Init(bool bUnique, bool bIgnoreNull, bool bPreserveOrder, bool bSortPointers)
{
	PreInit();
	CCollection::Init();

	mcArray.Init(false, false, bUnique, bIgnoreNull, bPreserveOrder, CalculateDataCompareForSortPointers(bSortPointers));
	mbSorted = true;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Class(void)
{
	CCollection::Class();
	U_Unknown(CArrayCommonUnknown, mcArray);
	U_Bool(mbSorted);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::PreFields(void)
{
	CCollection::PreFields();
	mcArray._Init();
	mbSorted = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::FreePointers(void)
{
	CBaseObject::FreePointers();
	KillMemberLocations();
	mcArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::UnsafeRemove(CBaseObject* pcObject)
{
	//This is a test method.
	if (pcObject)
	{
		if (RemoveCopies(pcObject))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Save(CObjectWriter* pcFile)
{
	size 				i;
	size 				uiNumElements;
	CEmbeddedObject*	pcPointedTo;
	bool				bFatHollow;

	ReturnOnFalse(mcArray.WriteArrayHeader(pcFile));

	bFatHollow = mcArray.IsMustSort();
	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcPointedTo = (CEmbeddedObject*)mcArray.UnsafeGet(i);
		ReturnOnFalse(pcFile->WriteDependent(pcPointedTo, bFatHollow));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Load(CObjectReader* pcFile)
{
	size 				i;
	uint16 				iFlags;
	size 				uiNumElements;
	CEmbeddedObject**	pcPointedTo;

	KillMemberLocations();
	ReturnOnFalse(mcArray.ReadArrayHeader(pcFile, &iFlags, &uiNumElements));

	for (i = 0; i < uiNumElements; i++)
	{
		pcPointedTo = (CEmbeddedObject**)mcArray.UnsafeGetPointer(i);
		ReturnOnFalse(pcFile->ReadDependent(pcPointedTo, this));
	}

	mbSorted = false;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Sort(void)
{
	mcArray.Sort();
	mbSorted = true;
}


//////////////////////////////////////////////////////////////////////////
//
// Only a set sorted by pointer is put back in order: an array keeps the
// order its objects were added in.
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::EnsureSorted(void)
{
	if (!mbSorted && IsMustSort())
	{
		Sort();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::IsSorted(void)
{
	return mbSorted;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::CalculateIsSorted(void)
{
	return mcArray.CalculateIsSorted();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::IsMustSort(void)
{
	return mcArray.IsMustSort();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Add(CPointer& pObject)
{
	CEmbeddedObject*	pcObject;

	pcObject = pObject.Object();
	return Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Add(CEmbeddedObject* pcObject)
{
	bool				bAdded;

	EnsureSorted();

	bAdded = mcArray.Add(pcObject);
	if (bAdded)
	{
		MemberAdded(pcObject, mcArray.UnsafeNumElements() - 1);
		AddObjectFrom(pcObject, true);
	}
	return bAdded;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds each of pcArray's objects.  True if every one was added: a unique
// array refuses those it has already.
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::AddAll(CArrayCommonObject* pcArray)
{
	CArrayEmbeddedObjectPtr		apcObjects;
	CEmbeddedObject*			pcObject;
	size 						i;
	size 						uiNumElements;
	bool						bIgnoreNull;
	bool						bAllAdded;

	//Every slot is looked at: an array that ignores NULLs has them where objects were removed, and they aren't objects.
	//They are copied first as adding can move them (a sorted array, or pcArray being this).
	apcObjects.Init();
	bIgnoreNull = pcArray->mcArray.IsIgnoreNull();
	uiNumElements = pcArray->mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcObject = (CEmbeddedObject*)pcArray->mcArray.UnsafeGet(i);
		if ((pcObject != NULL) || !bIgnoreNull)
		{
			apcObjects.Add(pcObject);
		}
	}

	bAllAdded = true;
	uiNumElements = apcObjects.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		bAllAdded &= Add(apcObjects.GetPtr(i));
	}
	apcObjects.Kill();
	return bAllAdded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Set(size iIndex, CPointer& pObject)
{
	CBaseObject*		pcPointedTo;
	CEmbeddedObject*	pvObject;

	if (iIndex >= mcArray.UnsafeNumElements())
	{
		return false;
	}

	EnsureSorted();

	//The new object's from is added before the old one's is removed: they may be the same object, which removing
	//first would free.
	pvObject = pObject.Object();
	pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(iIndex);
	if (!mcArray.Set(iIndex, pvObject))
	{
		return false;
	}

	MemberRemoved(pcPointedTo, iIndex);
	MemberAdded(pvObject, iIndex);
	AddObjectFrom(pvObject, pcPointedTo == NULL);
	return RemoveObjectTryFree(pcPointedTo, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CArrayCommonObject::Dereference(CEmbeddedObject** ppcObject)
{
	CEmbeddedObject*	pcOldObject;
	CEmbeddedObject*	pcLoaded;

	pcOldObject = *ppcObject;
	if (pcOldObject)
	{
		//If it can't be loaded the element is left as the hollow, to try again.
		pcLoaded = pcOldObject->Dehollow();
		if (pcLoaded == NULL)
		{
			return NULL;
		}
		*ppcObject = pcLoaded;
		if (pcLoaded != pcOldObject)
		{
			mbSorted = false;
		}
		return pcLoaded;
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
void CArrayCommonObject::GrowTo(size iNumElements)
{
	mcArray.GrowTo(iNumElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Remove(CPointer& pObject)
{
	CEmbeddedObject*	pcObject;

	if (pObject.IsNotNull())
	{
		pcObject = pObject.Object();
		return Remove(pcObject);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Remove(CEmbeddedObject* pcObject)
{
	size	uiRemoved;

	if (pcObject)
	{
		EnsureSorted();

		//Removes every copy of the Object in the array.
		uiRemoved = RemoveCopies(pcObject);
		if (uiRemoved != 0)
		{
			return RemoveObjectCopiesTryFree(pcObject, uiRemoved, true);
		}
		return gcLogger.Error2(__METHOD__, " Could not remove Object {", ObjectToString(pcObject), "} from Array {", ObjectToString(this), "}.", NULL);
	}
	return gcLogger.Error2(__METHOD__, " Could not remove Object {NULL} from Array {", ObjectToString(this), "}.", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Called by KillInternal and RemoveAll (bFree), and when killed with the
// objects around it.
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::ClearPointerTos(bool bFree)
{
	CBaseObject*	pcPointedTo;
	size 			i;
	size 			uiNumElements;
	bool			bResult;

	bResult = true;
	KillMemberLocations();
	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		//The object leaves the array before it is freed, as in Set.  Freeing a later object can walk back through the
		//array and mustn't find the ones already freed.
		pcPointedTo = UnsafeGetObject(i);
		mcArray.UnsafeSet(i, NULL);
		if (bFree)
		{
			bResult &= RemoveObjectTryFree(pcPointedTo, false);
		}
		else
		{
			bResult &= UnlinkFromUnlessFreed(pcPointedTo);
		}
	}
	mcArray.ReInit();
	return bResult;  // This can return an error but ClearPointerTos callers don't handle it.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonObject::NumElements(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::IsEmpty(void)
{
	return mcArray.IsEmpty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonObject::NonNullElements(void)
{
	return mcArray.UnsafeNonNullElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonObject::BaseNumPointerTos(void)
{
	CBaseObject*	pcPointedTo;
	size 			i;
	size 			iCount;
	size 			uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	iCount = 0;
	for (i = 0; i < uiNumElements; i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			iCount++;
		}
	}

	uiNumElements = NonNullElements();
	if (iCount != NonNullElements())
	{
		gcLogger.Error2(__METHOD__, " NumPointerTos [", SizeToString(iCount), "] should equal NonNullElements [", SizeToString(uiNumElements), "].", NULL);
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::RemovePointerTo(CEmbeddedObject* pcTo)
{
	RemoveCopies(pcTo);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::BaseGetPointerTos(CArrayEmbeddedObjectPtr* papcTos)
{
	CEmbeddedObject*	pcPointedTo;
	size 				i;
	size 				uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			papcTos->Add(pcPointedTo);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CArrayCommonObject::UnsafeGetObject(size iIndex)
{
	return (CBaseObject*)mcArray.UnsafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonObject::RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	size 				iCount;
	CEmbeddedObject**	ppcPointedTo;
	size 				i;
	size 				uiNumElements;
	CPointerLocations*	pcLocations;

	if (pcOld == pcNew)
	{
		return 0;
	}

	iCount = 0;
	pcLocations = GetMemberLocations();
	if (pcLocations == NULL)
	{
		uiNumElements = mcArray.UnsafeNumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			ppcPointedTo = (CEmbeddedObject**)mcArray.UnsafeGetPointer(i);
			if (*ppcPointedTo == pcOld)
			{
				*ppcPointedTo = pcNew;
				iCount++;
			}
		}
	}
	else
	{
		iCount = RemapLocatedMember(pcOld, pcNew);
	}

	if (iCount > 0)
	{
		mbSorted = false;
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CArrayCommonObject::GetEmbeddedObject(size iIndex)
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
CPointer CArrayCommonObject::StartIterationPointer(SSetIterator* psIter, bool* pbExists)
{
	CBaseObject*	pcObject;
	CPointer		pObject;
	bool			bExists;

	EnsureSorted();

	bExists = mcArray.StartIteration(psIter, (CUnknown**)&pcObject);
	*pbExists = bExists;
	pObject.AssignObject(pcObject);
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CArrayCommonObject::IteratePointer(SSetIterator* psIter, bool* pbExists)
{
	CBaseObject*	pcObject;
	CPointer		pObject;
	bool			bExists;

	bExists = mcArray.Iterate(psIter, (CUnknown**)&pcObject);
	*pbExists = bExists;
	pObject.AssignObject(pcObject);
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::TouchAll(void)
{
	CBaseObject*	pcObject;
	CPointer		pObject;
	size 			i;
	size 			uiNumElements;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		//Assigned, not AssignObject: the stack from given to the previous object must go with it.
		pcObject = (CBaseObject*)mcArray.UnsafeGet(i);
		pObject = pcObject;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::KillAll(void)
{
	//Killing an Object removes it from this array (and can kill other Objects in it) so always look for the first Object left.

	CBaseObject*	pcObject;
	size 			i;
	size 			uiStart;
	size 			uiNumElements;

	uiStart = 0;
	for (;;)
	{
		pcObject = NULL;
		uiNumElements = mcArray.UnsafeNumElements();
		for (i = uiStart; i < uiNumElements; i++)
		{
			pcObject = (CBaseObject*)mcArray.UnsafeGet(i);
			if (pcObject)
			{
				break;
			}
		}

		if (pcObject == NULL)
		{
			return;
		}

		pcObject->Kill();

		//If the Object was not removed from this array skip over it.
		if ((i < mcArray.UnsafeNumElements()) && (mcArray.UnsafeGet(i) == pcObject))
		{
			uiStart = i + 1;
		}
		else
		{
			uiStart = i;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::InsertAt(size iIndex, CPointer& pObject)
{
	CEmbeddedObject* pcObject;

	EnsureSorted();

	pcObject = pObject.Object();
	return InsertAt(iIndex, pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::InsertAt(size iIndex, CEmbeddedObject* pcObject)
{
	bool				bResult;

	EnsureSorted();

	bResult = mcArray.Insert(iIndex, pcObject);
	if (bResult)
	{
		MemberAdded(pcObject, iIndex);
		AddObjectFrom(pcObject, true);
	}
	return bResult;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::RemoveAt(size iIndex)
{
	CEmbeddedObject*	pcObject;

	if (iIndex < mcArray.UnsafeNumElements())
	{
		EnsureSorted();

		pcObject = UnsafeGetObject(iIndex);
		if (!mcArray.Remove(iIndex))
		{
			return false;
		}

		MemberRemoved(pcObject, iIndex);
		return RemoveObjectTryFree(pcObject, true);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::RemoveEnd(size iIndexInclusive)
{
	size						uiNumElements;
	size						i;
	CEmbeddedObject*			pcObject;
	bool						bResult;
	CArrayEmbeddedObjectPtr		apcRemoved;

	EnsureSorted();

	uiNumElements = mcArray.UnsafeNumElements();
	if (iIndexInclusive >= uiNumElements)
	{
		return false;
	}

	//The members are taken out of the array before any is let go of (as RemoveAt does), so none is freed while the
	//array still holds it.  Every one is let go of, even after one fails.
	apcRemoved.Init();
	for (i = iIndexInclusive; i < uiNumElements; i++)
	{
		pcObject = UnsafeGetObject(i);
		MemberRemoved(pcObject, i);
		apcRemoved.Add(pcObject);
	}
	mcArray.RemoveEnd(iIndexInclusive);

	bResult = true;
	uiNumElements = apcRemoved.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		bResult &= RemoveObjectTryFree(apcRemoved.GetPtr(i), false);
	}
	apcRemoved.Kill();

#ifdef _DEBUG
	ValidateObjectsConsistency(true);
#endif // _DEBUG
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::RemoveAll(CArrayCommonObject* pcArray)
{
	CBaseObject*	pcPointedTo;
	bool			bResult;
	SSetIterator	sIter;
	bool			bExists;
	CPointer		pObject;
	size			uiRemoved;

	bResult = true;
	if (pcArray == this)
	{
		return RemoveAll();
	}
	if (!pcArray->IsEmpty())
	{
		EnsureSorted();

		pObject = pcArray->StartIterationPointer(&sIter, &bExists);
		while (bExists)
		{
			pcPointedTo = pObject.BaseObject();
			if (pcPointedTo)
			{
				uiRemoved = RemoveCopies(pcPointedTo);
				if (uiRemoved != 0)
				{
					bResult &= RemoveObjectCopiesTryFree(pcPointedTo, uiRemoved, false);
				}
			}
			pObject = pcArray->IteratePointer(&sIter, &bExists);
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::RemoveAll(void)
{
	return CCollection::RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::Contains(CPointer& pObject)
{
	CEmbeddedObject* pcObject;

	EnsureSorted();

	pcObject = pObject.Object();
	return mcArray.Contains(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
// Takes every copy of pcObject out of the array (the caller looks after
// its froms).  Returns how many there were: 0 if it wasn't there.
// Where removing leaves a NULL behind the copies are found from the member
// locations; elsewhere removing shifts the elements after it along anyway.
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonObject::RemoveCopies(CEmbeddedObject* pcObject)
{
	CPointerLocations*			pcLocations;
	SPointerLocationsIterator	sIter;
	ptr							uiIndex;
	size						uiRemoved;

	if (!mcArray.IsIgnoreNull())
	{
		pcLocations = CurrentMemberLocations();
		if (pcLocations != NULL)
		{
			while (pcLocations->StartIteration(&sIter, pcObject, &uiIndex))
			{
				pcLocations->Remove(pcObject, uiIndex);
			}
		}
		return mcArray.Remove(pcObject);
	}

	pcLocations = GetMemberLocations();
	if (pcLocations == NULL)
	{
		return mcArray.Remove(pcObject);
	}

	uiRemoved = RemoveLocatedMember(pcObject);
	mcArray.CleanNullsIfNecessary();
	return uiRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::CanLocateMembers(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Where each member is: its index.
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::AddMemberLocations(CPointerLocations* pcLocations)
{
	size				i;
	size				uiNumElements;
	CEmbeddedObject*	pcMember;

	uiNumElements = mcArray.UnsafeNumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcMember = (CEmbeddedObject*)mcArray.UnsafeGet(i);
		if (pcMember != NULL)
		{
			pcLocations->Add(pcMember, i);
		}
	}
	mcArray.ClearSlotsMoved();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::IsMemberAt(CEmbeddedObject* pcMember, ptr uiLocation)
{
	if (uiLocation >= mcArray.UnsafeNumElements())
	{
		return false;
	}
	return mcArray.UnsafeGet((size)uiLocation) == pcMember;
}


//////////////////////////////////////////////////////////////////////////
//
// Only an array that ignores NULLs is removed from by location: a NULL is
// left in the slot, so nothing else moves.
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::RemoveMemberAt(ptr uiLocation)
{
	mcArray.Remove((size)uiLocation, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::ReplaceMemberAt(ptr uiLocation, CEmbeddedObject* pcMember)
{
	mcArray.UnsafeSet((size)uiLocation, pcMember);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonObject::HaveMembersMoved(void)
{
	return mcArray.HaveSlotsMoved();
}


//////////////////////////////////////////////////////////////////////////
//
// The member locations if they are kept and none have moved, without
// making them.
//
//////////////////////////////////////////////////////////////////////////
CPointerLocations* CArrayCommonObject::CurrentMemberLocations(void)
{
	if (HaveMembersMoved())
	{
		return NULL;
	}
	return mpcMemberLocations;
}


//////////////////////////////////////////////////////////////////////////
//
// pcMember was put at uiIndex without moving any other member.
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::MemberAdded(CEmbeddedObject* pcMember, size uiIndex)
{
	CPointerLocations*	pcLocations;

	pcLocations = CurrentMemberLocations();
	if ((pcLocations != NULL) && (pcMember != NULL))
	{
		pcLocations->Add(pcMember, uiIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pcMember was taken from uiIndex without moving any other member.
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::MemberRemoved(CEmbeddedObject* pcMember, size uiIndex)
{
	CPointerLocations*	pcLocations;

	pcLocations = CurrentMemberLocations();
	if ((pcLocations != NULL) && (pcMember != NULL))
	{
		pcLocations->Remove(pcMember, uiIndex);
	}
}

