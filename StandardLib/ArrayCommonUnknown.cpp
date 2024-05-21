/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "Unknowns.h"
#include "ArrayUnknown.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::Init(bool bTypeKnown, bool bKillElements, bool bUnique, bool bIgnoreNull, bool bPreserveOrder)
{
	mcArray.Init();
	miFlags = 0;
	SetFlagShort(&miFlags, ARRAY_COMMOM_KILL_ELEMENT, bKillElements);
	SetFlagShort(&miFlags, ARRAY_COMMOM_UNIQUE_ONLY, bUnique);
	SetFlagShort(&miFlags, ARRAY_COMMOM_IGNORE_NULL, bIgnoreNull);
	SetFlagShort(&miFlags, ARRAY_COMMOM_PRESERVE_ORDER, bPreserveOrder);
	SetFlagShort(&miFlags, ARRAY_COMMOM_TYPE_KNOWN, bTypeKnown);
	miNonNullElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::Init(bool bTypeKnown, bool bKillElements, bool bUnique, bool bIgnoreNull, bool bPreserveOrder, size iChunkSize)
{
	mcArray.Init(&gcSystemAllocator, iChunkSize);
	miFlags = 0;
	SetFlagShort(&miFlags, ARRAY_COMMOM_KILL_ELEMENT, bKillElements);
	SetFlagShort(&miFlags, ARRAY_COMMOM_UNIQUE_ONLY, bUnique);
	SetFlagShort(&miFlags, ARRAY_COMMOM_IGNORE_NULL, bIgnoreNull);
	SetFlagShort(&miFlags, ARRAY_COMMOM_PRESERVE_ORDER, bPreserveOrder);
	SetFlagShort(&miFlags, ARRAY_COMMOM_TYPE_KNOWN, bTypeKnown);
	miNonNullElements = 0;
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

	if (miFlags & ARRAY_COMMOM_KILL_ELEMENT)
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
	SetFlagShort(&miFlags, ARRAY_COMMOM_TYPE_KNOWN, bTypeKnown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::ReInit(void)
{
	PrivateKill();
	mcArray.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::SaveArrayHeader(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteSize(mcArray.NumElements()));
	ReturnOnFalse(pcFile->WriteSize(miNonNullElements));
	ReturnOnFalse(pcFile->WriteSize(mcArray.ChunkSize()));
	ReturnOnFalse(pcFile->WriteShort(miFlags));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::LoadArrayHeader(CFileReader* pcFile, uint16* piFlags, size* piNumElements)
{
	bool	bTypeKnown;
	size	iChunkSize;
	size	iNonNullElements;

	ReturnOnFalse(pcFile->ReadSize(piNumElements));
	ReturnOnFalse(pcFile->ReadSize(&iNonNullElements));
	ReturnOnFalse(pcFile->ReadSize(&iChunkSize));
	ReturnOnFalse(pcFile->ReadShort(piFlags));

	bTypeKnown = FixBool(*piFlags & ARRAY_COMMOM_TYPE_KNOWN);

	//These are all set to false because the flags will be fixed later.
	Init(bTypeKnown, false, false, false, false);
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
	SSetIterator	sIter;
	CUnknown*		pcUnknown;
	size				iCount;

	iCount = 0;

	ReturnOnFalse(SaveArrayHeader(pcFile));

	pcUnknown = StartIteration(&sIter);
	while (pcUnknown)
	{
		ReturnOnFalse(SaveElement(pcFile, pcUnknown));
		pcUnknown = Iterate(&sIter);
		iCount++;
	}

	if (iCount != NumElements())
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Load(CFileReader* pcFile)
{
	size		iNumElements;
	uint16		iFlags;
	size		i;
	CUnknown*	pcUnknown;

	ReturnOnFalse(LoadArrayHeader(pcFile, &iFlags, &iNumElements));

	for (i = 0; i < iNumElements; i++)
	{
		ReturnOnFalse(LoadElement(pcFile, &pcUnknown));
		mcArray.Set(i, &pcUnknown);
	}

	PostLoad(iFlags);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::PostLoad(uint16 iFlags)
{
	miFlags = iFlags;
	if (miFlags & ARRAY_COMMOM_IS_PTR_SORTED)
	{
		Sort();
	}
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
bool CArrayCommonUnknown::LoadElement(CFileReader* pcFile, CUnknown** ppcUnknown)
{
	return gcUnknowns.LoadUnknown(pcFile, ppcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::SaveElement(CFileWriter* pcFile, CUnknown* pcUnknown)
{
	return gcUnknowns.SaveUnknown(pcFile, pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayCommonUnknown::NumElements(void)
{
	if (!(miFlags & ARRAY_COMMOM_IGNORE_NULL))
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
	SetFlagShort(&miFlags, ARRAY_COMMOM_KILL_ELEMENT, bKill);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::UniqueElements(bool bUnique)
{
	if (bUnique && (!(miFlags & ARRAY_COMMOM_UNIQUE_ONLY)))
	{
		//MakeUnique();
	}
	SetFlagShort(&miFlags, ARRAY_COMMOM_UNIQUE_ONLY, bUnique);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::IgnoreNullElements(bool bIgnoreNull)
{
	if (!bIgnoreNull && (miFlags & ARRAY_COMMOM_IGNORE_NULL))
	{
		//RemoveNulls();
	}
	SetFlagShort(&miFlags, ARRAY_COMMOM_IGNORE_NULL, bIgnoreNull);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::PreserveOrder(bool bPreserveOrder)
{
	SetFlagShort(&miFlags, ARRAY_COMMOM_PRESERVE_ORDER, bPreserveOrder);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::Sort(void)
{
	mcArray.QuickSort(ComparePtrPtr);
	SetFlagShort(&miFlags, ARRAY_COMMOM_IS_PTR_SORTED, true);
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
	CUnknown*		pcCurrent;
	size				iIndex;
	bool			bResult;

	if ((pcUnknown == NULL) && (miFlags & ARRAY_COMMOM_IGNORE_NULL))
	{
		return -1;
	}

	if (miFlags & ARRAY_COMMOM_IS_PTR_SORTED)
	{
		bResult = mcArray.FindInSorted(&pcUnknown, ComparePtrPtr, &iIndex);
		if (bResult)
		{
			return iIndex;
		}
		return -1;
	}
	else
	{
		for (iIndex = 0; iIndex < mcArray.NumElements(); iIndex++)
		{
			pcCurrent = *mcArray.Get(iIndex);
			if (pcCurrent == pcUnknown)
			{
				return iIndex;
			}
		}
		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Add(CUnknown* pcUnknown, bool bCleanNullsIfNecessary)
{
	CleanNullsIfNecessary(bCleanNullsIfNecessary);

	if ((pcUnknown == NULL) && (miFlags & ARRAY_COMMOM_IGNORE_NULL))
	{
		return false;
	}

	if (miFlags & ARRAY_COMMOM_UNIQUE_ONLY)
	{
		if (Contains(pcUnknown))
		{
			return false;
		}
	}
	mcArray.Add(&pcUnknown);
	SetFlagShort(&miFlags, ARRAY_COMMOM_IS_PTR_SORTED, false);

	if (pcUnknown != NULL)
	{
		miNonNullElements++;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Set(size iIndex, CUnknown* pcUnknown, bool bCleanNullsIfNecessary)
{
	EArrayUnsetReturn	eUnset;

	if ((pcUnknown == NULL) && (miFlags & ARRAY_COMMOM_IGNORE_NULL))
	{
		CleanNullsIfNecessary(bCleanNullsIfNecessary);
		return false;
	}

	if (miFlags & ARRAY_COMMOM_UNIQUE_ONLY)
	{
		if (Contains(pcUnknown))
		{
			CleanNullsIfNecessary(bCleanNullsIfNecessary);
			return false;
		}
	}

	eUnset = Unset(iIndex);
	CleanNullsIfNecessary(bCleanNullsIfNecessary);

	mcArray.Set(iIndex, &pcUnknown);
	SetFlagShort(&miFlags, ARRAY_COMMOM_IS_PTR_SORTED, false);

	if (pcUnknown != NULL)
	{
		miNonNullElements++;
	}
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

	if (miFlags & ARRAY_COMMOM_UNIQUE_ONLY)
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
		mcArray.InsertArrayAfterEnd(&pcSource->mcArray);
		miNonNullElements += pcSource->NumElements();
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
	if ((pcUnknown == NULL) && (miFlags & ARRAY_COMMOM_IGNORE_NULL))
	{
		return false;
	}

	if (miFlags & ARRAY_COMMOM_UNIQUE_ONLY)
	{
		if (Contains(pcUnknown))
		{
			return false;
		}
	}
	mcArray.InsertAt(&pcUnknown, iIndex);
	SetFlagShort(&miFlags, ARRAY_COMMOM_IS_PTR_SORTED, false);
	miNonNullElements++;
	return true;
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
	size		iDest;
	size		iSource;
	CUnknown*	pcSource;
	size		uiLastNotNull;
	size		uiArrayElements;

	if (miFlags & ARRAY_COMMOM_IGNORE_NULL)
	{
		if (miNonNullElements != 0)
		{
			uiArrayElements = mcArray.NumElements();
			if (uiArrayElements - miNonNullElements >= mcArray.ChunkSize() * 3)
			{
				if (uiArrayElements != 0)
				{
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
				}

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
				}
				mcArray.SetUsedElements(miNonNullElements);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Remove(CUnknown* pcUnknown)
{
	size	iIndex;

	if ((miFlags & ARRAY_COMMOM_IGNORE_NULL) && (pcUnknown == NULL))
	{
		return false;
	}

	iIndex = Find(pcUnknown);
	if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		return Remove(iIndex);
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
bool CArrayCommonUnknown::RemoveLast(bool bCleanNullsIfNecessary)
{
	CUnknown*	pcUnknown;
	size		i;
	size		uiNumElements;

	uiNumElements = mcArray.NumElements();
	if (miFlags & ARRAY_COMMOM_IGNORE_NULL)
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

	eUnset = Unset(iIndex);
	if (eUnset == AUR_NoChange)
	{
		CleanNullsIfNecessary(bCleanNullsIfNecessary);
		return false;
	}
	else if (eUnset == AUR_SetNull)
	{
		CleanNullsIfNecessary(bCleanNullsIfNecessary);
		return true;
	}
	else if (eUnset == AUR_MustRemove)
	{
		mcArray.RemoveAt(iIndex, miFlags & ARRAY_COMMOM_PRESERVE_ORDER);
		CleanNullsIfNecessary(bCleanNullsIfNecessary);
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
//bool CArrayCommonUnknown::Remove(size iIndex, bool bCleanNullsIfNecessary)
//{
//	CUnknown*	pcNull;
//	CUnknown*	pcUnknown;
//
//	if (iIndex >= mcArray.NumElements())
//	{
//		if (bCleanNullsIfNecessary)
//		{
//			CleanNullsIfNecessary();
//		}
//		return false;
//	}
//
//	pcUnknown = *mcArray.Get(iIndex);
//	if (miFlags & ARRAY_COMMOM_IGNORE_NULL)
//	{
//		if (pcUnknown != NULL)
//		{
//			pcNull = NULL;
//			mcArray.Set(iIndex, &pcNull);
//			miNonNullElements--;
//		}
//		else
//		{
//			if (bCleanNullsIfNecessary)
//			{
//				CleanNullsIfNecessary();
//			}
//			return false;
//		}
//	}
//	else
//	{
//		mcArray.RemoveAt(iIndex, miFlags & ARRAY_COMMOM_PRESERVE_ORDER);
//		miNonNullElements--;
//	}
//
//	if (miFlags & ARRAY_COMMOM_KILL_ELEMENT)
//	{
//		if (pcUnknown)
//		{
//			pcUnknown->Kill();
//		}
//	}
//
//	if (bCleanNullsIfNecessary)
//	{
//		CleanNullsIfNecessary();
//	}
//	return true;
//}


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
	if (miFlags & ARRAY_COMMOM_IGNORE_NULL)
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
		//Element would be unset here.
		miNonNullElements--;
		eReturn = AUR_MustRemove;
	}

	if (miFlags & ARRAY_COMMOM_KILL_ELEMENT)
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
	bool	bRemoved;

	if ((mcArray.NumElements() > psIter->iIndex) && (psIter->iIndex != ARRAY_ELEMENT_NOT_FOUND))
	{
		bRemoved = Remove(psIter->iIndex, false);
		if (psIter->iIndex > 0)
		{
			if (miFlags & ARRAY_COMMOM_IGNORE_NULL)
			{
				Iterate(psIter);
				psIter->bStepFirst = false;
			}
			else
			{
				psIter->iIndex--;
				psIter->bStepFirst = false;
			}
		}
		else
		{
			if (miFlags & ARRAY_COMMOM_IGNORE_NULL)
			{
				Iterate(psIter);
				psIter->bStepFirst = false;
			}
			else
			{
				psIter->bStepFirst = false;
			}
		}
		return bRemoved;
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
bool CArrayCommonUnknown::RemoveEnd(size iIndexInclusive)
{
	size		i;
	bool	bResult;

	if (iIndexInclusive == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}

	//Don't call this if you're using a set.
	bResult = true;
	i = mcArray.NumElements();
	if (i != iIndexInclusive)
	{
		do
		{
			i--;
			bResult &= Remove(i, false);
		}
		while (i > iIndexInclusive);
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

	return StartIteration(&sIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CArrayCommonUnknown::Last(void)
{
	CUnknown*	pcUnknown;
	size			i;
	size		uiNumElements;

	uiNumElements = mcArray.NumElements();
	if (uiNumElements != 0)
	{
		pcUnknown = *mcArray.Tail();
		if ((pcUnknown == NULL) && (miFlags & ARRAY_COMMOM_IGNORE_NULL))
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
CUnknown* CArrayCommonUnknown::StartIteration(SSetIterator* psIter)
{
	CUnknown*	pcUnknown;

	if (mcArray.NumElements() != 0)
	{
		psIter->iIndex = 0;
		psIter->bStepFirst = true;
		pcUnknown = *mcArray.Get(psIter->iIndex);

		if ((pcUnknown == NULL) && (miFlags & ARRAY_COMMOM_IGNORE_NULL))
		{
			return Iterate(psIter);
		}
		return pcUnknown;
	}
	psIter->bStepFirst = false;
	psIter->iIndex = 0;
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CArrayCommonUnknown::Iterate(SSetIterator* psIter)
{
	CUnknown*	pcUnknown;
	bool		bMore;

	if (mcArray.NumElements() != 0)
	{
		if (!(miFlags & ARRAY_COMMOM_IGNORE_NULL))
		{
			PrivateIterate(psIter, &pcUnknown);
			return pcUnknown;
		}
		else
		{
			for (;;)
			{
				bMore = PrivateIterate(psIter, &pcUnknown);
				if (bMore)
				{
					if (pcUnknown)
					{
						return pcUnknown;
					}
				}
				else
				{
					return NULL;
				}
			}
		}
	}
	else
	{
		psIter->bStepFirst = false;
		psIter->iIndex = 0;
		return NULL;
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
	return miFlags & ARRAY_COMMOM_KILL_ELEMENT;
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

