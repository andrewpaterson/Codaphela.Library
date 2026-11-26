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
#ifndef __ARRAY_COMMON_UNKNOWN_H__
#define __ARRAY_COMMON_UNKNOWN_H__
#include "BaseLib/DataCompare.h"
#include "SetIterator.h"
#include "ArrayUnknownPtr.h"
#include "Unknown.h"


#define ARRAY_COMMON_KILL_ELEMENT	0x0001
#define ARRAY_COMMON_IS_PTR_SORTED	0x0002
#define ARRAY_COMMON_UNIQUE_ONLY	0x0004
#define ARRAY_COMMON_IGNORE_NULL	0x0008
#define ARRAY_COMMON_PRESERVE_ORDER	0x0010
#define ARRAY_COMMON_TYPE_KNOWN		0x0020
#define ARRAY_COMMON_SORT_POINTERS	0x0080
#define ARRAY_COMMON_SORT_BY_INDEX	0x0100

#define ARRAY_COMMON_INIT_FLAGS			(ARRAY_COMMON_KILL_ELEMENT | ARRAY_COMMON_UNIQUE_ONLY | ARRAY_COMMON_IGNORE_NULL | ARRAY_COMMON_PRESERVE_ORDER | ARRAY_COMMON_TYPE_KNOWN | ARRAY_COMMON_SORT_POINTERS | ARRAY_COMMON_SORT_BY_INDEX)
#define ARRAY_COMMON_DEFAULT_CHUNK_SIZE	1

// ARRAY_COMMON_IS_PTR_SORTED  indicates if the array is sorted, not if it must be sorted.
// ARRAY_COMMON_SORT_POINTERS  the array is kept sorted by its elements' addresses (a set made to sort them).  Each
//                             change that could leave it out of order clears ARRAY_COMMON_IS_PTR_SORTED.
// ARRAY_COMMON_SORT_BY_INDEX  with ARRAY_COMMON_SORT_POINTERS: the elements are objects, sorted by their indexes
//                             (CEmbeddedObject::CompareIndex) rather than their addresses, so an object loaded
//                             in place of its hollow keeps its place.
// ARRAY_COMMON_IGNORE_NULL    determines if iterate skips NULL elements.  Will also cause NULL additions to fail.

enum EArrayUnsetReturn
{
	AUR_NoChange,
	AUR_SetNull,
	AUR_MustRemove,
};


//This is called CArrayCommonUnknown because it *HAS* a _CArrayUnknownPtr not because it *IS* a the base class for unknown arrays (although it is that also).
class CArrayCommonUnknown : public CUnknown
{
CONSTRUCTABLE(CArrayCommonUnknown)
protected:
	uint16				muiFlags;

private:
	bool				mbSlotsMoved;  //An element has moved to another index since ClearSlotsMoved.  Not a flag: other threads read the flags while a load clears this.
	CArrayUnknownPtr	mcArray;  //CArrayTemplatePtr<CUnknown>
	size				miNonNullElements;

	bool				PrivateIterate(SSetIterator* psIter, CUnknown** ppcUnknown);
	void				PrivateKill(void);
	DataCompare			SortCompare(void) const;
	size				FindNextSortedWith(CUnknown* pcUnknown, size iFound, size uiStartIndex);

public:
			void				_Init(void);  //Valid but empty.  Allocates nothing.  Safe to walk before Init.
			void				Init(uint16 uiFlags, size iChunkSize = ARRAY_COMMON_DEFAULT_CHUNK_SIZE);
	static	uint16				Flags(bool bTypeKnown, bool bKillElements, bool bUnique, bool bIgnoreNull, bool bPreserveOrder, bool bSortPointers);
			void				Kill(void) override;
	virtual void				ReInit(void);

			bool				Save(CFileWriter* pcFile) override;
			bool				Load(CFileReader* pcFile) override;

			//Most of these should be protected or virtual.
			size				NumElements(void) const;
			void				KillElements(bool bKill);

			void				Sort(void);
			bool				IsSorted(void) const;
			bool				CalculateIsSorted(void);
			bool				IsMustSort(void) const;
			void				PointerReplaced(CUnknown* pcOld, CUnknown* pcNew);

			bool				Contains(CUnknown* pcUnknown);
			size				Find(CUnknown* pcUnknown);
			size				FindNext(CUnknown* pcUnknown, size uiStartIndex);

			void				CleanNullsIfNecessary(bool bCleanNullsIfNecessary);
			void				CleanNullsIfNecessary(void);
			void				CleanNullsBeforeBinarySearch(void);
			void				CleanNulls(void);
			size				CountNonNull(void);

			bool				Add(CUnknown* pcUnknown, bool bCleanNullsIfNecessary = true);
			bool				AddAll(CArrayCommonUnknown* pcSource);
			bool				Set(size iIndex, CUnknown* pcUnknown, bool bCleanNullsIfNecessary = true);
			bool				Insert(size iIndex, CUnknown* pcUnknown);
			size				InsertIntoSorted(CUnknown* pcUnknown);

			bool				RemoveEnd(size iIndexInclusive);
	virtual size				Remove(CUnknown* pcUnknown);
			bool				Remove(size iIndex, bool bCleanNullsIfNecessary = true);
			bool				RemoveLast(bool bCleanNullsIfNecessary = true);
			bool				RemoveDuringIteration(SSetIterator* psIter);

			EArrayUnsetReturn	Unset(size iIndex);

			CUnknown*			First(void);
			CUnknown*			Last(void);

			bool				StartIteration(SSetIterator* psIter, CUnknown** ppcUnknown);
			bool				Iterate(SSetIterator* psIter, CUnknown** ppcUnknown);

			void				PreserveOrder(bool bPreserveOrder);
			void				TypeKnown(bool bTypeKnown);
			bool				IsKillElements(void) const;
			bool				IsEmpty(void) const;
			bool				IsNotEmpty(void) const;
			bool				IsIgnoreNull(void) const;
			bool				HaveSlotsMoved(void);
			void				ClearSlotsMoved(void);

			void				UnsafeSet(size iIndex, CUnknown* pcUnknown);
			CUnknown*			UnsafeGet(size iIndex);
			CUnknown**			UnsafeGetPointer(size iIndex);
			size				UnsafeNumElements(void);
			size				UnsafeNonNullElements(void);

			bool				ReadArrayHeader(CFileReader* pcFile, uint16* piFlags, size* piNumElements);
			bool				WriteArrayHeader(CFileWriter* pcFile);
			void				PostLoad(uint16 iFlags);
			void				PostLoadPointers(uint16 iFlags);

			void				SetChunkSize(size iChunkSize);
			void				GrowTo(size iNumElements);

protected:
	virtual bool				ReadElement(CFileReader* pcFile, CUnknown** ppcUnknown);
	virtual bool				WriteElement(CFileWriter* pcFile, CUnknown* pcUnknown);

			bool				WriteElements(CFileWriter* pcFile);
			bool				ReadElements(CFileReader* pcFile, size iNumElements);
			void				SlotsMoved(void);
};


#endif // __ARRAY_COMMON_UNKNOWN_H__

