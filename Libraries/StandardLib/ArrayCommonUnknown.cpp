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
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/PrimitiveTypes.h"
#include "SetIterator.h"
#include "Unknowns.h"
#include "ArrayUnknown.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::_Init(void)
{
	mcArray.Init(&gcSystemAllocator);  //Nothing is allocated until something is added, which an array still can be.
	muiFlags = 0;
	miNonNullElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// uiFlags are ARRAY_COMMON_KILL_ELEMENT, _UNIQUE_ONLY, _IGNORE_NULL,
// _PRESERVE_ORDER, _TYPE_KNOWN and _SORT_POINTERS (Flags makes them from
// bools).  An empty array is sorted.
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::Init(uint16 uiFlags, size iChunkSize)
{
	mcArray.Init(&gcSystemAllocator, iChunkSize);
	muiFlags = uiFlags & ARRAY_COMMON_INIT_FLAGS;
	SetFlagShort(&muiFlags, ARRAY_COMMON_IS_PTR_SORTED, FixBool(uiFlags & ARRAY_COMMON_SORT_POINTERS));
	miNonNullElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Init's flags, for a caller that has them as bools.
//
//////////////////////////////////////////////////////////////////////////
uint16 CArrayCommonUnknown::Flags(bool bTypeKnown, bool bKillElements, bool bUnique, bool bIgnoreNull, bool bPreserveOrder, bool bSortPointers)
{
	uint16	uiFlags;

	uiFlags = 0;
	SetFlagShort(&uiFlags, ARRAY_COMMON_TYPE_KNOWN, bTypeKnown);
	SetFlagShort(&uiFlags, ARRAY_COMMON_KILL_ELEMENT, bKillElements);
	SetFlagShort(&uiFlags, ARRAY_COMMON_UNIQUE_ONLY, bUnique);
	SetFlagShort(&uiFlags, ARRAY_COMMON_IGNORE_NULL, bIgnoreNull);
	SetFlagShort(&uiFlags, ARRAY_COMMON_PRESERVE_ORDER, bPreserveOrder);
	SetFlagShort(&uiFlags, ARRAY_COMMON_SORT_POINTERS, bSortPointers);
	return uiFlags;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::Kill(void)
{
	PrivateKill();
	mcArray.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::PrivateKill(void)
{
	size		i;
	CUnknown*	pcUnknown;
	size		uiNumElements;

	if (muiFlags & ARRAY_COMMON_KILL_ELEMENT)
	{
		uiNumElements = mcArray.NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			pcUnknown = *mcArray.Get(i);
			if (pcUnknown)
			{
				pcUnknown->Kill();
			}
		}
	}
	miNonNullElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::TypeKnown(bool bTypeKnown)
{
	SetFlagShort(&muiFlags, ARRAY_COMMON_TYPE_KNOWN, bTypeKnown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::ReInit(void)
{
	PrivateKill();
	mcArray.ReInit();
	SlotsMoved();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::WriteArrayHeader(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteSize(mcArray.NumElements()));
	ReturnOnFalse(pcFile->WriteSize(miNonNullElements));
	ReturnOnFalse(pcFile->WriteSize(mcArray.ChunkSize()));
	return pcFile->WriteInt16((uint16)(muiFlags & ~ARRAY_COMMON_SLOTS_MOVED));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::ReadArrayHeader(CFileReader* pcFile, uint16* piFlags, size* piNumElements)
{
	bool	bTypeKnown;
	size	iChunkSize;
	size	iNonNullElements;

	ReturnOnFalse(pcFile->ReadSize(piNumElements));
	ReturnOnFalse(pcFile->ReadSize(&iNonNullElements));
	ReturnOnFalse(pcFile->ReadSize(&iChunkSize));
	ReturnOnFalse(pcFile->ReadInt16(piFlags));
	if (iNonNullElements > *piNumElements)
	{
		gcLogger.Error2(__METHOD__, " The array has [", SizeToString(iNonNullElements), "] elements that aren't NULL but only [", SizeToString(*piNumElements), "] elements.", NULL);
		return false;
	}

	bTypeKnown = FixBool(*piFlags & ARRAY_COMMON_TYPE_KNOWN);

	//The other flags are set from *piFlags once the elements are read (PostLoad or PostLoadPointers).
	Init(bTypeKnown ? ARRAY_COMMON_TYPE_KNOWN : 0);
	mcArray.SetAllocateSize(iChunkSize);
	miNonNullElements = iNonNullElements;

	mcArray.Resize(*piNumElements);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(WriteArrayHeader(pcFile));
	return WriteElements(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Load(CFileReader* pcFile)
{
	size		iNumElements;
	uint16		iFlags;

	ReturnOnFalse(ReadArrayHeader(pcFile, &iFlags, &iNumElements));
	ReturnOnFalse(ReadElements(pcFile, iNumElements));
	miNonNullElements = CountNonNull();  //What was read, not what the header said.
	PostLoad(iFlags);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::WriteElements(CFileWriter* pcFile)
{
	size	i;
	size	uiNumElements;

	//Every slot is written, NULLs too, so the elements match the count in the header and keep their indices.
	uiNumElements = mcArray.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		ReturnOnFalse(WriteElement(pcFile, *mcArray.Get(i)));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::ReadElements(CFileReader* pcFile, size iNumElements)
{
	size		i;
	CUnknown*	pcUnknown;

	for (i = 0; i < iNumElements; i++)
	{
		ReturnOnFalse(ReadElement(pcFile, &pcUnknown));
		mcArray.Set(i, &pcUnknown);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::PostLoad(uint16 iFlags)
{
	muiFlags = iFlags;

	//If the array was saved sorted it must be sorted after it is loaded.
	//Which might not be the case as pointer position will depend on allocation,
	//not load order.
	if (muiFlags & ARRAY_COMMON_IS_PTR_SORTED)
	{
		Sort();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The flags of an array whose elements were read as pointers to objects
// (hollows, or objects already in memory): their addresses aren't those
// saved, so it isn't sorted until it is sorted again.
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::PostLoadPointers(uint16 iFlags)
{
	muiFlags = iFlags & ~ARRAY_COMMON_IS_PTR_SORTED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::SetChunkSize(size iChunkSize)
{
	mcArray.SetChunkSize(iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::GrowTo(size iNumElements)
{
	mcArray.GrowToAtLeastNumElements(iNumElements, true, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::ReadElement(CFileReader* pcFile, CUnknown** ppcUnknown)
{
	return gcUnknowns.LoadUnknown(pcFile, ppcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::WriteElement(CFileWriter* pcFile, CUnknown* pcUnknown)
{
	return gcUnknowns.SaveUnknown(pcFile, pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonUnknown::NumElements(void)
{
	if (!(muiFlags & ARRAY_COMMON_IGNORE_NULL))
	{
		return mcArray.NumElements();
	}
	else
	{
		return miNonNullElements;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::KillElements(bool bKill)
{
	SetFlagShort(&muiFlags, ARRAY_COMMON_KILL_ELEMENT, bKill);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::PreserveOrder(bool bPreserveOrder)
{
	SetFlagShort(&muiFlags, ARRAY_COMMON_PRESERVE_ORDER, bPreserveOrder);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::Sort(void)
{
	mcArray.QuickSort(ComparePtrPtr);
	SetFlagShort(&muiFlags, ARRAY_COMMON_IS_PTR_SORTED, true);
	SlotsMoved();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Contains(CUnknown* pcUnknown)
{
	size		iIndex;
	
	iIndex = Find(pcUnknown);
	if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonUnknown::Find(CUnknown* pcUnknown)
{
	return FindNext(pcUnknown, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonUnknown::FindNext(CUnknown* pcUnknown, size uiStartIndex)
{
	CUnknown*	pcCurrent;
	size		iIndex;
	bool		bResult;
	size		uiNumElements;

	if ((pcUnknown == NULL) && (muiFlags & ARRAY_COMMON_IGNORE_NULL))
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	if (muiFlags & ARRAY_COMMON_IS_PTR_SORTED)
	{
		if (pcUnknown != NULL)
		{
			CleanNullsBeforeBinarySearch();
			bResult = mcArray.FindInSorted(&pcUnknown, ComparePtrPtr, &iIndex);
			if (bResult && (iIndex >= uiStartIndex))
			{
				return iIndex;
			}
		}
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	else
	{
		uiNumElements = mcArray.NumElements();
		for (iIndex = uiStartIndex; iIndex < uiNumElements; iIndex++)
		{
			pcCurrent = *mcArray.Get(iIndex);
			if (pcCurrent == pcUnknown)
			{
				return iIndex;
			}
		}
		return ARRAY_ELEMENT_NOT_FOUND;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Add(CUnknown* pcUnknown, bool bCleanNullsIfNecessary)
{
	size	uiIndex;

	CleanNullsIfNecessary(bCleanNullsIfNecessary);

	if ((pcUnknown == NULL) && (muiFlags & ARRAY_COMMON_IGNORE_NULL))
	{
		return false;
	}

	if (muiFlags & ARRAY_COMMON_UNIQUE_ONLY)
	{
		if (Contains(pcUnknown))
		{
			return false;
		}
	}

	if (((muiFlags & ARRAY_COMMON_IS_PTR_SORTED) || mcArray.NumElements() < 2) && (muiFlags & ARRAY_COMMON_SORT_POINTERS))
	{
		CleanNullsBeforeBinarySearch();
		uiIndex = mcArray.InsertIntoSorted(ComparePtrPtr, &pcUnknown, false);
		if (uiIndex + 1 != mcArray.NumElements())
		{
			SlotsMoved();
		}
		SetFlagShort(&muiFlags, ARRAY_COMMON_IS_PTR_SORTED, true);
	}
	else
	{
		mcArray.Add(pcUnknown);
		SetFlagShort(&muiFlags, ARRAY_COMMON_IS_PTR_SORTED, false);
	}

	if (pcUnknown != NULL)
	{
		miNonNullElements++;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Puts pcUnknown at iIndex in place of what is there, which is killed if
// the array kills its elements.  False if iIndex is out of range or
// pcUnknown can't go in (a NULL where NULLs are ignored, or an element
// already in a unique array).
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Set(size iIndex, CUnknown* pcUnknown, bool bCleanNullsIfNecessary)
{
	CUnknown*	pcOld;
	size		i;
	size		uiNumElements;

	uiNumElements = mcArray.NumElements();
	if (iIndex >= uiNumElements)
	{
		return false;
	}

	pcOld = *mcArray.Get(iIndex);
	if (pcOld == pcUnknown)
	{
		return true;
	}
	if ((pcUnknown == NULL) && (muiFlags & ARRAY_COMMON_IGNORE_NULL))
	{
		return false;
	}
	if (muiFlags & ARRAY_COMMON_UNIQUE_ONLY)
	{
		//Searched without Contains: a binary search can clean the NULLs, moving the element at iIndex.
		for (i = 0; i < uiNumElements; i++)
		{
			if (*mcArray.Get(i) == pcUnknown)
			{
				return false;
			}
		}
	}

	mcArray.Set(iIndex, &pcUnknown);
	SetFlagShort(&muiFlags, ARRAY_COMMON_IS_PTR_SORTED, false);
	if (pcOld != NULL)
	{
		miNonNullElements--;
	}
	if (pcUnknown != NULL)
	{
		miNonNullElements++;
	}

	//Once it is out of the array, and before the NULLs are cleaned (which moves the elements).
	if ((pcOld != NULL) && (muiFlags & ARRAY_COMMON_KILL_ELEMENT))
	{
		pcOld->Kill();
	}
	CleanNullsIfNecessary(bCleanNullsIfNecessary);
	return true;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::AddAll(CArrayCommonUnknown* pcSource)
{
	size		i;
	CUnknown*	pcUnknown;

	if ((muiFlags & ARRAY_COMMON_KILL_ELEMENT) && (pcSource->muiFlags & ARRAY_COMMON_KILL_ELEMENT))
	{
		//Both would kill the same elements.  The source must give them up first: KillElements(false).
		gcLogger.Error2(__METHOD__, " Both arrays kill their elements.  Call KillElements(false) on the source before adding all of it.", NULL);
		return false;
	}

	if (muiFlags & ARRAY_COMMON_UNIQUE_ONLY)
	{
		for (i = 0; i < pcSource->mcArray.NumElements(); i++)
		{
			pcUnknown = *pcSource->mcArray.Get(i);
			Add(pcUnknown, false);
		}
		CleanNullsIfNecessary();
		return true;
	}
	else
	{
		//NULLs are copied too, but only the elements that aren't NULL are counted.  The appended elements aren't
		//in pointer order.
		mcArray.InsertArrayAfterEnd(&pcSource->mcArray);
		miNonNullElements += pcSource->miNonNullElements;
		SetFlagShort(&muiFlags, ARRAY_COMMON_IS_PTR_SORTED, false);
		CleanNullsIfNecessary();
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Insert(size iIndex, CUnknown* pcUnknown)
{
	if ((pcUnknown == NULL) && (muiFlags & ARRAY_COMMON_IGNORE_NULL))
	{
		return false;
	}

	if (muiFlags & ARRAY_COMMON_UNIQUE_ONLY)
	{
		if (Contains(pcUnknown))
		{
			return false;
		}
	}
	mcArray.InsertAt(&pcUnknown, iIndex);
	if (iIndex + 1 != mcArray.NumElements())
	{
		SlotsMoved();
	}
	SetFlagShort(&muiFlags, ARRAY_COMMON_IS_PTR_SORTED, false);
	if (pcUnknown != NULL)
	{
		miNonNullElements++;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Inserts pcUnknown where its address sorts it, or with bOverwriteExisting
// in place of the same element (which is killed if the array kills its
// elements).  Returns its index.
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonUnknown::InsertIntoSorted(CUnknown* pcUnknown, bool bOverwriteExisting)
{
	size		uiIndex;
	size		uiNumElements;
	CUnknown*	pcReplaced;

	if ((pcUnknown == NULL) && (muiFlags & ARRAY_COMMON_IGNORE_NULL))
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	if (!(muiFlags & ARRAY_COMMON_IS_PTR_SORTED))
	{
		Sort();
	}
	CleanNullsBeforeBinarySearch();

	pcReplaced = NULL;
	if (bOverwriteExisting && mcArray.FindInSorted(&pcUnknown, ComparePtrPtr, &uiIndex))
	{
		pcReplaced = *mcArray.Get(uiIndex);
	}

	uiNumElements = mcArray.NumElements();
	uiIndex = mcArray.InsertIntoSorted(ComparePtrPtr, &pcUnknown, bOverwriteExisting);
	if (mcArray.NumElements() != uiNumElements)
	{
		if (uiIndex + 1 != mcArray.NumElements())
		{
			SlotsMoved();
		}
		if (pcUnknown != NULL)
		{
			miNonNullElements++;
		}
	}
	else if (pcReplaced != pcUnknown)
	{
		if (pcReplaced != NULL)
		{
			miNonNullElements--;
		}
		if (pcUnknown != NULL)
		{
			miNonNullElements++;
		}
		if ((pcReplaced != NULL) && (muiFlags & ARRAY_COMMON_KILL_ELEMENT))
		{
			pcReplaced->Kill();
		}
	}
	return uiIndex;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::CleanNullsIfNecessary(bool bCleanNullsIfNecessary)
{
	if (bCleanNullsIfNecessary)
	{
		CleanNullsIfNecessary();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::CleanNullsIfNecessary(void)
{
	size		uiArrayElements;

	if (muiFlags & ARRAY_COMMON_IGNORE_NULL)
	{
		uiArrayElements = mcArray.NumElements();
		if (uiArrayElements - miNonNullElements >= mcArray.ChunkSize() * 3)
		{
			CleanNulls();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::CleanNullsBeforeBinarySearch(void)
{
	//Removing from an array that ignores NULLs leaves NULLs in place.  A NULL sorts before every other pointer so a sorted array can only be binary searched if it contains no NULLs.
	//Cleaning the NULLs moves elements so a sorted array must not be searched (or added to or removed from) while it is being iterated.
	if ((muiFlags & ARRAY_COMMON_IS_PTR_SORTED) && (muiFlags & ARRAY_COMMON_IGNORE_NULL))
	{
		if (miNonNullElements != mcArray.NumElements())
		{
			CleanNulls();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the slots hold an element.
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonUnknown::CountNonNull(void)
{
	size	i;
	size	uiNumElements;
	size	uiNonNull;

	uiNonNull = 0;
	uiNumElements = mcArray.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		if (*mcArray.Get(i) != NULL)
		{
			uiNonNull++;
		}
	}
	return uiNonNull;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::CleanNulls(void)
{
	size		iDest;
	size		iSource;
	CUnknown*	pcSource;
	size		uiLastNotNull;
	size		uiArrayElements;

	if (miNonNullElements == 0)
	{
		mcArray.SetUsedElements(0);
		return;
	}

	uiArrayElements = mcArray.NumElements();
	uiLastNotNull = uiArrayElements;
	do
	{
		uiLastNotNull--;
		pcSource = *mcArray.Get(uiLastNotNull);
		if (pcSource)
		{
			break;
		}
	}
	while (uiLastNotNull != 0);

	//If uiLastNotNull+1 == miNonNullElements then all the NULLs are at the end.  No need to touch the array.
	if (uiLastNotNull + 1 != miNonNullElements)
	{
		iSource = 0;
		for (iDest = 0; iDest < miNonNullElements; iDest++)
		{
			do
			{
				pcSource = *mcArray.Get(iSource);
				iSource++;
			}
			while (pcSource == NULL);
			mcArray.Set(iDest, &pcSource);
		}
		SlotsMoved();
	}
	mcArray.SetUsedElements(miNonNullElements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonUnknown::Remove(CUnknown* pcUnknown)
{
	size	iIndex;
	bool	bResult;
	size	uiRemoved;

	//An array that ignores NULLs has none to remove.
	if ((muiFlags & ARRAY_COMMON_IGNORE_NULL) && (pcUnknown == NULL))
	{
		return 0;
	}

	uiRemoved = 0;
	iIndex = Find(pcUnknown);
	if (iIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return uiRemoved;
	}

	while (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		bResult = Remove(iIndex);
		if (!bResult)
		{
			gcLogger.Error2(__METHOD__, " Could not remove the element found at [", SizeToString(iIndex), "].", NULL);
			return uiRemoved;
		}
		uiRemoved++;
		iIndex = FindNext(pcUnknown, iIndex);
	}
	return uiRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::RemoveLast(bool bCleanNullsIfNecessary)
{
	CUnknown*	pcUnknown;
	size		i;
	size		uiNumElements;

	uiNumElements = mcArray.NumElements();
	if (muiFlags & ARRAY_COMMON_IGNORE_NULL)
	{
		if (uiNumElements != 0)
		{
			i = uiNumElements;
			do
			{
				i--;
				pcUnknown = *mcArray.Get(i);
				if (pcUnknown)
				{
					return Remove(i, bCleanNullsIfNecessary);
				}
			}
			while (i != 0);
			return false;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (uiNumElements != 0)
		{
			return Remove(uiNumElements - 1, bCleanNullsIfNecessary);
		}
		else
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Remove(size iIndex, bool bCleanNullsIfNecessary)
{
	EArrayUnsetReturn	eUnset;
	bool				bPreserveOrder;

	eUnset = Unset(iIndex);
	if (eUnset == AUR_MustRemove)
	{
		//A sorted array keeps its order, so it stays sorted.
		bPreserveOrder = FixBool(muiFlags & (ARRAY_COMMON_PRESERVE_ORDER | ARRAY_COMMON_IS_PTR_SORTED));
		if (iIndex + 1 != mcArray.NumElements())
		{
			SlotsMoved();
		}
		mcArray.RemoveAt(iIndex, bPreserveOrder);
	}
	CleanNullsIfNecessary(bCleanNullsIfNecessary);
	return eUnset != AUR_NoChange;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EArrayUnsetReturn CArrayCommonUnknown::Unset(size iIndex)
{
	CUnknown*			pcNull;
	CUnknown*			pcUnknown;
	EArrayUnsetReturn	eReturn;

	eReturn = AUR_NoChange;
	if (iIndex >= mcArray.NumElements())
	{
		//Index out of bounds.
		return eReturn;
	}

	pcUnknown = *mcArray.Get(iIndex);
	if (muiFlags & ARRAY_COMMON_IGNORE_NULL)
	{
		if (pcUnknown != NULL)
		{
			pcNull = NULL;
			mcArray.Set(iIndex, &pcNull);
			miNonNullElements--;
			eReturn = AUR_SetNull;
		}
		else
		{
			//Setting NULL and ignoring NULLs.
			return eReturn;
		}
	}
	else
	{
		//Element would be unset here.  NULLs aren't counted.
		if (pcUnknown != NULL)
		{
			miNonNullElements--;
		}
		eReturn = AUR_MustRemove;
	}

	if (muiFlags & ARRAY_COMMON_KILL_ELEMENT)
	{
		if (pcUnknown)
		{
			pcUnknown->Kill();
		}
	}

	//The array should be changed.
	return eReturn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::RemoveDuringIteration(SSetIterator* psIter)
{
	bool		bRemoved;
	CUnknown*	pcUnknown;

	if ((psIter->iIndex == ARRAY_ELEMENT_NOT_FOUND) || (psIter->iIndex >= mcArray.NumElements()))
	{
		return false;
	}

	bRemoved = Remove(psIter->iIndex, false);

	//The next Iterate gives the element now at the iterator's index: removing moved the one after (or the last) there.
	//An array that ignores NULLs leaves a NULL there instead, so the iterator moves on to the next element now.
	if (muiFlags & ARRAY_COMMON_IGNORE_NULL)
	{
		Iterate(psIter, &pcUnknown);
	}
	psIter->bStepFirst = false;
	return bRemoved;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::RemoveEnd(size iIndexInclusive)
{
	size	i;
	bool	bResult;

	i = mcArray.NumElements();
	if ((iIndexInclusive == ARRAY_ELEMENT_NOT_FOUND) || (iIndexInclusive >= i))
	{
		return false;
	}

	//Don't call this if you're using a set.
	bResult = true;
	if (i != iIndexInclusive)
	{
		do
		{
			i--;
			bResult &= Remove(i, false);
		}
		while ((i > iIndexInclusive) && (i != 0));
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::UnsafeSet(size iIndex, CUnknown* pcUnknown)
{
	mcArray.Set(iIndex, &pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CArrayCommonUnknown::UnsafeGet(size iIndex)
{
	return *mcArray.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown** CArrayCommonUnknown::UnsafeGetPointer(size iIndex)
{
	return mcArray.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonUnknown::UnsafeNumElements(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonUnknown::UnsafeNonNullElements(void)
{
	return miNonNullElements;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CArrayCommonUnknown::First(void)
{
	SSetIterator	sIter;
	CUnknown*		pcUnknown;
	bool			bExists;

	bExists = StartIteration(&sIter, &pcUnknown);
	if (bExists)
	{
		return pcUnknown;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CArrayCommonUnknown::Last(void)
{
	CUnknown*	pcUnknown;
	size		i;
	size		uiNumElements;

	uiNumElements = mcArray.NumElements();
	if (uiNumElements != 0)
	{
		pcUnknown = *mcArray.Tail();
		if ((pcUnknown == NULL) && (muiFlags & ARRAY_COMMON_IGNORE_NULL))
		{
			i = uiNumElements - 1;
			if (i != 0)
			{
				do
				{
					i--;
					pcUnknown = *mcArray.Get(i);
					if (pcUnknown)
					{
						return pcUnknown;
					}
				}
				while (i != 0);
			}
		}
		else
		{
			return pcUnknown;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::StartIteration(SSetIterator* psIter, CUnknown** ppcUnknown)
{
	CUnknown*	pcUnknown;

	if (mcArray.NumElements() != 0)
	{
		psIter->iIndex = 0;
		psIter->bStepFirst = true;
		pcUnknown = *mcArray.Get(psIter->iIndex);

		if ((pcUnknown == NULL) && (muiFlags & ARRAY_COMMON_IGNORE_NULL))
		{
			return Iterate(psIter, ppcUnknown);
		}
		SafeAssign(ppcUnknown, pcUnknown);
		return true;
	}
	psIter->bStepFirst = false;
	psIter->iIndex = 0;
	*ppcUnknown = NULL;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Iterate(SSetIterator* psIter, CUnknown** ppcUnknown)
{
	bool		bExists;
	CUnknown*	pcUnknown;

	if (mcArray.NumElements() != 0)
	{
		if (!(muiFlags & ARRAY_COMMON_IGNORE_NULL))
		{
			bExists = PrivateIterate(psIter, &pcUnknown);
			SafeAssign(ppcUnknown, pcUnknown);
			return bExists;
		}
		else
		{
			for (;;)
			{
				bExists = PrivateIterate(psIter, &pcUnknown);
				SafeAssign(ppcUnknown, pcUnknown);
				if (bExists)
				{
					if (pcUnknown)
					{
						return true;
					}
				}
				else
				{
					return false;
				}
			}
		}
	}
	else
	{
		psIter->bStepFirst = false;
		psIter->iIndex = 0;
		SafeAssign(ppcUnknown, NULL);
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::PrivateIterate(SSetIterator* psIter, CUnknown** ppcUnknown)
{
	size	iNum;

	iNum = mcArray.NumElements();
	if (psIter->bStepFirst)
	{
		iNum--;
	}
	if (psIter->iIndex < iNum)
	{
		if (psIter->bStepFirst)
		{
			psIter->iIndex++;
		}
		psIter->bStepFirst = true;
		*ppcUnknown = *mcArray.Get(psIter->iIndex);
		return true;
	}
	else
	{
		psIter->iIndex = mcArray.NumElements();
		*ppcUnknown = NULL;
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::IsKillElements(void)
{
	return FixBool(muiFlags & ARRAY_COMMON_KILL_ELEMENT);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::IsSorted(void)
{
	return FixBool(muiFlags & ARRAY_COMMON_IS_PTR_SORTED);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::CalculateIsSorted(void)
{
	size		ui;
	size		uiNumElements;
	CUnknown*	pcLeft;
	CUnknown*	pcRight;
	int			iResult;

	uiNumElements = mcArray.NumElements();
	if (uiNumElements < 2)
	{
		return true;
	}

	for (ui = 0; ui < uiNumElements - 1; ui++)
	{
		pcLeft = UnsafeGet(ui);
		pcRight = UnsafeGet(ui + 1);

		iResult = ComparePtrPtr(&pcLeft, &pcRight);
		if (iResult > 0)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::IsMustSort(void)
{
	return FixBool(muiFlags & ARRAY_COMMON_SORT_POINTERS);
}


//////////////////////////////////////////////////////////////////////////
//
// An element was changed where it is (a hollow loaded, or remapped): the
// array may no longer be in pointer order.
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::PointersChanged(void)
{
	SetFlagShort(&muiFlags, ARRAY_COMMON_IS_PTR_SORTED, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::IsEmpty(void)
{
	return NumElements() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::IsNotEmpty(void)
{
	return NumElements() != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Removing an element from an array that ignores NULLs leaves a NULL in
// its place: the other elements stay where they are.
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::IsIgnoreNull(void)
{
	return FixBool(muiFlags & ARRAY_COMMON_IGNORE_NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an element has moved to another index since ClearSlotsMoved:
// sorting, inserting, removing from the middle and cleaning NULLs move
// them.  Whoever keeps where the elements are knows its record is stale.
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::HaveSlotsMoved(void)
{
	return FixBool(muiFlags & ARRAY_COMMON_SLOTS_MOVED);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::ClearSlotsMoved(void)
{
	SetFlagShort(&muiFlags, ARRAY_COMMON_SLOTS_MOVED, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::SlotsMoved(void)
{
	SetFlagShort(&muiFlags, ARRAY_COMMON_SLOTS_MOVED, true);
}


