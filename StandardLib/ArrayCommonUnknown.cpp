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
	SetFlag(&miFlags, ARRAY_COMMOM_KILL_ELEMENT, bKillElements);
	SetFlag(&miFlags, ARRAY_COMMOM_UNIQUE_ONLY, bUnique);
	SetFlag(&miFlags, ARRAY_COMMOM_IGNORE_NULL, bIgnoreNull);
	SetFlag(&miFlags, ARRAY_COMMOM_PRESERVE_ORDER, bPreserveOrder);
	SetFlag(&miFlags, ARRAY_COMMOM_TYPE_KNOWN, bTypeKnown);
	miNonNullElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::Init(bool bTypeKnown, bool bKillElements, bool bUnique, bool bIgnoreNull, bool bPreserveOrder, int iChunkSize)
{
	mcArray.Init(&gcSystemAllocator, iChunkSize);
	miFlags = 0;
	SetFlag(&miFlags, ARRAY_COMMOM_KILL_ELEMENT, bKillElements);
	SetFlag(&miFlags, ARRAY_COMMOM_UNIQUE_ONLY, bUnique);
	SetFlag(&miFlags, ARRAY_COMMOM_IGNORE_NULL, bIgnoreNull);
	SetFlag(&miFlags, ARRAY_COMMOM_PRESERVE_ORDER, bPreserveOrder);
	SetFlag(&miFlags, ARRAY_COMMOM_TYPE_KNOWN, bTypeKnown);
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
	int			i;
	CUnknown*	pcUnknown;

	if (miFlags & ARRAY_COMMOM_KILL_ELEMENT)
	{
		for (i = 0; i < mcArray.NumElements(); i++)
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
	SetFlag(&miFlags, ARRAY_COMMOM_TYPE_KNOWN, bTypeKnown);
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
	ReturnOnFalse(pcFile->WriteInt(mcArray.NumElements()));
	ReturnOnFalse(pcFile->WriteInt(miNonNullElements));
	ReturnOnFalse(pcFile->WriteInt(mcArray.ChunkSize()));
	ReturnOnFalse(pcFile->WriteInt(miFlags));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::LoadArrayHeader(CFileReader* pcFile, int* piFlags, int* piNumElements)
{
	bool	bTypeKnown;
	int		iChunkSize;
	int		iNonNullElements;

	ReturnOnFalse(pcFile->ReadInt(piNumElements));
	ReturnOnFalse(pcFile->ReadInt(&iNonNullElements));
	ReturnOnFalse(pcFile->ReadInt(&iChunkSize));
	ReturnOnFalse(pcFile->ReadInt(piFlags));

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
	int				iCount;

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
	int				iNumElements;
	int				iFlags;
	int				i;
	CUnknown*		pcUnknown;

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
void CArrayCommonUnknown::PostLoad(int iFlags)
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
void CArrayCommonUnknown::SetChunkSize(int iChunkSize)
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
int CArrayCommonUnknown::NumElements(void)
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
	SetFlag(&miFlags, ARRAY_COMMOM_KILL_ELEMENT, bKill);
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
	SetFlag(&miFlags, ARRAY_COMMOM_UNIQUE_ONLY, bUnique);
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
	SetFlag(&miFlags, ARRAY_COMMOM_IGNORE_NULL, bIgnoreNull);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::PreserveOrder(bool bPreserveOrder)
{
	SetFlag(&miFlags, ARRAY_COMMOM_PRESERVE_ORDER, bPreserveOrder);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::Sort(void)
{
	mcArray.QuickSort(ComparePtrPtr);
	SetFlag(&miFlags, ARRAY_COMMOM_IS_PTR_SORTED, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Contains(CUnknown* pcUnknown)
{
	int		iIndex;
	
	iIndex = Find(pcUnknown);
	if (iIndex >= 0)
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
int CArrayCommonUnknown::Find(CUnknown* pcUnknown)
{
	CUnknown*		pcCurrent;
	int				iIndex;
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
	SetFlag(&miFlags, ARRAY_COMMOM_IS_PTR_SORTED, false);

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
bool CArrayCommonUnknown::Set(int iIndex, CUnknown* pcUnknown, bool bCleanNullsIfNecessary)
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
	SetFlag(&miFlags, ARRAY_COMMOM_IS_PTR_SORTED, false);

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
	int			i;
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
bool CArrayCommonUnknown::Insert(int iIndex, CUnknown* pcUnknown)
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
	SetFlag(&miFlags, ARRAY_COMMOM_IS_PTR_SORTED, false);
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
	int			iDest;
	int			iSource;
	CUnknown*	pcSource;
	int			iLastNotNull;

	if (miFlags & ARRAY_COMMOM_IGNORE_NULL)
	{
		if (mcArray.NumElements() - miNonNullElements >= mcArray.ChunkSize() * 3)
		{
			for (iLastNotNull = mcArray.NumElements()-1; iLastNotNull >= 0; iLastNotNull--)
			{
				pcSource = *mcArray.Get(iLastNotNull);
				if (pcSource)
				{
					break;
				}
			}

			//If iLastNotNull+1 == miNonNullElements then all the NULLs are at the end.  No need to touch the array.
			if (iLastNotNull+1 != miNonNullElements)
			{
				iSource = 0;
				for (iDest = 0; iDest < miNonNullElements; iDest++)
				{
					do 
					{
						pcSource = *mcArray.Get(iSource);
						iSource++;

					} while(pcSource == NULL);
					mcArray.Set(iDest, &pcSource);
				}
			}
			mcArray.SetUsedElements(miNonNullElements);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Remove(CUnknown* pcUnknown)
{
	int			iIndex;

	if ((miFlags & ARRAY_COMMOM_IGNORE_NULL) && (pcUnknown == NULL))
	{
		return false;
	}

	iIndex = Find(pcUnknown);
	if (iIndex != -1)
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
	int			i;

	if (mcArray.NumElements() > 0)
	{
		if (miFlags & ARRAY_COMMOM_IGNORE_NULL)
		{
			for (i = mcArray.NumElements()-1; i >=0; i--)
			{
				pcUnknown = *mcArray.Get(i);
				if (pcUnknown)
				{
					return Remove(i, bCleanNullsIfNecessary);
				}
			}
		}
		else
		{
			return Remove(mcArray.NumElements()-1, bCleanNullsIfNecessary);
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::Remove(int iIndex, bool bCleanNullsIfNecessary)
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
//bool CArrayCommonUnknown::Remove(int iIndex, bool bCleanNullsIfNecessary)
//{
//	CUnknown*	pcNull;
//	CUnknown*	pcUnknown;
//
//	if ((iIndex < 0) && (iIndex >= mcArray.NumElements()))
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
EArrayUnsetReturn CArrayCommonUnknown::Unset(int iIndex)
{
	CUnknown*			pcNull;
	CUnknown*			pcUnknown;
	EArrayUnsetReturn	eReturn;

	eReturn = AUR_NoChange;
	if ((iIndex < 0) && (iIndex >= mcArray.NumElements()))
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
void CArrayCommonUnknown::RemoveDuringIteration(SSetIterator* psIter)
{
	bool	bRemoved;

	if (mcArray.NumElements() > psIter->iIndex)
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
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayCommonUnknown::RemoveEnd(int iIndexInclusive)
{
	int		i;
	bool	bResult;

	//Don't call this if you're using a set.
	bResult = true;
	for (i = mcArray.NumElements()-1; i >= iIndexInclusive; i--)
	{
		bResult &= Remove(i, false);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonUnknown::UnsafeSet(int iIndex, CUnknown* pcUnknown)
{
	mcArray.Set(iIndex, &pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CArrayCommonUnknown::UnsafeGet(int iIndex)
{
	return *mcArray.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown** CArrayCommonUnknown::UnsafeGetPointer(int iIndex)
{
	return mcArray.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayCommonUnknown::UnsafeNumElements(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayCommonUnknown::UnsafeNonNullElements(void)
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
	int			i;

	if (mcArray.NumElements() > 0)
	{
		pcUnknown = *mcArray.Tail();
		if ((pcUnknown == NULL) && (miFlags & ARRAY_COMMOM_IGNORE_NULL))
		{
			for (i = mcArray.NumElements()-2; i >=0; i--)
			{
				pcUnknown = *mcArray.Get(i);
				if (pcUnknown)
				{
					return pcUnknown;
				}
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

	if (mcArray.NumElements() > 0)
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

	if (mcArray.NumElements() > 0)
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
	int			iNum;

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
